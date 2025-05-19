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
    void registerHotKey(HWND hwnd, int id, UINT mod, UINT vk, std::function<void()> action) {
        RegisterHotKey(hwnd, id, mod, vk);
        actions[id] = [=]() {
            Log::d("ShortcutManager", 
                "Hotkey triggered: ", getModifierName(mod), getKeyName(vk));
            action();
        };
    }

    void handleHotKey(WPARAM id) {
        if (actions.count((int)id)) actions[(int)id]();
    }

private:
    std::map<int, std::function<void()>> actions;

    static std::string getModifierName(UINT mod) {
        std::ostringstream oss;
        if (mod & MOD_CONTROL) oss << "Ctrl+";
        if (mod & MOD_ALT)     oss << "Alt+";
        if (mod & MOD_SHIFT)   oss << "Shift+";
        if (mod & MOD_WIN)     oss << "Win+";
        return oss.str();
    }

    static std::string getKeyName(UINT vk) {
        char name[128] = { 0 };
        UINT scanCode = MapVirtualKey(vk, MAPVK_VK_TO_VSC) << 16;
        if (GetKeyNameTextA(scanCode, name, sizeof(name))) {
            return std::string(name);
        }
        std::ostringstream oss;
        oss << "VK_0x" << std::hex << std::uppercase << vk;
        return oss.str();
    }
};