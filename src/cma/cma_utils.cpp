#include "cma_utils.h"

#include "mimalloc.h"
#include "mimalloc/types.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

int CmaGetReservedHugePagesCount(void)
{
	MEMORYSTATUSEX mem_status{};
	mem_status.dwLength = sizeof(mem_status);

	GlobalMemoryStatusEx(&mem_status);

	if (mem_status.ullAvailPhys > 17179869184) // Avail > 16G
		return 8;

	if (mem_status.ullAvailPhys > 12884901888) // Avail > 12G
		return 6;

	if (mem_status.ullAvailPhys > 8589934592) // Avail > 8G
		return 4;

	return 0;
}

size_t CmaGetReservedOsMemorySize(void)
{
	MEMORYSTATUSEX mem_status{};
	mem_status.dwLength = sizeof(mem_status);

	GlobalMemoryStatusEx(&mem_status);

	if (mem_status.ullAvailPhys > 8589934592) // Avail > 8G
		return min((floor(mem_status.ullAvailPhys / (MI_GiB * MI_SIZE_BITS)) - 2) * MI_GiB, 8 * MI_GiB); // (Avail - 2G) but no more than 8G

	return 0;
}

void CmaSetMemoryAllocatorRuntimeOptions(void)
{
	mi_option_enable(mi_option_large_os_pages); // Enable large pages by default

#ifdef CMA_LOCK_PAGES // Enable reserved large pages on CMA_LOCK_PAGES build variant
	if (mi_option_is_enabled(mi_option_large_os_pages))
		mi_option_set(mi_option_reserve_huge_os_pages, CmaGetReservedHugePagesCount());
#endif
}

DWORD WINAPI ScheduledMemoryCollectorThread(LPVOID /*lpParam*/)
{
	while (true) // Collect memory every 5 minutes
	{
		Sleep(300000);

		mi_collect(false);
	}
}

HANDLE scheduled_memory_collector_thread;

void CmaCreateScheduledMemoryCollectorThread(void)
{
	scheduled_memory_collector_thread = CreateThread(NULL, 0, ScheduledMemoryCollectorThread, NULL, 0, NULL);
	SetThreadPriority(scheduled_memory_collector_thread, THREAD_MODE_BACKGROUND_BEGIN);
}

void CmaTerminateScheduledMemoryCollectorThread(void)
{
	TerminateThread(scheduled_memory_collector_thread, 0);
	CloseHandle(scheduled_memory_collector_thread);
}
