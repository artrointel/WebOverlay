#pragma once
#include <windows.h>
#include <wrl.h>
#include <WebView2.h>
#include <string>
#include <thread>
#include <chrono>
#include <shlwapi.h>
#include <iostream>
#include <algorithm>
#include "String.hpp"
#include "Log.hpp"
#include <mutex>

using namespace Microsoft::WRL;
using namespace std;

// OverlayManager.hpp
//
// Manages the lifecycle and behavior of the overlay window.
// - Initializes and controls a transparent WebView2 window.
// - Handles overlay visibility and user input pass-through toggling.
// - Supports smooth opacity animation with thread safety.

class OverlayManager {
public:
    OverlayManager(HWND hwnd) : hwnd(hwnd), visible(true), clickThrough(true) {}
    ~OverlayManager() {
        cancelOpacityThread.store(true);
        if (opacityThread.joinable()) opacityThread.join();
    }

    void initializeWebView() {
        CreateCoreWebView2Environment(
            Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
                [this](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {
                    env->CreateCoreWebView2Controller(
                        hwnd,
                        Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                            [this](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
                                if (controller) {
                                    webviewController = controller;
                                    controller->get_CoreWebView2(&webviewWindow);
                                }
                                RECT bounds;
                                GetClientRect(hwnd, &bounds);
                                webviewController->put_Bounds(bounds);
                                std::wstring url = getHtmlUrl();
                                Log::d("OverlayManager", "Navigate URL", WStringToString(url));
                                webviewWindow->Navigate(url.c_str());
                                return S_OK;
                            }).Get());
                    return S_OK;
                }).Get());
    }

    void toggleOverlay() {
        visible = !visible;
        ShowWindow(hwnd, visible ? SW_SHOW : SW_HIDE);

        cancelOpacityThread.store(true);
        if (opacityThread.joinable()) opacityThread.join(); // 이전 쓰레드 종료
        cancelOpacityThread.store(false);

        opacityThread = std::thread(&OverlayManager::animateOpacity, this, visible ? 180 : 0);
    }

    void toggleClickThrough() {
        if (!visible) {
            Log::d("OverlayManager", "Overlay is invisible. Skipped Clickthrough. ");
            return;
        }
        clickThrough = !clickThrough;
        applyClickThrough();
    }

private:
    HWND hwnd;
    std::atomic<bool> cancelOpacityThread{ false };
    std::mutex opacityMutex;
    std::thread opacityThread;

    bool visible;
    bool clickThrough;
    ComPtr<ICoreWebView2Controller> webviewController;
    ComPtr<ICoreWebView2> webviewWindow;

    void applyClickThrough() {
        LONG exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
        if (clickThrough)
            exStyle |= WS_EX_TRANSPARENT;
        else
            exStyle &= ~WS_EX_TRANSPARENT;
        SetWindowLong(hwnd, GWL_EXSTYLE, exStyle);
    }

    void animateOpacity(BYTE targetAlpha) {
        std::lock_guard<std::mutex> lock(opacityMutex);

        BYTE current = 0;
        GetLayeredWindowAttributes(hwnd, nullptr, &current, nullptr);
        const int steps = 30;
        const int interval = 10;
        float delta = (targetAlpha - current) / static_cast<float>(steps);

        for (int i = 1; i <= steps; ++i) {
            if (cancelOpacityThread.load()) return;

            BYTE alpha = static_cast<BYTE>(current + delta * i);
            SetLayeredWindowAttributes(hwnd, 0, alpha, LWA_ALPHA);
            std::this_thread::sleep_for(std::chrono::milliseconds(interval));
        }
    }

    std::wstring getHtmlUrl() {
        wchar_t exePath[MAX_PATH];
        GetModuleFileName(nullptr, exePath, MAX_PATH);
        PathRemoveFileSpec(exePath);

        wchar_t fullPath[MAX_PATH];
        PathCombineW(fullPath, exePath, L"..\\..\\resources\\index.html");

        if (!PathFileExistsW(fullPath)) {
            std::wostringstream oss;
            oss << L"index.html does not exist. Path was: " << fullPath;
            Log::e("OverlayManager", WStringToString(oss.str()));
        }

        std::wstring url = L"file:///" + std::wstring(fullPath);
        std::replace(url.begin(), url.end(), L'\\', L'/');
        return url;
    }
};