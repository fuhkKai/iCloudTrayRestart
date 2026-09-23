#pragma once

#include <windows.h>

#if !defined(_M_X64) && !defined(__x86_64__)
#error iCloudTrayRestart requires an x64 toolchain.
#endif

namespace icloud_tray_restart {

inline constexpr wchar_t kBridgeMutexName[] =
    L"Local\\ICloudTrayRestart_Bridge_v1";
inline constexpr wchar_t kRestartEventName[] =
    L"Local\\ICloudTrayRestart_Request_v1";
inline constexpr wchar_t kHookReadyEventName[] =
    L"Local\\ICloudTrayRestart_HookReady_v1";
inline constexpr wchar_t kPackageFamilyName[] =
    L"AppleInc.iCloud_nzyj5cx40ttqa";
inline constexpr wchar_t kICloudHomeExecutable[] = L"iCloudHome.exe";
inline constexpr wchar_t kInjectedDllName[] = L"ICloudTrayMenu.dll";
inline constexpr wchar_t kRestartMenuLabelEnglish[] = L"Restart iCloud";
inline constexpr wchar_t kRestartMenuLabelJapanese[] = L"iCloudを再起動";
inline constexpr wchar_t kRestartMenuLabelChineseSimplified[] =
    L"重新启动 iCloud";
inline constexpr wchar_t kRestartMenuLabelChineseTraditional[] =
    L"重新啟動 iCloud";
inline constexpr UINT kRestartMenuId = 0x7F71;

}  // namespace icloud_tray_restart
