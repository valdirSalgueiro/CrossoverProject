#pragma once
#include <IterableQueue.h>

class Metrics
{
public:
	Metrics();
	~Metrics();
	void updateMetrics();
	void initCpu();

	size_t getTotalVirtualMemoryStatistic()
	{
		return totalVirtualMemoryStatistic;
	}
	size_t getVirtualMemoryCurrentlyUsedStatistic()
	{
		return virtualMemoryCurrentlyUsedStatistic;
	}
	size_t getVirtualMemoryCurrentlyUsedByCurrentProcessStatistic()
	{
		return virtualMemoryCurrentlyUsedByCurrentProcessStatistic;
	}
	size_t getTotalPhysicalMemoryStatistic()
	{
		return totalPhysicalMemoryStatistic;
	}
	size_t getPhysicalMemoryCurrentlyUsedStatistic()
	{
		return physicalMemoryCurrentlyUsedStatistic;
	}
	size_t getPhysicalMemoryCurrentlyUsedByCurrentProcessStatistic()
	{
		return physicalMemoryCurrentlyUsedByCurrentProcessStatistic;
	}

	size_t getMemoryPercentStatistic()
	{
		return memoryPercentStatistic;
	}
	double getCpuValueStatistic() {
		return cpuValue;
	}
	size_t getNumberProcessesStatistic()
	{
		return numberProcesses;
	}

	void updateQueues() 
	{
		if (memoryQueue.size() > 10)
			memoryQueue.pop();
		memoryQueue.push(memoryPercentStatistic);

		if (cpuQueue.size() > 10)
			cpuQueue.pop();
		cpuQueue.push(cpuValue);
	}

	iterable_queue<int> memoryQueue;
	iterable_queue<int> cpuQueue;
private:
	PDH_HQUERY cpuQuery;
	PDH_HCOUNTER cpuTotal;


	size_t getTotalVirtualMemory();
	size_t getVirtualMemoryCurrentlyUsed();
	size_t getVirtualMemoryCurrentlyUsedByCurrentProcess();
	size_t getTotalPhysicalMemory();
	size_t getPhysicalMemoryCurrentlyUsed();
	size_t getPhysicalMemoryCurrentlyUsedByCurrentProcess();
	size_t getMemoryPercent();
	double getCpuValue();
	size_t getNumberProcesses();

	size_t totalVirtualMemoryStatistic;
	size_t virtualMemoryCurrentlyUsedStatistic;
	size_t virtualMemoryCurrentlyUsedByCurrentProcessStatistic;
	size_t totalPhysicalMemoryStatistic;
	size_t physicalMemoryCurrentlyUsedStatistic;
	size_t physicalMemoryCurrentlyUsedByCurrentProcessStatistic;

	size_t memoryPercentStatistic;
	size_t cpuValue;
	size_t numberProcesses;



};

