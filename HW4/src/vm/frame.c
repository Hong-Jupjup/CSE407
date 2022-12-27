#include "frame.h"
#include <stdio.h>

/* search the next clock in the LRU list */
static struct list_elem *
get_next_lru_clock (void)
{
	if (list_empty (&lru_list))
		return NULL;

	if (lru_clock == NULL || lru_clock == list_end (&lru_list))
		return list_begin (&lru_list);

	if (list_next (lru_clock) == list_end (&lru_list))
		return list_begin (&lru_list);

	else
		return list_next (lru_clock);

	return lru_clock;
}

/* initialize the LRU list */
void
lru_list_init (void)
{
	list_init (&lru_list);
	lock_init (&lru_list_lock);
	lru_clock = NULL;
}

/* insert an user page to the end of the LRU list */
void
add_page_to_lru_list (struct page *page)
{
	list_push_back (&lru_list, &(page->lru));
}

/* delete an user page at the end of the LRU list */
void
del_page_from_lru_list (struct page *page)
{
	if (&page->lru == lru_clock)
		lru_clock = list_next (lru_clock);

	list_remove (&page->lru);
}

/* when the physical pages are insufficient, secure free memory using clock algorithm */
void
try_to_free_pages (enum palloc_flags flags UNUSED)
{
	struct page *page, *victim;
	lru_clock = get_next_lru_clock ();
	page = list_entry (lru_clock, struct page, lru);

	while (page->vme->pinned || pagedir_is_accessed (page->thread->pagedir, page->vme->vaddr)) {
		pagedir_set_accessed (page->thread->pagedir, page->vme->vaddr, false);
		lru_clock = get_next_lru_clock ();
		page = list_entry (lru_clock, struct page, lru);
	}

	victim = page;
	switch (victim->vme->type) {
		case VM_BIN:
			if (pagedir_is_dirty (victim->thread->pagedir, victim->vme->vaddr)) {
				victim->vme->swap_slot = swap_out (victim->kaddr);
				victim->vme->type = VM_ANON;
			}
			break;

		case VM_ANON:
			victim->vme->swap_slot = swap_out (victim->kaddr);
			break;
	}
	victim->vme->is_loaded = false;
	_free_page (victim);
}

struct page *
alloc_page (enum palloc_flags flags)
{
	lock_acquire (&lru_list_lock);
	uint8_t *kpage = palloc_get_page (flags);
	while (kpage == NULL) {
		try_to_free_pages (flags);
		kpage = palloc_get_page (flags);
	}
	struct page *page = malloc (sizeof (struct page));
	page->kaddr = kpage;
	page->thread = thread_current ();

	add_page_to_lru_list (page);
	lock_release (&lru_list_lock);

	return page;
}

void
free_page (void *kaddr)
{
	lock_acquire (&lru_list_lock);
	struct list_elem *e;
	struct page *page = NULL;

	for (e = list_begin (&lru_list); e != list_end (&lru_list); e = list_next (e)) {
		struct page *cand_page = list_entry (e, struct page, lru);
		if (cand_page->kaddr == kaddr) {
			page = cand_page;
			break;
		}
	}

	if (page != NULL)
		_free_page (page);

	lock_release (&lru_list_lock);
}

void
_free_page (struct page *page)
{
	del_page_from_lru_list (page);
	pagedir_clear_page (page->thread->pagedir, pg_round_down (page->vme->vaddr));
	palloc_free_page (page->kaddr);
	free (page);
}
