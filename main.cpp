#pragma comment(lib, "Shlwapi.lib")

#define UNICODE
#define NOMINMAX
#include <windows.h>
#include <wrl.h>
#include <WebView2.h>
#include <chrono>
#include <thread>
#include <shlwapi.h>
#include <algorithm>
#include <iostream>
#include <map>
#include <functional>
#include "Application.hpp"

using namespace Microsoft::WRL;
using namespace std;

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow) {
    Application app(hInstance);
    app.run();
    return 0;
}
