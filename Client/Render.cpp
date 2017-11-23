#include "stdafx.h"
#include "Render.h"
#include "Utils.h"

using namespace Gdiplus;


Render::Render()
{
	clientGUID = Utils::getComputerGUID();
}

Render::~Render()
{
}

void Render::draw(HDC hdc, Metrics* metrics)
{
	Graphics graphics(hdc);
	Pen black(Color(128, 0, 0, 0), 2);
	graphics.Clear(Color::White);
	graphics.SetSmoothingMode(SmoothingMode::SmoothingModeAntiAlias);

	Font font(L"Arial", 16);
	StringFormat format;
	SolidBrush blackBrush(Color(255, 0, 0, 0));

	SolidBrush grayBrush(Color(128, 0, 0, 0));

	Font fontMarker(L"Arial", 8);
	SolidBrush redBrush(Color(255, 255, 0, 0));

	WCHAR Str[9][255] = {
		L"Total Virtual Memory", L"Virtual Memory Currently Used",
		L"Virtual Memory Currently Used by Current Process", L"Total Physical Memory(RAM)",
		L"Physical Memory Currently Used", L"Physical Memory Currently Used by Current Process",
		L"Number of Processes",
		L"CPU Usage",
		L"Memory Usage",
	};
	
	int values[8] = {
		metrics->getTotalVirtualMemoryStatistic(),
		metrics->getVirtualMemoryCurrentlyUsedStatistic(),
		metrics->getVirtualMemoryCurrentlyUsedByCurrentProcessStatistic(),
		metrics->getTotalPhysicalMemoryStatistic(),
		metrics->getPhysicalMemoryCurrentlyUsedStatistic(),
		metrics->getPhysicalMemoryCurrentlyUsedByCurrentProcessStatistic(),
		metrics->getNumberProcessesStatistic(),
		metrics->getCpuValueStatistic()
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

		if (i <= 7)
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
				&grayBrush);
		}
		else if (i == 8)
		{
			graphics.DrawString(L"100%", lstrlenW(L"100%"),
				&fontMarker,
				RectF(rectX - 30, rectY + 25, 50, 20),
				&format,
				&redBrush);
			graphics.DrawString(L"0%", lstrlenW(L"0%"),
				&fontMarker,
				RectF(rectX - 30, rectY + 125, 50, 20),
				&format,
				&redBrush);
			if (metrics->cpuQueue.size() > 1) {
				int lastY = metrics->cpuQueue.front();
				int x = 0;
				for (auto it = metrics->cpuQueue.begin(); it != metrics->cpuQueue.end(); ++it) {
					int y = *it;
					graphics.DrawLine(&black, rectX + x, rectY + 125 - lastY, rectX + x + 11, rectY + 125 - y);
					lastY = y;
					x += 10;
				}
			}
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
				RectF(rectX - 30, rectY + 125, 50, 20),
				&format,
				&redBrush);
			if (metrics->memoryQueue.size() > 1) {
				int lastY = metrics->memoryQueue.front();
				int x = 0;
				for (auto it = metrics->memoryQueue.begin(); it != metrics->memoryQueue.end(); ++it) {
					int y = *it;
					graphics.DrawLine(&black, rectX + x, rectY + 125 - lastY, rectX + x + 11, rectY + 125 - y);
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
		RectF(0, rectY + 100, 100, 20),
		&format,
		&blackBrush);

	graphics.DrawString(clientGUIDStr, lstrlenW(clientGUIDStr),
		&font,
		RectF(90, rectY + 100, 300, 20),
		&format,
		&redBrush);
}
