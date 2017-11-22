#include "stdafx.h"

#include "spdlog/spdlog.h"

#include "Client.h"

#include "client_http.hpp"
#include "server_http.hpp"

#include "json.hpp"
#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
#include <queue>
#include <Psapi.h>
#include <iostream>
#include <strsafe.h>
#include "tinyxml2.h"


using namespace Gdiplus;
#pragma comment (lib,"Gdiplus.lib")
#pragma comment (lib,"Pdh.lib")
using namespace std;
namespace spd = spdlog;
using json = nlohmann::json;

// Use to convert bytes to MB
#define DIV 1048576

#define MAX_LOADSTRING 100

HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];

ATOM                RegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

using HttpClient = SimpleWeb::Client<SimpleWeb::HTTP>;
HttpClient* client;

size_t total_virtual_memory_statistic = 0;
size_t virtual_memory_currently_used_statistic = 0;
size_t virtual_memory_currently_used_by_current_process_statistic = 0;
size_t total_physical_memory_statistic = 0;
size_t physical_memory_currently_used_statistic = 0;
size_t physical_memory_currently_used_by_current_process_statistic = 0;
size_t cpuValue = 0;
size_t numberProcesses = 0;


float physical_memory_percent = 0;
float virtual_memory_percent = 0;
float memory_percent_statistic = 0;

typedef struct MyData {
	int val1;
	int val2;
} MYDATA, *PMYDATA;

PMYDATA pDataArray;
DWORD   dwThreadIdArray;
HANDLE  hThreadArray;

#define BUF_SIZE 255

std::shared_ptr<spdlog::logger> logger;

LONG GetStringRegKey(HKEY hKey, const std::wstring &strValueName, std::wstring &strValue, const std::wstring &strDefaultValue)
{
	strValue = strDefaultValue;
	WCHAR szBuffer[512];
	DWORD dwBufferSize = sizeof(szBuffer);
	ULONG nError;
	DWORD dataType = REG_SZ;
	nError = RegQueryValueExW(hKey, strValueName.c_str(), NULL, &dataType, (LPBYTE)szBuffer, &dwBufferSize);
	if (ERROR_SUCCESS == nError)
	{
		strValue = szBuffer;
	}
	return nError;
}

size_t total_virtual_memory();
size_t virtual_memory_currently_used();
size_t virtual_memory_currently_used_by_current_process();
size_t total_physical_memory();
size_t physical_memory_currently_used();
size_t physical_memory_currently_used_by_current_process();
size_t memory_percent();
double getCpuCurrentValue();
size_t number_of_processes();
string guid;

DWORD WINAPI MyThreadFunction(LPVOID lpParam)
{
	HANDLE hStdout;
	PMYDATA pDataArray;

	TCHAR msgBuf[BUF_SIZE];
	size_t cchStringSize;
	DWORD dwChars;

	pDataArray = (PMYDATA)lpParam;

	StringCchPrintf(msgBuf, BUF_SIZE, TEXT("Parameters = %d, %d\n"),
		pDataArray->val1, pDataArray->val2);
	StringCchLength(msgBuf, BUF_SIZE, &cchStringSize);

	total_virtual_memory_statistic = total_physical_memory() / DIV;
	virtual_memory_currently_used_statistic = virtual_memory_currently_used() / DIV;
	virtual_memory_currently_used_by_current_process_statistic = virtual_memory_currently_used_by_current_process() / DIV;
	total_physical_memory_statistic = total_physical_memory() / DIV;
	physical_memory_currently_used_statistic = physical_memory_currently_used() / DIV;
	physical_memory_currently_used_by_current_process_statistic = physical_memory_currently_used_by_current_process() / DIV;

	physical_memory_percent = (float)physical_memory_currently_used_statistic / (float)total_physical_memory_statistic;
	virtual_memory_percent = (float)virtual_memory_currently_used_statistic / (float)total_physical_memory_statistic;
	memory_percent_statistic = memory_percent();
	cpuValue = getCpuCurrentValue();
	numberProcesses = number_of_processes();

	return 0;
}

