#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <appmodel.h>
#include <shellapi.h>
#include <tlhelp32.h>

#include <algorithm>
#include <cwctype>
#include <string>
#include <vector>

#include "../common/constants.h"

namespace {

using namespace icloud_tray_restart;

class UniqueHandle {
 public:
  UniqueHandle() = default;
  explicit UniqueHandle(HANDLE handle) : handle_(handle) {}
  ~UniqueHandle() { Reset(); }

  UniqueHandle(const UniqueHandle&) = delete;
  UniqueHandle& operator=(const UniqueHandle&) = delete;

  UniqueHandle(UniqueHandle&& other) noexcept : handle_(other.Release()) {}
  UniqueHandle& operator=(UniqueHandle&& other) noexcept {
    if (this != &other) {
      Reset(other.Release());
    }
    return *this;
  }

  HANDLE Get() const { return handle_; }
  explicit operator bool() const {
    return handle_ != nullptr && handle_ != INVALID_HANDLE_VALUE;
  }
  HANDLE Release() {
    HANDLE result = handle_;
    handle_ = nullptr;
    return result;
  }
  void Reset(HANDLE handle = nullptr) {
    if (handle_ != nullptr && handle_ != INVALID_HANDLE_VALUE) {
      CloseHandle(handle_);
    }
    handle_ = handle;
  }

 private:
  HANDLE handle_ = nullptr;
};

std::wstring ToLower(std::wstring value) {
  std::transform(value.begin(), value.end(), value.begin(),
                 [](wchar_t character) {
                   return static_cast<wchar_t>(std::towlower(character));
                 });
  return value;
}

bool EndsWithCaseInsensitive(const std::wstring& value,
                             const std::wstring& suffix) {
  if (suffix.size() > value.size()) {
    return false;
  }
  return ToLower(value.substr(value.size() - suffix.size())) == ToLower(suffix);
}

std::wstring GetModuleDirectory() {
  std::vector<wchar_t> buffer(32768);
  const DWORD length =
      GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
  if (length == 0 || length >= buffer.size()) {
    return L"";
  }
  std::wstring path(buffer.data(), length);
  const auto slash = path.find_last_of(L"\\/");
  return slash == std::wstring::npos ? L"" : path.substr(0, slash);
}

std::wstring GetLogPath() {
  wchar_t local_app_data[32768] = {};
  DWORD length = GetEnvironmentVariableW(L"LOCALAPPDATA", local_app_data,
                                         ARRAYSIZE(local_app_data));
  if (length == 0 || length >= ARRAYSIZE(local_app_data)) {
    return GetModuleDirectory() + L"\\ICloudTrayRestart.log";
  }

  std::wstring root = std::wstring(local_app_data) + L"\\iCloudTrayRestart";
  std::wstring logs = root + L"\\Logs";
  CreateDirectoryW(root.c_str(), nullptr);
  CreateDirectoryW(logs.c_str(), nullptr);
  return logs + L"\\bridge.log";
}

std::string WideToUtf8(const std::wstring& value) {
  if (value.empty()) {
    return {};
  }
  const int required = WideCharToMultiByte(CP_UTF8, 0, value.c_str(),
                                            static_cast<int>(value.size()), nullptr,
                                            0, nullptr, nullptr);
  if (required <= 0) {
    return {};
  }
  std::string result(static_cast<size_t>(required), '\0');
  WideCharToMultiByte(CP_UTF8, 0, value.c_str(), static_cast<int>(value.size()),
                      result.data(), required, nullptr, nullptr);
  return result;
}

void Log(const std::wstring& message) {
  SYSTEMTIME now = {};
  GetLocalTime(&now);
  wchar_t prefix[64] = {};
  wsprintfW(prefix, L"[%04u-%02u-%02u %02u:%02u:%02u] ", now.wYear,
            now.wMonth, now.wDay, now.wHour, now.wMinute, now.wSecond);
  const std::string line = WideToUtf8(std::wstring(prefix) + message + L"\r\n");
  if (line.empty()) {
    return;
  }

  UniqueHandle file(CreateFileW(GetLogPath().c_str(), FILE_APPEND_DATA,
                                FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
                                OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr));
  if (!file) {
    return;
  }
  DWORD written = 0;
  WriteFile(file.Get(), line.data(), static_cast<DWORD>(line.size()), &written,
            nullptr);
}

std::wstring GetProcessImagePath(HANDLE process) {
  std::vector<wchar_t> buffer(32768);
  DWORD length = static_cast<DWORD>(buffer.size());
  if (!QueryFullProcessImageNameW(process, 0, buffer.data(), &length)) {
    return L"";
  }
  return std::wstring(buffer.data(), length);
}

std::wstring GetProcessPackageFamily(HANDLE process) {
  UINT32 length = 0;
  LONG result = GetPackageFamilyName(process, &length, nullptr);
  if (result != ERROR_INSUFFICIENT_BUFFER || length == 0) {
    return L"";
  }
  std::vector<wchar_t> buffer(length);
  result = GetPackageFamilyName(process, &length, buffer.data());
  if (result != ERROR_SUCCESS) {
    return L"";
  }
  return std::wstring(buffer.data());
}

std::wstring GetProcessAppUserModelId(HANDLE process) {
  UINT32 length = 0;
  LONG result = GetApplicationUserModelId(process, &length, nullptr);
  if (result != ERROR_INSUFFICIENT_BUFFER || length == 0) {
    return L"";
  }
  std::vector<wchar_t> buffer(length);
  result = GetApplicationUserModelId(process, &length, buffer.data());
  return result == ERROR_SUCCESS ? std::wstring(buffer.data()) : L"";
}

bool IsExpectedICloudProcess(HANDLE process, const std::wstring& expected_name,
                             std::wstring* image_path = nullptr) {
  const std::wstring path = GetProcessImagePath(process);
  if (path.empty()) {
    return false;
  }

  const bool expected_file = EndsWithCaseInsensitive(path, L"\\" + expected_name);
  const bool expected_package =
      _wcsicmp(GetProcessPackageFamily(process).c_str(), kPackageFamilyName) == 0;

  if (expected_file && expected_package) {
    if (image_path != nullptr) {
      *image_path = path;
    }
    return true;
  }
  return false;
}

DWORD FindICloudProcess(const wchar_t* executable_name) {
  UniqueHandle snapshot(
      CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0));
  if (!snapshot) {
    return 0;
  }

