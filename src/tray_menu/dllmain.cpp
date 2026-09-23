#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <roapi.h>
#include <winstring.h>

#include <string>

#include "MinHook.h"
#include "../common/constants.h"

namespace {

using namespace icloud_tray_restart;

using TrackPopupMenuExFunction =
    BOOL(WINAPI*)(HMENU, UINT, int, int, HWND, LPTPMPARAMS);
using TrackPopupMenuFunction = BOOL(WINAPI*)(HMENU, UINT, int, int, int, HWND,
                                             const RECT*);

TrackPopupMenuExFunction g_original_track_popup_menu_ex = nullptr;
TrackPopupMenuFunction g_original_track_popup_menu = nullptr;
HANDLE g_restart_event = nullptr;
HANDLE g_hook_ready_event = nullptr;
thread_local bool g_inside_hook = false;

struct StringVectorView : IInspectable {
  virtual HRESULT STDMETHODCALLTYPE GetAt(UINT32 index, HSTRING* value) = 0;
  virtual HRESULT STDMETHODCALLTYPE get_Size(UINT32* value) = 0;
};

struct GlobalizationPreferencesStatics : IInspectable {
  virtual HRESULT STDMETHODCALLTYPE get_Calendars(void** value) = 0;
  virtual HRESULT STDMETHODCALLTYPE get_Clocks(void** value) = 0;
  virtual HRESULT STDMETHODCALLTYPE get_Currencies(void** value) = 0;
  virtual HRESULT STDMETHODCALLTYPE get_Languages(StringVectorView** value) = 0;
};

constexpr GUID kGlobalizationPreferencesStaticsId = {
    0x01bf4326, 0xed37, 0x4e96, {0xb0, 0xe9, 0xc1, 0x34, 0x0d, 0x1e, 0xa1, 0x58}};
constexpr wchar_t kGlobalizationPreferencesClass[] =
    L"Windows.System.UserProfile.GlobalizationPreferences";

bool MenuContainsCommand(HMENU menu, UINT command_id) {
  const int count = GetMenuItemCount(menu);
  for (int index = 0; index < count; ++index) {
    if (GetMenuItemID(menu, index) == command_id) {
      return true;
    }
  }
  return false;
}

bool LooksLikeICloudTrayMenu(HMENU menu) {
  const int count = GetMenuItemCount(menu);
  if (count < 2 || count > 12) {
    return false;
  }

  bool has_icloud_website = false;
  bool has_second_icloud_item = false;
  wchar_t text[256] = {};

  for (int index = 0; index < count; ++index) {
    text[0] = L'\0';
    if (GetMenuStringW(menu, static_cast<UINT>(index), text,
                       static_cast<int>(ARRAYSIZE(text)), MF_BYPOSITION) == 0) {
      continue;
    }
    const std::wstring item(text);
    if (item.find(L"iCloud.com") != std::wstring::npos) {
      has_icloud_website = true;
    } else if (item.find(L"iCloud") != std::wstring::npos) {
      has_second_icloud_item = true;
    }
  }

  return has_icloud_website && has_second_icloud_item;
}

bool LanguageTagMatches(const wchar_t* language, const wchar_t* prefix) {
  const size_t length = wcslen(prefix);
  return wcslen(language) >= length &&
         _wcsnicmp(language, prefix, length) == 0 &&
         (language[length] == L'\0' || language[length] == L'-');
}

const wchar_t* RestartMenuLabelForLanguage(const wchar_t* language) {
  if (LanguageTagMatches(language, L"en")) {
    return kRestartMenuLabelEnglish;
  }
  if (LanguageTagMatches(language, L"ja")) {
    return kRestartMenuLabelJapanese;
  }
  if (LanguageTagMatches(language, L"zh")) {
    if (LanguageTagMatches(language, L"zh-Hant") ||
        LanguageTagMatches(language, L"zh-TW") ||
        LanguageTagMatches(language, L"zh-HK") ||
        LanguageTagMatches(language, L"zh-MO")) {
      return kRestartMenuLabelChineseTraditional;
    }
    return kRestartMenuLabelChineseSimplified;
  }
  return nullptr;
}

const wchar_t* RestartMenuLabelForUser() {
  const HRESULT initialization = RoInitialize(RO_INIT_MULTITHREADED);
  if (FAILED(initialization) && initialization != RPC_E_CHANGED_MODE) {
    return kRestartMenuLabelEnglish;
  }

  const wchar_t* label = kRestartMenuLabelEnglish;
  HSTRING class_name = nullptr;
  GlobalizationPreferencesStatics* preferences = nullptr;
  StringVectorView* languages = nullptr;

  if (SUCCEEDED(WindowsCreateString(kGlobalizationPreferencesClass,
                                    ARRAYSIZE(kGlobalizationPreferencesClass) - 1,
                                    &class_name)) &&
      SUCCEEDED(RoGetActivationFactory(
          class_name, kGlobalizationPreferencesStaticsId,
          reinterpret_cast<void**>(&preferences))) &&
      SUCCEEDED(preferences->get_Languages(&languages))) {
    UINT32 count = 0;
    if (SUCCEEDED(languages->get_Size(&count))) {
      for (UINT32 index = 0; index < count; ++index) {
        HSTRING value = nullptr;
        if (FAILED(languages->GetAt(index, &value))) {
          break;
        }
        const wchar_t* candidate = RestartMenuLabelForLanguage(
            WindowsGetStringRawBuffer(value, nullptr));
        WindowsDeleteString(value);
        if (candidate != nullptr) {
          label = candidate;
          break;
        }
      }
    }
  }

  if (languages != nullptr) languages->Release();
  if (preferences != nullptr) preferences->Release();
  if (class_name != nullptr) WindowsDeleteString(class_name);
  if (SUCCEEDED(initialization)) RoUninitialize();
  return label;
}

bool PrepareICloudMenu(HMENU menu) {
  if (menu == nullptr) {
    return false;
  }
  if (MenuContainsCommand(menu, kRestartMenuId)) {
    return true;
  }
  if (!LooksLikeICloudTrayMenu(menu)) {
    return false;
  }

  const int original_count = GetMenuItemCount(menu);
  if (!AppendMenuW(menu, MF_SEPARATOR, 0, nullptr)) {
    return false;
  }
  if (!AppendMenuW(menu, MF_STRING, kRestartMenuId,
                   RestartMenuLabelForUser())) {
    while (GetMenuItemCount(menu) > original_count) {
      DeleteMenu(menu, original_count, MF_BYPOSITION);
    }
    return false;
  }
  return true;
}

void RequestEvent(HANDLE* event, const wchar_t* event_name) {
  if (*event == nullptr) {
    *event = OpenEventW(EVENT_MODIFY_STATE, FALSE, event_name);
  }
  if (*event != nullptr) {
    SetEvent(*event);
  }
}

bool HandleOwnCommand(UINT command) {
  if (command == kRestartMenuId) {
    RequestEvent(&g_restart_event, kRestartEventName);
    return true;
  }
  return false;
}

BOOL DispatchSelection(UINT selected_command, HWND owner) {
  if (selected_command == 0) {
    return FALSE;
  }
  if (HandleOwnCommand(selected_command)) {
    return TRUE;
  }
  if (owner != nullptr) {
    SendMessageW(owner, WM_COMMAND, MAKEWPARAM(selected_command, 0), 0);
  }
  return TRUE;
}

BOOL WINAPI HookTrackPopupMenuEx(HMENU menu, UINT flags, int x, int y,
                                 HWND owner, LPTPMPARAMS parameters) {
  if (g_inside_hook || !PrepareICloudMenu(menu)) {
    return g_original_track_popup_menu_ex(menu, flags, x, y, owner, parameters);
  }

  g_inside_hook = true;
  BOOL result = FALSE;
  if ((flags & TPM_RETURNCMD) != 0) {
    result = g_original_track_popup_menu_ex(menu, flags, x, y, owner, parameters);
    if (HandleOwnCommand(static_cast<UINT>(result))) {
      result = 0;
    }
  } else {
    const UINT selected = static_cast<UINT>(g_original_track_popup_menu_ex(
        menu, flags | TPM_RETURNCMD | TPM_NONOTIFY, x, y, owner, parameters));
    result = DispatchSelection(selected, owner);
  }
  g_inside_hook = false;
  return result;
}

BOOL WINAPI HookTrackPopupMenu(HMENU menu, UINT flags, int x, int y,
                               int reserved, HWND owner,
                               const RECT* excluded_rectangle) {
  if (g_inside_hook || !PrepareICloudMenu(menu)) {
    return g_original_track_popup_menu(menu, flags, x, y, reserved, owner,
                                       excluded_rectangle);
  }

  g_inside_hook = true;
  BOOL result = FALSE;
  if ((flags & TPM_RETURNCMD) != 0) {
    result = g_original_track_popup_menu(menu, flags, x, y, reserved, owner,
                                         excluded_rectangle);
    if (HandleOwnCommand(static_cast<UINT>(result))) {
      result = 0;
    }
  } else {
    const UINT selected = static_cast<UINT>(g_original_track_popup_menu(
        menu, flags | TPM_RETURNCMD | TPM_NONOTIFY, x, y, reserved, owner,
        excluded_rectangle));
    result = DispatchSelection(selected, owner);
  }
  g_inside_hook = false;
  return result;
}

DWORD WINAPI InitializeHooks(void*) {
  g_restart_event = OpenEventW(EVENT_MODIFY_STATE, FALSE, kRestartEventName);
  if (g_restart_event == nullptr) {
    OutputDebugStringW(
        L"ICloudTrayMenu: restart event is unavailable; hooks not installed\n");
    return 1;
  }
  g_hook_ready_event =
      OpenEventW(EVENT_MODIFY_STATE, FALSE, kHookReadyEventName);

  const MH_STATUS initialize_status = MH_Initialize();
  if (initialize_status != MH_OK &&
      initialize_status != MH_ERROR_ALREADY_INITIALIZED) {
    OutputDebugStringW(L"ICloudTrayMenu: MH_Initialize failed\n");
    return 2;
  }

  bool created_any_hook = false;
  if (MH_CreateHookApi(L"user32.dll", "TrackPopupMenuEx",
                       reinterpret_cast<void*>(&HookTrackPopupMenuEx),
                       reinterpret_cast<void**>(
                           &g_original_track_popup_menu_ex)) == MH_OK) {
    created_any_hook = true;
  }
  if (MH_CreateHookApi(L"user32.dll", "TrackPopupMenu",
                       reinterpret_cast<void*>(&HookTrackPopupMenu),
                       reinterpret_cast<void**>(&g_original_track_popup_menu)) ==
      MH_OK) {
    created_any_hook = true;
  }

  if (!created_any_hook || MH_EnableHook(MH_ALL_HOOKS) != MH_OK) {
    OutputDebugStringW(L"ICloudTrayMenu: unable to enable popup menu hooks\n");
    return 3;
  }

  if (g_hook_ready_event != nullptr) {
    SetEvent(g_hook_ready_event);
  }
  OutputDebugStringW(L"ICloudTrayMenu: hooks installed\n");
  return 0;
}

}  // namespace

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, void*) {
  if (reason == DLL_PROCESS_ATTACH) {
    DisableThreadLibraryCalls(instance);
    HANDLE thread = CreateThread(nullptr, 0, InitializeHooks, nullptr, 0, nullptr);
    if (thread != nullptr) {
      CloseHandle(thread);
    }
  } else if (reason == DLL_PROCESS_DETACH) {
    if (g_restart_event != nullptr) {
      CloseHandle(g_restart_event);
      g_restart_event = nullptr;
    }
    if (g_hook_ready_event != nullptr) {
      CloseHandle(g_hook_ready_event);
      g_hook_ready_event = nullptr;
    }
  }
  return TRUE;
}
