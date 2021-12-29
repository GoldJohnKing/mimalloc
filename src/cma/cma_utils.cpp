#include "cma_utils.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

int CmaGetReservedHugePagesCount(void)
{

#ifdef WITH_LOCK_PAGES

	MEMORYSTATUSEX mem_status;
	mem_status.dwLength = sizeof(mem_status);

	GlobalMemoryStatusEx(&mem_status);

	const uint64_t GSIZE = 1024 * 1024 * 1024; // "ull" in "ullAvailPhys" indicates it is unsigned long long type, so use "uint64_t" here

	if (mem_status.ullAvailPhys > 8589934592) // Avail > 8G
		return mem_status.ullAvailPhys / GSIZE / 2; // Reserved size is set to half of the avail phys_mem

	if (mem_status.ullAvailPhys > 6442450944) // Avail > 6G
		return 2;

	if (mem_status.ullAvailPhys > 4294967296) // Avail > 4G
		return 1;

#endif // WITH_LOCK_PAGES

	return 0;
}