  PROCESSENTRY32W entry = {};
  entry.dwSize = sizeof(entry);
  if (!Process32FirstW(snapshot.Get(), &entry)) {
    return 0;
  }

  DWORD current_session = 0;
  if (!ProcessIdToSessionId(GetCurrentProcessId(), &current_session)) {
    return 0;
  }
  do {
    if (_wcsicmp(entry.szExeFile, executable_name) != 0) {
      continue;
    }
    DWORD process_session = 0;
    if (!ProcessIdToSessionId(entry.th32ProcessID, &process_session) ||
        process_session != current_session) {
      continue;
    }
    UniqueHandle process(OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE,
                                     entry.th32ProcessID));
    if (process && IsExpectedICloudProcess(process.Get(), executable_name)) {
      return entry.th32ProcessID;
    }
  } while (Process32NextW(snapshot.Get(), &entry));

  return 0;
}

uintptr_t FindRemoteModuleBase(DWORD process_id, const wchar_t* module_name) {
  UniqueHandle snapshot(CreateToolhelp32Snapshot(
      TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, process_id));
  if (!snapshot) {
    return 0;
  }

  MODULEENTRY32W entry = {};
  entry.dwSize = sizeof(entry);
  if (!Module32FirstW(snapshot.Get(), &entry)) {
    return 0;
  }

  do {
    if (_wcsicmp(entry.szModule, module_name) == 0) {
      return reinterpret_cast<uintptr_t>(entry.modBaseAddr);
    }
  } while (Module32NextW(snapshot.Get(), &entry));
  return 0;
}

bool IsModuleLoaded(DWORD process_id, const wchar_t* module_name) {
  return FindRemoteModuleBase(process_id, module_name) != 0;
}

bool IsProcessAlive(DWORD process_id) {
  if (process_id == 0) {
    return false;
  }
  UniqueHandle process(
      OpenProcess(SYNCHRONIZE | PROCESS_QUERY_LIMITED_INFORMATION, FALSE,
                  process_id));
  return process && WaitForSingleObject(process.Get(), 0) == WAIT_TIMEOUT;
}

