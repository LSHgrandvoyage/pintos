#ifndef VM_PAGE_H
#define VM_PAGE_H

#include <hash.h>
#include <stdbool.h>

#include "filesys/file.h"
#include <stddef.h>

struct page {
    void *upage;
    void *kpage;
    bool writable;
    struct hash_elem elem;
    bool is_loaded;

    //demand paging information
    struct file *file;
    off_t offset;
    size_t read_bytes;
    size_t zero_bytes;
};

void page_init(struct hash *spt);
struct page *page_lookup(struct hash *spt, void *upage);
bool page_insert(struct hash *spt,struct page *p);
void page_remove(struct hash *spt, void *upage);
void page_table_destroy(struct hash *spt);

#endif