#include "pdh.h"

static PDH_HQUERY cpuQuery;
static PDH_HCOUNTER cpuTotal;
std::wstring clientGUID;
std::string key;
std::string host;
std::string email;
std::map<std::string, std::string> alerts;

void readConfiguration() 
{
	tinyxml2::XMLDocument doc;
	doc.LoadFile("config.xml");	
	auto client = doc.FirstChildElement("client");

	key = client->Attribute("key");
	host = client->Attribute("host");
	email = client->Attribute("mail");

	for (tinyxml2::XMLElement* e = client->FirstChildElement("alert"); e != NULL; e = e->NextSiblingElement("alert"))
	{
		alerts[e->Attribute("type")]= e->Attribute("limit");
	}
}

void initCpu() {
	PdhOpenQuery(NULL, NULL, &cpuQuery);
	PdhAddEnglishCounter(cpuQuery, "\\Processor(_Total)\\% Processor Time", NULL, &cpuTotal);
	PdhCollectQueryData(cpuQuery);
}

double getCpuCurrentValue() {
	PDH_FMT_COUNTERVALUE counterVal;

	PdhCollectQueryData(cpuQuery);
	PdhGetFormattedCounterValue(cpuTotal, PDH_FMT_DOUBLE, NULL, &counterVal);
	return counterVal.doubleValue;
}

static size_t total_virtual_memory()
{
	MEMORYSTATUSEX mem_info;
	mem_info.dwLength = sizeof(mem_info);
	GlobalMemoryStatusEx(&mem_info);
	return mem_info.ullTotalPageFile;
}
static size_t virtual_memory_currently_used()
{
	MEMORYSTATUSEX mem_info;
	mem_info.dwLength = sizeof(mem_info);
	GlobalMemoryStatusEx(&mem_info);
	return mem_info.ullTotalPageFile - mem_info.ullAvailPageFile;
}
static size_t virtual_memory_currently_used_by_current_process()
{
	PROCESS_MEMORY_COUNTERS_EX pmc;
	pmc.cb = sizeof(pmc);
	GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
	return pmc.PrivateUsage;
}

static size_t total_physical_memory()
{
	MEMORYSTATUSEX mem_info;
	mem_info.dwLength = sizeof(mem_info);
	GlobalMemoryStatusEx(&mem_info);
	return mem_info.ullTotalPhys;
}
static size_t physical_memory_currently_used()
{
	MEMORYSTATUSEX mem_info;
	mem_info.dwLength = sizeof(mem_info);
	GlobalMemoryStatusEx(&mem_info);
	return mem_info.ullTotalPhys - mem_info.ullAvailPhys;
}
static size_t physical_memory_currently_used_by_current_process()
{
	PROCESS_MEMORY_COUNTERS_EX pmc;
	pmc.cb = sizeof(pmc);
	GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
	return pmc.PrivateUsage;
}
static size_t memory_percent()
{
	MEMORYSTATUSEX mem_info;
	mem_info.dwLength = sizeof(mem_info);
	GlobalMemoryStatusEx(&mem_info);
	return mem_info.dwMemoryLoad;
}

static size_t number_of_processes()
{
	DWORD aProcesses[1024], cbNeeded, cProcesses;
	unsigned int i;

	if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded))
	{
		return 1;
	}
	// Calculate how many process identifiers were returned.
	return (cbNeeded / sizeof(DWORD));
}

template<typename T, typename Container = std::deque<T> >
class iterable_queue : public std::queue<T, Container>
{
public:
	typedef typename Container::iterator iterator;
	typedef typename Container::const_iterator const_iterator;

	iterator begin() { return this->c.begin(); }
	iterator end() { return this->c.end(); }
	const_iterator begin() const { return this->c.begin(); }
	const_iterator end() const { return this->c.end(); }
};