bool InjectLibrary(DWORD process_id, const std::wstring& dll_path,
                   HANDLE hook_ready_event) {
  if (IsModuleLoaded(process_id, kInjectedDllName)) {
    return WaitForSingleObject(hook_ready_event, 0) == WAIT_OBJECT_0;
  }

  ResetEvent(hook_ready_event);

  UniqueHandle process(OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION |
                                       PROCESS_VM_OPERATION | PROCESS_VM_WRITE |
                                       PROCESS_VM_READ,
                                   FALSE, process_id));
  if (!process) {
    Log(L"OpenProcess failed for PID " + std::to_wstring(process_id) +
        L", error=" + std::to_wstring(GetLastError()));
    return false;
  }

  const SIZE_T bytes = (dll_path.size() + 1) * sizeof(wchar_t);
  void* remote_memory = VirtualAllocEx(process.Get(), nullptr, bytes,
                                       MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
  if (remote_memory == nullptr) {
    Log(L"VirtualAllocEx failed, error=" + std::to_wstring(GetLastError()));
    return false;
  }

  bool success = false;
  do {
    SIZE_T written = 0;
    if (!WriteProcessMemory(process.Get(), remote_memory, dll_path.c_str(), bytes,
                            &written) ||
        written != bytes) {
      Log(L"WriteProcessMemory failed, error=" +
          std::to_wstring(GetLastError()));
      break;
    }

    HMODULE local_kernel32 = GetModuleHandleW(L"kernel32.dll");
    FARPROC local_load_library =
        local_kernel32 == nullptr
            ? nullptr
            : GetProcAddress(local_kernel32, "LoadLibraryW");
    const uintptr_t remote_kernel32 =
        FindRemoteModuleBase(process_id, L"kernel32.dll");
    if (local_kernel32 == nullptr || local_load_library == nullptr ||
        remote_kernel32 == 0) {
      Log(L"Unable to resolve LoadLibraryW in target process");
      break;
    }

    const uintptr_t loader_offset =
        reinterpret_cast<uintptr_t>(local_load_library) -
        reinterpret_cast<uintptr_t>(local_kernel32);
    auto remote_loader = reinterpret_cast<LPTHREAD_START_ROUTINE>(
        remote_kernel32 + loader_offset);

    UniqueHandle thread(CreateRemoteThread(process.Get(), nullptr, 0, remote_loader,
                                           remote_memory, 0, nullptr));
    if (!thread) {
      Log(L"CreateRemoteThread failed, error=" +
          std::to_wstring(GetLastError()));
      break;
    }

    if (WaitForSingleObject(thread.Get(), 10000) != WAIT_OBJECT_0) {
      Log(L"Timed out waiting for LoadLibraryW");
      break;
    }

    DWORD exit_code = 0;
    if (!GetExitCodeThread(thread.Get(), &exit_code) || exit_code == 0) {
      Log(L"LoadLibraryW failed in target process");
      break;
    }
    success = true;
  } while (false);

  VirtualFreeEx(process.Get(), remote_memory, 0, MEM_RELEASE);
  if (!success) {
    return false;
  }

  if (WaitForSingleObject(hook_ready_event, 5000) != WAIT_OBJECT_0) {
    Log(L"DLL loaded, but the popup menu hooks did not report ready");
    return false;
  }
  return true;
}

std::vector<DWORD> FindAllICloudProcessIds() {
  std::vector<DWORD> process_ids;
  UniqueHandle snapshot(
      CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0));
  if (!snapshot) {
    return process_ids;
  }

  PROCESSENTRY32W entry = {};
  entry.dwSize = sizeof(entry);
  if (!Process32FirstW(snapshot.Get(), &entry)) {
    return process_ids;
  }

  const DWORD current_process = GetCurrentProcessId();
  DWORD current_session = 0;
  if (!ProcessIdToSessionId(current_process, &current_session)) {
    return process_ids;
  }
  do {
    if (entry.th32ProcessID == current_process) {
      continue;
    }
    DWORD process_session = 0;
    if (!ProcessIdToSessionId(entry.th32ProcessID, &process_session) ||
        process_session != current_session) {
      continue;
    }
    UniqueHandle process(OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE,
                                     entry.th32ProcessID));
    if (!process) {
      continue;
    }
    if (_wcsicmp(GetProcessPackageFamily(process.Get()).c_str(),
                 kPackageFamilyName) == 0) {
      process_ids.push_back(entry.th32ProcessID);
    }
  } while (Process32NextW(snapshot.Get(), &entry));

  return process_ids;
}

