#pragma once
#include <windows.h>
#include <memory>
#include <iostream>
#include "OverlayManager.hpp"
#include "ShortcutManager.hpp"
#include "Log.hpp"

// Application.hpp
//
// Entry-point controller for the application.
// - Registers window class, creates the overlay window, and initializes components.
// - Connects hotkeys with overlay behavior via ShortcutManager.
// - Provides a debugging console toggle (Ctrl+F1).

class Application {
public:
    Application(HINSTANCE hInstance) : hInstance(hInstance), hwnd(nullptr) {}
    void run() {
        initConsole();
        registerWindowClass();
        createOverlayWindow();
        overlayManager = std::make_unique<OverlayManager>(hwnd);
        overlayManager->initializeWebView();
        registerHotKeys();
        messageLoop();
    }

private:
    HINSTANCE hInstance;
    HWND hwnd;
    ShortcutManager shortcutManager;
    std::unique_ptr<OverlayManager> overlayManager;

    void initConsole() {
        AllocConsole();
        freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
        freopen_s((FILE**)stderr, "CONOUT$", "w", stderr);
        freopen_s((FILE**)stdin, "CONIN$", "r", stdin);
        Log::d("Application", "App Started");
    }

    void registerWindowClass() {
        const wchar_t CLASS_NAME[] = L"WebView2Overlay";
        WNDCLASS wc = {};
        wc.lpfnWndProc = WndProcStatic;
        wc.hInstance = hInstance;
        wc.lpszClassName = CLASS_NAME;
        RegisterClass(&wc);
    }

    void createOverlayWindow() {
        hwnd = CreateWindowEx(
            WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TRANSPARENT,
            L"WebView2Overlay", L"WebOverlay",
            WS_POPUP,
            0, 0, 500, 600, // TODO make ini
            nullptr, nullptr, hInstance, this
        );
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)this);
        SetLayeredWindowAttributes(hwnd, 0, 180, LWA_ALPHA);
        ShowWindow(hwnd, SW_SHOW);
    }

    void registerHotKeys() {
        shortcutManager.registerHotKey(hwnd, 1, MOD_CONTROL, VK_OEM_3, [this](UINT mod, UINT vk) { overlayManager->toggleOverlay(); });
        shortcutManager.registerHotKey(hwnd, 2, MOD_CONTROL, VK_F1, [this](UINT mod, UINT vk) { overlayManager->toggleClickThrough(); });
        shortcutManager.registerHotKey(hwnd, 3, MOD_CONTROL, VK_F12, [](UINT mod, UINT vk) {
            static bool visible = true;
            visible = !visible;
            ShowWindow(GetConsoleWindow(), visible ? SW_SHOW : SW_HIDE);
        });

        // Key event to Web
        for (int i = 1; i <= 5; ++i) {
            shortcutManager.registerHotKey(
                hwnd, i + 3, MOD_CONTROL, '0' + i,
                [this, i](UINT mod, UINT vk) {
                    overlayManager->dispatchKeyEventToWeb(mod, StringToWString(getKeyName(vk)));
                },
                [this]() {
                    return true;
                }
            );
        }
    }

    void messageLoop() {
        MSG msg;
        while (GetMessage(&msg, nullptr, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    LRESULT wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
        if (message == WM_HOTKEY) {
            shortcutManager.handleHotKey(wParam);
            return 0;
        }
        if (message == WM_DESTROY) {
            PostQuitMessage(0);
            return 0;
        }
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    static LRESULT CALLBACK WndProcStatic(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
        Application* app = reinterpret_cast<Application*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
        if (app) return app->wndProc(hWnd, message, wParam, lParam);
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
};
