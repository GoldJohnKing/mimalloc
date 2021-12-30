#include "cma_utils.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

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
