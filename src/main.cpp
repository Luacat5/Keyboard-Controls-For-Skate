// vigem_setup.cpp
// Minimal: only ensures ViGEmBus driver is installed and attaches a virtual X360 pad.
// You add your own control logic afterwards.

#include <windows.h>
#include <shlobj.h>
#include <shellapi.h>
#include <winhttp.h>
#include <iostream>
#include <vector>
#include <vigem/Client.h>
#include <Xinput.h>

#include "logic.h" //my custom controller logicz

bool vigemDriverPresent()
{
    PVIGEM_CLIENT client = vigem_alloc();
    if (!client) return false;
    VIGEM_ERROR err = vigem_connect(client);
    if (!VIGEM_SUCCESS(err)) {
        vigem_free(client);
        return false;
    }
    vigem_disconnect(client);
    vigem_free(client);
    return true;
}

bool downloadAndRunViGEmInstaller()
{
    const wchar_t* host = L"github.com";
    const wchar_t* path = L"/nefarius/ViGEmBus/releases/download/v1.22.0/ViGEmBus_1.22.0_x64_x86_arm64.exe";

    HINTERNET hSession = WinHttpOpen(L"ViGEmInstaller/1.0",
                                     WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                     WINHTTP_NO_PROXY_NAME,
                                     WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) return false;

    HINTERNET hConnect = WinHttpConnect(hSession, host, INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!hConnect) { WinHttpCloseHandle(hSession); return false; }

    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", path,
                                            NULL, WINHTTP_NO_REFERER,
                                            WINHTTP_DEFAULT_ACCEPT_TYPES,
                                            WINHTTP_FLAG_SECURE);
    if (!hRequest) { WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession); return false; }

    if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                            WINHTTP_NO_REQUEST_DATA, 0, 0, 0)) {
        WinHttpCloseHandle(hRequest); WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession);
        return false;
    }
    if (!WinHttpReceiveResponse(hRequest, NULL)) {
        WinHttpCloseHandle(hRequest); WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession);
        return false;
    }

    wchar_t tempPath[MAX_PATH];
    GetTempPathW(MAX_PATH, tempPath);
    std::wstring outFile = std::wstring(tempPath) + L"ViGEmBus_Setup.exe";

    HANDLE hFile = CreateFileW(outFile.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                               FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        WinHttpCloseHandle(hRequest); WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession);
        return false;
    }

    DWORD bytesRead;
    std::vector<char> buffer(8192);
    while (true) {
        DWORD dwSize = 0;
        if (!WinHttpQueryDataAvailable(hRequest, &dwSize) || dwSize == 0) break;
        if (!WinHttpReadData(hRequest, buffer.data(), dwSize, &bytesRead) || bytesRead == 0) break;
        DWORD written;
        WriteFile(hFile, buffer.data(), bytesRead, &written, NULL);
    }
    CloseHandle(hFile);
    WinHttpCloseHandle(hRequest); WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession);

    // Run installer (with elevation)
    SHELLEXECUTEINFOW sei = { sizeof(sei) };
    sei.fMask = SEE_MASK_NOCLOSEPROCESS;
    sei.lpVerb = L"runas";
    sei.lpFile = outFile.c_str();
    sei.nShow = SW_SHOWNORMAL;
    if (!ShellExecuteExW(&sei)) return false;

    if (sei.hProcess) {
        WaitForSingleObject(sei.hProcess, INFINITE);
        CloseHandle(sei.hProcess);
    }
    return true;
}



int main()
{
    std::cout << "Checking ViGEmBus driver...\n";

    if (!vigemDriverPresent()) {
        std::cout << "Driver not found. Attempting to install...\n";
        if (!downloadAndRunViGEmInstaller()) {
            std::cerr << "Failed to install ViGEmBus automatically.\n";
            return 1;
        }
        std::cout << "Installer completed. Please re-run this program.\n";
        return 0;
    }

    std::cout << "Driver found. Creating virtual controller...\n";
      
    if(!initController()){
        std::cerr << "Failed to init initial controller\n";
        return 1;
    }
    mainLogicLoop();

    return 0;
}
