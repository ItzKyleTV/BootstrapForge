#include <Windows.h>
#include <shellapi.h>

#include <array>
#include <filesystem>
#include <string>

#ifndef BOOTSTRAPFORGE_CONFIGURATION
#define BOOTSTRAPFORGE_CONFIGURATION L"Release"
#endif

namespace
{
    bool HasMicrosoftRuntime()
    {
        const HMODULE runtime = LoadLibraryW(L"VCRUNTIME140_1.dll");
        if (runtime == nullptr)
        {
            return false;
        }
        FreeLibrary(runtime);

        const HMODULE standardLibrary = LoadLibraryW(L"MSVCP140.dll");
        if (standardLibrary == nullptr)
        {
            return false;
        }
        FreeLibrary(standardLibrary);
        return true;
    }

    int ShowLaunchError(const std::wstring& message)
    {
        MessageBoxW(
            nullptr,
            message.c_str(),
            L"BootstrapForge",
            MB_OK | MB_ICONERROR);
        return 1;
    }
}

int WINAPI wWinMain(
    HINSTANCE,
    HINSTANCE,
    PWSTR,
    int)
{
    std::array<wchar_t, 32768> modulePathBuffer{};
    const DWORD modulePathLength = GetModuleFileNameW(
        nullptr,
        modulePathBuffer.data(),
        static_cast<DWORD>(modulePathBuffer.size()));

    if (modulePathLength == 0 || modulePathLength >= modulePathBuffer.size())
    {
        return ShowLaunchError(
            L"BootstrapForge could not determine its installation folder.");
    }

    const std::filesystem::path launcherPath(
        std::wstring(modulePathBuffer.data(), modulePathLength));
    const std::filesystem::path applicationDirectory =
        launcherPath.parent_path() /
        L"bin" /
        BOOTSTRAPFORGE_CONFIGURATION;
    const std::filesystem::path applicationPath =
        applicationDirectory / L"BootstrapForge.Client.exe";

    if (!HasMicrosoftRuntime())
    {
        const std::filesystem::path redistributablePath =
            applicationDirectory / L"vc_redist.x64.exe";
        if (!std::filesystem::is_regular_file(redistributablePath))
        {
            return ShowLaunchError(
                L"BootstrapForge requires the Microsoft Visual C++ 2015-2022 "
                L"x64 Redistributable. Install it, then launch BootstrapForge again.");
        }

        const int answer = MessageBoxW(
            nullptr,
            L"BootstrapForge needs the Microsoft Visual C++ 2015-2022 "
            L"x64 Redistributable. Install it now?",
            L"BootstrapForge Runtime Setup",
            MB_YESNO | MB_ICONINFORMATION | MB_DEFBUTTON1);
        if (answer == IDYES)
        {
            const auto setupResult = ShellExecuteW(
                nullptr,
                L"open",
                redistributablePath.c_str(),
                nullptr,
                applicationDirectory.c_str(),
                SW_SHOWNORMAL);
            if (reinterpret_cast<INT_PTR>(setupResult) <= 32)
            {
                return ShowLaunchError(
                    L"Windows could not start the Visual C++ Redistributable installer.");
            }
        }
        return 0;
    }

    if (!std::filesystem::is_regular_file(applicationPath))
    {
        return ShowLaunchError(
            L"The BootstrapForge application package is missing.\n\n"
            L"Expected to find:\n" + applicationPath.wstring() +
            L"\n\nBuild the BootstrapForge.Client and BootstrapForge.Launcher targets "
            L"or restore the complete bin\\Release folder.");
    }

    const auto result = ShellExecuteW(
        nullptr,
        L"open",
        applicationPath.c_str(),
        nullptr,
        applicationDirectory.c_str(),
        SW_SHOWNORMAL);

    if (reinterpret_cast<INT_PTR>(result) <= 32)
    {
        return ShowLaunchError(
            L"Windows could not start BootstrapForge.Client.exe.\n\n" +
            applicationPath.wstring());
    }

    return 0;
}