const int ID_TIMER_REFRESH = 1;
const int ID_TIMER_SEND_STATISTICS = 2;
HWND hWnd;
iterable_queue<int> memoryQueue;

void OnPaint(HDC hdc)
{
	Graphics graphics(hdc);
	Pen black(Color(255, 0, 0, 0), 2);
	graphics.Clear(Color::White);
	graphics.SetSmoothingMode(SmoothingMode::SmoothingModeAntiAlias);

	Font font(L"Arial", 16);
	StringFormat format;
	SolidBrush blackBrush(Color(255, 0, 0, 0));

	Font fontMarker(L"Arial", 8);
	SolidBrush redBrush(Color(255, 255, 0, 0));

	WCHAR Str[9][255] = {
		L"Total Virtual Memory", L"Virtual Memory Currently Used",
		L"Virtual Memory Currently Used by Current Process", L"Total Physical Memory(RAM)",
		L"Physical Memory Currently Used", L"Physical Memory Currently Used by Current Process",
		L"CPU Usage",
		L"Number of Processes",
		L"Memory Usage",
	};

	int values[8] = {
		total_virtual_memory_statistic,
		virtual_memory_currently_used_statistic,
		virtual_memory_currently_used_by_current_process_statistic,
		total_physical_memory_statistic,
		physical_memory_currently_used_statistic,
		physical_memory_currently_used_by_current_process_statistic,
		cpuValue,
		numberProcesses
	};

	int rectX = 0;
	int rectY = 0;

	int COLS = 3;

	for (int i = 1; i <= 9; i++)
	{
		RectF rect(rectX, rectY, 600, 50);
		wchar_t value[256];

		graphics.DrawString(Str[i - 1], lstrlenW(Str[i - 1]),
			&font,
			rect,
			&format,
			&blackBrush);

		if (i <= 8)
		{
			RectF rectValue(rectX, rectY + 25, 200, 50);
			if (i <= 6)
				swprintf_s(value, L"%u MB", values[i - 1]);
			else
				swprintf_s(value, L"%u", values[i - 1]);
			graphics.DrawString(value, lstrlenW(value),
				&font,
				rectValue,
				&format,
				&blackBrush);
		}
		else
		{
			graphics.DrawString(L"100%", lstrlenW(L"100%"),
				&fontMarker,
				RectF(rectX - 30, rectY + 25, 50, 20),
				&format,
				&redBrush);
			graphics.DrawString(L"0%", lstrlenW(L"0%"),
				&fontMarker,
				RectF(rectX - 30, rectY + 65, 50, 20),
				&format,
				&redBrush);
			if (memoryQueue.size() > 1) {
				int lastY = memoryQueue.back();
				int x = 0;
				for (auto it = memoryQueue.begin(); it != memoryQueue.end(); ++it) {
					int y = *it;
					graphics.DrawLine(&black, rectX + x, rectY + 65 - lastY, rectX + x + 11, rectY + 65 - y);
					lastY = y;
					x += 10;
				}
			}
		}

		rectX += 400;

		if (i%COLS == 0)
		{
			rectY += 100;
			rectX = 0;
		}
	}


	std::wcin >> clientGUID;  
	const WCHAR * clientGUIDStr = clientGUID.c_str();

	graphics.DrawString(L"GUID", lstrlenW(L"GUID"),
		&font,
		RectF(10, rectY + 50, 100, 20),
		&format,
		&redBrush);

	graphics.DrawString(clientGUIDStr, lstrlenW(clientGUIDStr),
		&font,
		RectF(100, rectY + 50, 300, 20),
		&format,
		&redBrush);

}

