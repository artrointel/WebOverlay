#pragma once

#include <string>
#include <stringapiset.h>

std::string WStringToString(const std::wstring& wstr) {
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string result(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &result[0], size_needed, nullptr, nullptr);
    return result;
}

std::wstring StringToWString(const std::string& str) {
    if (str.empty()) return L"";

    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
    if (size_needed == 0) return L"";

    std::wstring result(size_needed - 1, 0); // exclude null terminator
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &result[0], size_needed);
    return result;
}

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