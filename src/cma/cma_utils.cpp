#include "cma_utils.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

int CmaGetReservedHugePagesCount(void)
{
	int reservedHugePagesCount = 0;

	MEMORYSTATUSEX mem_status;
	mem_status.dwLength = sizeof(mem_status);
	GlobalMemoryStatusEx(&mem_status);

	if (mem_status.ullAvailPhys > 4294967296) // Avail > 4G
		reservedHugePagesCount = 1;

	if (mem_status.ullAvailPhys > 8589934592) // Avail > 8G
	{
		reservedHugePagesCount = 2;

		if (mem_status.ullTotalPhys > 17179869184) // Avail > 8G && Total > 16G
		{
			reservedHugePagesCount = 4;

			if (mem_status.ullAvailPhys > 12884901888) // Total > 16G && Avail > 12G
				reservedHugePagesCount = 6;
		}
	}

	return reservedHugePagesCount;
}
