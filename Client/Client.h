#pragma once
#include "resource.h"
#include "Utils.h"
#include "Render.h"
#include "ConfigurationManager.h"
#include "Metrics.h"
#include "HttpHelper.h"

#define MAX_LOADSTRING 100

using namespace Gdiplus;
#pragma comment (lib,"Gdiplus.lib")
#pragma comment (lib,"Pdh.lib")

// stl library
using namespace std;

// logging
namespace spd = spdlog;

// timers
const int ID_TIMER_REFRESH = 1;
const int ID_TIMER_SEND_STATISTICS = 2;

// Windows handles
HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];
DWORD   dwThreadId;
HANDLE  hThread;
HWND hWnd;

ATOM                RegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);


// client variables
std::shared_ptr<spdlog::logger> logger;
Render* render;
ConfigurationManager* configuration;
HttpHelper* http;
Metrics* metrics;
