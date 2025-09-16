#include "hooks.h"
#include <thread>
#include <atomic>

HHOOK hHook;
std::atomic<bool> running;

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT* p = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
        // Process key -> send to your mapping system

        std::cout << "I have recieved input" << p->vkCode << std::endl;

        bool swallow = processKeyEvent(p->vkCode, wParam); 

        if (swallow && config["input_sink"].get<bool>()) {
            return 1; // hide from other apps
        }
    }
    return CallNextHookEx(hHook, nCode, wParam, lParam);
}

void WorkerIntegration() {
    hHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, GetModuleHandle(NULL), 0);
    if (!hHook) {
        std::cerr << "Failed to install keyboard hook\n";
        return;
    } else {
        std::cout << "Successfully init keyboard hook\n";
    }

    MSG msg;
    while (running && GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(hHook);
}

void startKeyboardHook() {
    running = true;
    std::thread hookThread(WorkerIntegration);
    hookThread.detach(); 
}

void stopKeyboardHook() {
    running = false;
    // Post a dummy message to wake GetMessage up so it can exit
    PostThreadMessage(GetThreadId(GetCurrentThread()), WM_QUIT, 0, 0);
}