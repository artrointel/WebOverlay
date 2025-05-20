#pragma once
#include <windows.h>
#include <functional>
#include <map>

// ShortcutManager.hpp
//
// Handles registration and execution of global hotkeys.
// - Maps hotkeys to user-defined callback functions.
// - Provides readable modifier and key name utilities for logging.
// - Designed to work with WM_HOTKEY messages in Windows API.

class ShortcutManager {
public:
    void registerHotKey(
        HWND hwnd,
        int id,
        UINT mod,
        UINT vk,
        std::function<void(UINT, UINT)> action,
        std::function<bool()> condition = nullptr)
    {
        RegisterHotKey(hwnd, id, mod, vk);

        actions[id] = [=]() {
            Log::d("ShortcutManager",
                "Hotkey triggered: ",
                getModifierName(mod), getKeyName(vk));
            if (condition == nullptr) {
                Log::d("ShortcutManager", "Action triggered");
                if (action) action(mod, vk);
            }
            else {
                if (condition()) {
                    Log::d("ShortcutManager", "Action triggered");
                    if (action) action(mod, vk);
                }
            }
        };
    }

    void handleHotKey(WPARAM id) {
        if (actions.count((int)id)) actions[(int)id]();
    }

private:
    std::map<int, std::function<void()>> actions;
};