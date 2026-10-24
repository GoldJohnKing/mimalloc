#include "cma_api.h"

#include "mimalloc.h"

size_t __stdcall MemTotalCommitted(void)
{
	return 0;
}

size_t __stdcall MemTotalReserved(void)
{
	return 0;
}

size_t __stdcall MemFlushCache(size_t size)
{
	return size;
}

void __stdcall MemFlushCacheAll(void)
{
	// This function does not affect actual behavior, see: https://community.bistudio.com/wiki/Arma_3:_Custom_Memory_Allocator
}

size_t __stdcall MemSize(void* mem)
{
	return mi_usable_size(mem);
}

size_t __stdcall MemSizeA(void* mem, size_t /*align*/)
{
	return mi_usable_size(mem);
}

void* __stdcall MemAlloc(size_t size)
{
	return mi_malloc(size);
}

void* __stdcall MemAllocA(size_t size, size_t align)
{
	return mi_malloc_aligned(size, align);
}

void __stdcall MemFree(void* mem)
{
	mi_free(mem);
}

void __stdcall MemFreeA(void* mem)
{
	mi_free(mem);
}

void __stdcall EnableHugePages(void)
{
	// Huge pages and other runtime options are set once the dll is loaded, so no need to do it here
}