void getComputerGUID() {
	HKEY hKey;
	LONG lRes = RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Cryptography", 0, KEY_READ, &hKey);
	bool bExistsAndSuccess(lRes == ERROR_SUCCESS);
	bool bDoesNotExistsSpecifically(lRes == ERROR_FILE_NOT_FOUND);
	GetStringRegKey(hKey, L"MachineGuid", clientGUID, L"bad");
}

#include <time.h>

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR           gdiplusToken;

	// Initialize GDI+.
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	readConfiguration();

	json j;
	j["guid"] = key;
	j["type"] = "authenticate";
	j["email"] = email;
	j["alert"]["memory"] = alerts["memory"];
	j["alert"]["cpu"] = alerts["cpu"];
	j["alert"]["processes"] = alerts["processes"];
	client = new HttpClient(host);
	auto r = client->request("POST", "/json", j.dump());
	cout << r->content.rdbuf() << endl; // Alternatively, use the convenience function r1->content.string()

	initCpu();

	getComputerGUID();


	logger = spd::basic_logger_mt("crossover computer monitoring", "logs/log.txt");

	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_CLIENT, szWindowClass, MAX_LOADSTRING);
	RegisterClass(hInstance);

	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CLIENT));

	MSG msg;

	// Message loop:
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}



ATOM RegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CLIENT));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_CLIENT);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
	//wcex.hbrBackground = NULL;

	return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance;

	hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	SetTimer(hWnd, ID_TIMER_REFRESH, 1000, NULL);
	SetTimer(hWnd, ID_TIMER_SEND_STATISTICS, 5000, NULL);
	//SetTimer(hWnd, ID_TIMER_SEND_STATISTICS, 300000, NULL); // 5minutes

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);

		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		RECT Client_Rect;
		GetClientRect(hWnd, &Client_Rect);
		int win_width = Client_Rect.right - Client_Rect.left;
		int win_height = Client_Rect.bottom + Client_Rect.left;
		HDC Memhdc;
		HDC hdc;
		HBITMAP Membitmap;
		hdc = BeginPaint(hWnd, &ps);
		Memhdc = CreateCompatibleDC(hdc);
		Membitmap = CreateCompatibleBitmap(hdc, win_width, win_height);
		SelectObject(Memhdc, Membitmap);
		OnPaint(Memhdc);
		BitBlt(hdc, 0, 0, win_width, win_height, Memhdc, 0, 0, SRCCOPY);
		DeleteObject(Membitmap);
		DeleteDC(Memhdc);
		DeleteDC(hdc);
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_TIMER:
		switch (wParam)
		{
		case ID_TIMER_REFRESH:
			pDataArray = (PMYDATA)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
				sizeof(MYDATA));

			if (pDataArray == NULL)
			{
				// If the array allocation fails, the system is out of memory
				// so there is no point in trying to print an error message.
				// Just terminate execution.
				ExitProcess(2);
			}

			// Generate unique data for each thread to work with.

			pDataArray->val1 = 2;
			pDataArray->val2 = 100;

			// Create the thread to begin execution on its own.

			hThreadArray = CreateThread(
				NULL,                   // default security attributes
				0,                      // use default stack size  
				MyThreadFunction,       // thread function name
				pDataArray,          // argument to thread function 
				0,                      // use default creation flags 
				&dwThreadIdArray);   // returns the thread identifier 

			InvalidateRect(hWnd, NULL, FALSE);
			UpdateWindow(hWnd);
			if (memoryQueue.size() > 10)
				memoryQueue.pop();
			memoryQueue.push(memory_percent_statistic / 10);
			return 0;
		case ID_TIMER_SEND_STATISTICS:
			//async request
			json j;
			j["guid"] = key;
			j["type"] = "stats";
			j["memory"] = (int)memory_percent_statistic;
			j["cpu"] = (int)cpuValue;
			j["number_processes"] = (int)numberProcesses;
			auto r = client->request("POST", "/json", j.dump());
			cout << r->content.rdbuf() << endl; // Alternatively, use the convenience function r1->content.string()
			return 0;
		}


	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