bool StartICloud(const std::wstring& app_user_model_id) {
  const std::wstring fallback_id =
      std::wstring(kPackageFamilyName) + L"!iCloud";
  const std::wstring arguments = std::wstring(L"shell:AppsFolder\\") +
      (app_user_model_id.empty() ? fallback_id : app_user_model_id);
  const HINSTANCE result =
      ShellExecuteW(nullptr, L"open", L"explorer.exe", arguments.c_str(), nullptr,
                    SW_SHOWNORMAL);
  if (reinterpret_cast<INT_PTR>(result) <= 32) {
    Log(L"ShellExecuteW failed to start iCloud, result=" +
        std::to_wstring(reinterpret_cast<INT_PTR>(result)));
    return false;
  }
  Log(L"iCloud launch requested");
  return true;
}

void RestartICloud() {
  Log(L"Restart requested from tray menu");
  Sleep(350);

  std::wstring app_user_model_id;
  const DWORD home_process_id = FindICloudProcess(kICloudHomeExecutable);
  if (home_process_id != 0) {
    UniqueHandle home_process(OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,
                                          FALSE, home_process_id));
    if (home_process) {
      app_user_model_id = GetProcessAppUserModelId(home_process.Get());
    }
  }

  const std::vector<DWORD> process_ids = FindAllICloudProcessIds();
  std::vector<UniqueHandle> terminated_processes;
  terminated_processes.reserve(process_ids.size());

  for (DWORD process_id : process_ids) {
    UniqueHandle process(OpenProcess(PROCESS_TERMINATE | SYNCHRONIZE, FALSE,
                                     process_id));
    if (!process) {
      Log(L"Unable to open iCloud PID " + std::to_wstring(process_id) +
          L" for termination, error=" + std::to_wstring(GetLastError()));
      continue;
    }
    if (!TerminateProcess(process.Get(), 0)) {
      Log(L"TerminateProcess failed for PID " + std::to_wstring(process_id) +
          L", error=" + std::to_wstring(GetLastError()));
      continue;
    }
    terminated_processes.push_back(std::move(process));
  }

  for (auto& process : terminated_processes) {
    WaitForSingleObject(process.Get(), 5000);
  }
  Sleep(1200);
  StartICloud(app_user_model_id);
}

}  // namespace

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int) {
  UniqueHandle mutex(CreateMutexW(nullptr, TRUE, kBridgeMutexName));
  if (!mutex) {
    return 2;
  }
  if (GetLastError() == ERROR_ALREADY_EXISTS) {
    return 0;
  }

  UniqueHandle restart_event(
      CreateEventW(nullptr, FALSE, FALSE, kRestartEventName));
  if (!restart_event) {
    Log(L"CreateEventW failed, error=" + std::to_wstring(GetLastError()));
    return 3;
  }

  UniqueHandle hook_ready_event(
      CreateEventW(nullptr, TRUE, FALSE, kHookReadyEventName));
  if (!hook_ready_event) {
    Log(L"Unable to create the hook-ready event, error=" +
        std::to_wstring(GetLastError()));
    return 4;
  }

  const std::wstring dll_path =
      GetModuleDirectory() + L"\\" + kInjectedDllName;
  if (GetFileAttributesW(dll_path.c_str()) == INVALID_FILE_ATTRIBUTES) {
    Log(L"Injected DLL not found: " + dll_path);
    return 5;
  }

  Log(L"Bridge started");
  DWORD injected_process_id = 0;
  ULONGLONG next_retry_at = 0;
  while (true) {
    const DWORD wait_result =
        WaitForSingleObject(restart_event.Get(), 1000);
    if (wait_result == WAIT_OBJECT_0) {
      RestartICloud();
      injected_process_id = 0;
      next_retry_at = GetTickCount64() + 2000;
      continue;
    }
    if (wait_result == WAIT_FAILED) {
      Log(L"Request event wait failed, error=" +
          std::to_wstring(GetLastError()));
      return 6;
    }

    if (injected_process_id != 0 && IsProcessAlive(injected_process_id)) {
      continue;
    }
    injected_process_id = 0;

    const ULONGLONG now = GetTickCount64();
    if (now < next_retry_at) {
      continue;
    }

    const DWORD process_id = FindICloudProcess(kICloudHomeExecutable);
    if (process_id == 0) {
      next_retry_at = now + 2000;
      continue;
    }

    if (InjectLibrary(process_id, dll_path, hook_ready_event.Get())) {
      injected_process_id = process_id;
      Log(L"Tray menu module loaded into PID " +
          std::to_wstring(process_id));
    } else {
      Log(L"Injection failed for PID " + std::to_wstring(process_id));
      next_retry_at = now + 5000;
    }
  }
}
