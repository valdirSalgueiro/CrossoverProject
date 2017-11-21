#include "stdafx.h"
#include "Client.h"
#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
#include <queue>
#include <Psapi.h>
#include <iostream>
#include <strsafe.h>
using namespace Gdiplus;
#pragma comment (lib,"Gdiplus.lib")


#define MAX_LOADSTRING 100

HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];

ATOM                RegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

size_t total_virtual_memory_statistic = 0;
size_t virtual_memory_currently_used_statistic = 0;
size_t virtual_memory_currently_used_by_current_process_statistic = 0;
size_t total_physical_memory_statistic = 0;
size_t physical_memory_currently_used_statistic = 0;
size_t physical_memory_currently_used_by_current_process_statistic = 0;

float physical_memory_percent = 0;
float virtual_memory_percent = 0;


// Sample custom data structure for threads to use.
// This is passed by void pointer so it can be any data type
// that can be passed using a single void pointer (LPVOID).
typedef struct MyData {
	int val1;
	int val2;
} MYDATA, *PMYDATA;

PMYDATA pDataArray;
DWORD   dwThreadIdArray;
HANDLE  hThreadArray;

#define BUF_SIZE 255

size_t total_virtual_memory();
size_t virtual_memory_currently_used();
size_t virtual_memory_currently_used_by_current_process();
size_t total_physical_memory();
size_t physical_memory_currently_used();
size_t physical_memory_currently_used_by_current_process();

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

	total_virtual_memory_statistic = total_physical_memory();
	virtual_memory_currently_used_statistic = virtual_memory_currently_used();
	virtual_memory_currently_used_by_current_process_statistic = virtual_memory_currently_used_by_current_process();
	total_physical_memory_statistic = total_physical_memory();
	physical_memory_currently_used_statistic = physical_memory_currently_used();
	physical_memory_currently_used_by_current_process_statistic = physical_memory_currently_used_by_current_process();

	physical_memory_percent = (float)physical_memory_currently_used_statistic / (float)total_physical_memory_statistic;
	virtual_memory_percent = (float)virtual_memory_currently_used_statistic / (float)total_physical_memory_statistic;

	return 0;
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

int idTimer = -1;
HWND hWnd;
iterable_queue<int> myqueue;

