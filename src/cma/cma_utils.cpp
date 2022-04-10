#include "cma_utils.h"

#include "mimalloc.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

HANDLE memoryCollectorThread;
DWORD WINAPI MemoryAllocatorThread(LPVOID lpParam);

int CmaGetReservedHugePagesCount(void)
{
#ifdef WITH_LOCK_PAGES
	MEMORYSTATUSEX mem_status;
	mem_status.dwLength = sizeof(mem_status);

	GlobalMemoryStatusEx(&mem_status);

	if (mem_status.ullAvailPhys > 17179869184) // Avail > 16G
		return 8;

	if (mem_status.ullAvailPhys > 12884901888) // Avail > 12G
		return 6;

	if (mem_status.ullAvailPhys > 8589934592) // Avail > 8G
		return 4;
#endif // WITH_LOCK_PAGES

	return 0;
}

void CmaCreateMemoryCollectorThread(void)
{
    memoryCollectorThread = CreateThread(NULL, 0, MemoryAllocatorThread, NULL, 0, NULL);
	SetThreadPriority(memoryCollectorThread, THREAD_MODE_BACKGROUND_BEGIN);
}

void CmaTerminateMemoryCollectorThread(void)
{
	TerminateThread(memoryCollectorThread, 0);
	CloseHandle(memoryCollectorThread);
}

DWORD WINAPI MemoryAllocatorThread(LPVOID /*lpParam*/)
{
	while (true)
	{
		Sleep(300000); // Force collect memory every 5 minutes

		mi_collect(false);
	}
}
