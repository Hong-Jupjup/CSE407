#ifndef VM_PAGE_H
#define VM_PAGE_H

#include <hash.h>
#include <list.h>
#include "threads/thread.h"
#include "threads/vaddr.h"

#define VM_BIN 0		/* load data from a binary file */
#define VM_ANON 1		/* load data from swap area */

struct vm_entry {
	uint8_t type;				/* type of VM_BIN, VM_ANON */
	void *vaddr;				/* number of virtual page managed by vm_entry */
	bool writable;				/* if True, can write on the corresponding address,
							   		if False, cannot write */
	bool pinned;
	bool is_loaded;				/* flag indicating whether physical memory is mounted or not */
//	struct file *file;			/* file mapped to virtual addresses */
	char *file;

	size_t offset;				/* file offset to read */
	size_t read_bytes;			/* data size written in virtual page */
	size_t zero_bytes;			/* byte of remaining page to be filled with 0 */

	size_t swap_slot;			/* swap slot */
	
	struct hash_elem elem;		/* hash table element */
};

struct page {
	void *kaddr;
	struct vm_entry *vme;
	struct thread *thread;
	struct list_elem lru;
};

static unsigned vm_hash_func (const struct hash_elem *e, void *aux);
static bool vm_less_func (const struct hash_elem *a, const struct hash_elem *b, void *aux);

/* Functions for Initializing a Hash Table */
/* initialize a hash table using hash_init () */
void vm_init (struct hash *vm);

/* Functions for Inserting or Deleting Element into a Hash Table */
/* insert vm_entry into a hash table using hash_insert () */
bool insert_vme (struct hash *vm, struct vm_entry *vme);
/* delete vm_entry from a hash table using hash_delete () */
bool delete_vme (struct hash *vm, struct vm_entry *vme);

/* A Function for Searching vm_entry in a Hash Table */
/* return after searching vm_entry corresponding vaddr */
struct vm_entry *find_vme (void *vaddr);

/* A Function for Deleting a Hash Table */
/* delete bucket lists of a hash table and vm_entry using hash_destroy () */
void vm_destroy (struct hash *vm);

/* A Function for Loading a File in Disk into a Physical Page */
bool load_file (void *kaddr, struct vm_entry *vme);

#endif
