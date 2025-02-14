#include "cma_utils.h"

#include "mimalloc.h"

void CmaSetMemoryAllocatorRuntimeOptions(void)
{
    // Use `mi_option_set_default` instead of `mi_option_set` to prioritize user defined options in environment variables

	mi_option_set_default(mi_option_arena_eager_commit, 1); // Always eager commit arenas, which has a significant performance improvement on Arma 3

	mi_option_set_default(mi_option_allow_large_os_pages, 1); // Always use large pages when available

	mi_option_set_default(mi_option_eager_commit_delay, 0); // Always eager commit segments

	mi_option_set_default(mi_option_purge_delay, -1); // Do not purge memory back to the OS
}
