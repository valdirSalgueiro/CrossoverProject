#include "stdafx.h"
#include "Metrics.h"

// Use to convert bytes to MB
#define DIV 1048576

Metrics::Metrics()
{
}


Metrics::~Metrics()
{
}

void Metrics::updateMetrics()
{
	totalVirtualMemoryStatistic = getTotalVirtualMemory() / DIV;
	virtualMemoryCurrentlyUsedStatistic = getVirtualMemoryCurrentlyUsed() / DIV;
	virtualMemoryCurrentlyUsedByCurrentProcessStatistic = getVirtualMemoryCurrentlyUsedByCurrentProcess() / DIV;
	totalPhysicalMemoryStatistic = getTotalPhysicalMemory() / DIV;
	physicalMemoryCurrentlyUsedStatistic = getPhysicalMemoryCurrentlyUsed() / DIV;
	physicalMemoryCurrentlyUsedByCurrentProcessStatistic = getPhysicalMemoryCurrentlyUsedByCurrentProcess() / DIV;

	memoryPercentStatistic = getMemoryPercent();
	cpuValue = getCpuValue();
	numberProcesses = getNumberProcesses();
}

size_t Metrics::getTotalVirtualMemory()
{
	MEMORYSTATUSEX mem_info;
	mem_info.dwLength = sizeof(mem_info);
	GlobalMemoryStatusEx(&mem_info);
	return mem_info.ullTotalPageFile;
}

size_t Metrics::getVirtualMemoryCurrentlyUsed()
{
	MEMORYSTATUSEX mem_info;
	mem_info.dwLength = sizeof(mem_info);
	GlobalMemoryStatusEx(&mem_info);
	return mem_info.ullTotalPageFile - mem_info.ullAvailPageFile;
}

size_t Metrics::getVirtualMemoryCurrentlyUsedByCurrentProcess()
{
	PROCESS_MEMORY_COUNTERS_EX pmc;
	pmc.cb = sizeof(pmc);
	GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
	return pmc.PrivateUsage;
}

size_t Metrics::getTotalPhysicalMemory()
{
	MEMORYSTATUSEX mem_info;
	mem_info.dwLength = sizeof(mem_info);
	GlobalMemoryStatusEx(&mem_info);
	return mem_info.ullTotalPhys;
}

size_t Metrics::getPhysicalMemoryCurrentlyUsed()
{
	MEMORYSTATUSEX mem_info;
	mem_info.dwLength = sizeof(mem_info);
	GlobalMemoryStatusEx(&mem_info);
	return mem_info.ullTotalPhys - mem_info.ullAvailPhys;
}

size_t Metrics::getPhysicalMemoryCurrentlyUsedByCurrentProcess()
{
	PROCESS_MEMORY_COUNTERS_EX pmc;
	pmc.cb = sizeof(pmc);
	GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
	return pmc.PrivateUsage;
}

size_t Metrics::getMemoryPercent()
{
	MEMORYSTATUSEX mem_info;
	mem_info.dwLength = sizeof(mem_info);
	GlobalMemoryStatusEx(&mem_info);
	return mem_info.dwMemoryLoad;
}

double Metrics::getCpuValue()
{
	PDH_FMT_COUNTERVALUE counterVal;

	PdhCollectQueryData(cpuQuery);
	PdhGetFormattedCounterValue(cpuTotal, PDH_FMT_DOUBLE, NULL, &counterVal);
	return counterVal.doubleValue;
}

size_t Metrics::getNumberProcesses()
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

void Metrics::initCpu() {
	PdhOpenQuery(NULL, NULL, &cpuQuery);
	PdhAddEnglishCounter(cpuQuery, "\\Processor(_Total)\\% Processor Time", NULL, &cpuTotal);
	PdhCollectQueryData(cpuQuery);
}

