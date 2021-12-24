#pragma once

extern "C" {
	__declspec(dllexport) size_t __stdcall MemTotalCommitted(void);

	__declspec(dllexport) size_t __stdcall MemTotalReserved(void);

	__declspec(dllexport) size_t __stdcall MemFlushCache(size_t size);

	__declspec(dllexport) void __stdcall MemFlushCacheAll(void);

	__declspec(dllexport) size_t __stdcall MemSize(void* mem);

	__declspec(dllexport) size_t __stdcall MemSizeA(void* mem, size_t align);

	__declspec(dllexport) void* __stdcall MemAlloc(size_t size);

	__declspec(dllexport) void* __stdcall MemAllocA(size_t size, size_t align);

	__declspec(dllexport) void __stdcall MemFree(void* mem);

	__declspec(dllexport) void __stdcall MemFreeA(void* mem);

	__declspec(dllexport) void __stdcall EnableHugePages(void);
}