void OnPaint(HDC hdc)
{
	Graphics graphics(hdc);
	Pen black(Color(255, 0, 0, 0), 2);
	graphics.Clear(Color::White);
	graphics.SetSmoothingMode(SmoothingMode::SmoothingModeAntiAlias);
	int x = 0;
	int lastY = 0;
	for (auto it = myqueue.begin(); it != myqueue.end(); ++it) {
		int y = *it;
		graphics.DrawLine(&black, x, 100 - lastY, x + 11, 100 - y);
		lastY = y;
		x += 10;
	}

	// Initialize arguments.
	Font font(L"Arial", 16);
	StringFormat format;
	//format.SetAlignment(StringAlignmentCenter);
	SolidBrush blackBrush(Color(255, 0, 0, 0));

	WCHAR memoryStr[][] = { TEXT("Memory usage") };

	RectF memoryUsageRect(0.0f, 0.0f, 200.0f, 50.0f);
	WCHAR memoryStr[] = L"Memory usage";
	graphics.DrawString(memoryStr, lstrlenW(memoryStr),
		&font,
		memoryUsageRect,
		&format,
		&blackBrush);

	WCHAR cpuUsageStr[] = L"CPU usage";
	RectF cpuUsageRect(0.0f, 100.0f, 200.0f, 50.0f);
	graphics.DrawString(cpuUsageStr, lstrlenW(cpuUsageStr),
		&font,
		cpuUsageRect, 
		&format,
		&blackBrush);

	WCHAR NumberOfProcessesStr[] = L"Number of processes";
	RectF numberOfProcessesRect(0.0f, 200.0f, 200.0f, 50.0f);
	graphics.DrawString(NumberOfProcessesStr, lstrlenW(NumberOfProcessesStr),
		&font,
		numberOfProcessesRect,
		&format,
		&blackBrush);

	WCHAR TotalVirtualMemoryStr[] = L"total_virtual_memory";
	RectF TotalVirtualMemoryRect(0.0f, 200.0f, 200.0f, 50.0f);
	wchar_t value[256];

	graphics.DrawString(TotalVirtualMemoryStr, lstrlenW(TotalVirtualMemoryStr),
		&font,
		TotalVirtualMemoryRect,
		&format,
		&blackBrush);

	RectF TotalVirtualMemoryValueRect(500.0f, 200.0f, 200.0f, 50.0f);
	swprintf_s(value, L"%u", total_virtual_memory_statistic);
	graphics.DrawString(value, lstrlenW(value),
		&font,
		TotalVirtualMemoryValueRect,
		&format,
		&blackBrush);

	WCHAR VirtualMemoryCurrentlyUsedStr[] = L"virtual_memory_currently_used";
	RectF VirtualMemoryCurrentlyUsedRect(0.0f, 200.0f, 200.0f, 50.0f);
	graphics.DrawString(VirtualMemoryCurrentlyUsedStr, lstrlenW(VirtualMemoryCurrentlyUsedStr),
		&font,
		VirtualMemoryCurrentlyUsedRect,
		&format,
		&blackBrush);

	WCHAR VirtualMemoryCurrentlyUsedByCurrentProcessStr[] = L"virtual_memory_currently_used_by_current_process";
	RectF VirtualMemoryCurrentlyUsedByCurrentProcessRect(0.0f, 200.0f, 200.0f, 50.0f);
	graphics.DrawString(VirtualMemoryCurrentlyUsedByCurrentProcessStr, lstrlenW(VirtualMemoryCurrentlyUsedByCurrentProcessStr),
		&font,
		VirtualMemoryCurrentlyUsedByCurrentProcessRect,
		&format,
		&blackBrush);

	WCHAR TotalPhysicalMemoryStr[] = L"Total Physical Memory(RAM)";
	RectF TotalPhysicalMemoryRect(0.0f, 200.0f, 200.0f, 50.0f);
	graphics.DrawString(TotalPhysicalMemoryStr, lstrlenW(TotalPhysicalMemoryStr),
		&font,
		TotalPhysicalMemoryRect,
		&format,
		&blackBrush);

	WCHAR PhysicalMemoryCurrentlyUsedStr[] = L"Physical Memory currently used";
	RectF PhysicalMemoryCurrentlyUsedRect(0.0f, 200.0f, 200.0f, 50.0f);
	graphics.DrawString(PhysicalMemoryCurrentlyUsedStr, lstrlenW(PhysicalMemoryCurrentlyUsedStr),
		&font,
		PhysicalMemoryCurrentlyUsedRect,
		&format,
		&blackBrush);

	WCHAR PhysicalMemoryCurrentlyUsedByCurrentProcessStr[] = L"Physical Memory currently used by current process";
	RectF PhysicalMemoryCurrentlyUsedrect(0.0f, 200.0f, 200.0f, 50.0f);
	graphics.DrawString(PhysicalMemoryCurrentlyUsedByCurrentProcessStr, lstrlenW(PhysicalMemoryCurrentlyUsedByCurrentProcessStr),
		&font,
		PhysicalMemoryCurrentlyUsedrect,
		&format,
		&blackBrush);

}

#include <time.h>

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	srand((unsigned)time(NULL));

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR           gdiplusToken;

	// Initialize GDI+.
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

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
	hInst = hInstance; // Salvar gerenciador de instância na variável global

	hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	SetTimer(hWnd, idTimer = 1, 1000, NULL);

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
		// Interpreta as seleções do menu:
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
	case WM_SIZE:
		switch (wParam)
		{
		case SIZE_MINIMIZED:
			// Stop the timer if the window is minimized.

			KillTimer(hWnd, 1);
			idTimer = -1;
			break;

		case SIZE_RESTORED:
			// Fall through to the next case.  

		case SIZE_MAXIMIZED:

			// Start the timer if it had been stopped.  

			if (idTimer == -1)
				SetTimer(hWnd, idTimer = 1, 1000 / 30, NULL);
			break;
		}
		return 0L;
	case WM_TIMER:
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
		if (myqueue.size() > 10)
			myqueue.pop();
		myqueue.push(rand() % 10);
		break;

		//case WM_ERASEBKGND:
		//	return DefWindowProc(hWnd, message, wParam, lParam);

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
