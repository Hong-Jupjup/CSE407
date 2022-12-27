#include "vm/swap.h"
#include "devices/block.h"
#include "vm/frame.h"
#include "vm/page.h"

const size_t BLOCK_PER_PAGE = PGSIZE / BLOCK_SECTOR_SIZE;

void
swap_init (void)
{
	swap = bitmap_create (8 * 1024);
}

void
swap_in (size_t used_index, void *kaddr)
{
	struct block *swap_disk = block_get_role (BLOCK_SWAP);
	if (bitmap_test (swap, used_index)) {
		for (int i=0; (size_t) i<BLOCK_PER_PAGE; i++) {
			block_read (swap_disk, BLOCK_PER_PAGE * used_index + i, BLOCK_SECTOR_SIZE * i + kaddr);
		}
		bitmap_reset (swap, used_index);
	}
}

size_t
swap_out (void *kaddr)
{
	struct block *swap_disk = block_get_role (BLOCK_SWAP);
	size_t swap_index = bitmap_scan (swap, 0, 1, false);

	if (BITMAP_ERROR != swap_index) {
		for (int i=0; (size_t) i<BLOCK_PER_PAGE; i++) {
			block_write (swap_disk, BLOCK_PER_PAGE * swap_index + i, BLOCK_SECTOR_SIZE * i + kaddr);
		}
		bitmap_set (swap, swap_index, true);
	}
	return swap_index;
}
