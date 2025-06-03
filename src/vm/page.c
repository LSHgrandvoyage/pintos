#include "vm/page.h"
#include "threads/malloc.h"
#include "threads/palloc.h"
#include "threads/vaddr.h"
#include <hash.h>

unsigned page_hash(const struct hash_elem *e, void *aus UNUSED) {
    struct page *p=hash_entry(e,struct page, elem);
    return hash_bytes(&p->upage, sizeof(p->upage));
}

bool page_less(const struct hash_elem *a, const struct hash_elem *b, void *aux UNUSED) {
  struct page *pa = hash_entry(a, struct page, elem);
  struct page *pb = hash_entry(b, struct page, elem);
  return pa->upage < pb->upage;
}

//spt chogihwa
void page_init(struct hash *spt) {
  hash_init(spt, page_hash, page_less, NULL);
}

//page fault si page search
struct page *page_lookup(struct hash *spt, void *upage) {
  struct page p;
  struct hash_elem *e;

  p.upage = pg_round_down(upage);
  e = hash_find(spt, &p.elem);

  return e != NULL ? hash_entry(e, struct page, elem) : NULL;
}

bool page_insert(struct hash *spt, struct page *p) {
  return hash_insert(spt, &p->elem) == NULL;
}

void page_remove(struct hash *spt, void *upage) {
  struct page *p = page_lookup(spt, upage);
  if (p != NULL) {
    hash_delete(spt, &p->elem);
    free(p);
  }
}

void page_destroy(struct hash_elem *e, void *aux UNUSED) {
  struct page *p = hash_entry(e, struct page, elem);
  free(p);
}

void page_table_destroy(struct hash *spt) {
  hash_destroy(spt, page_destroy);
}

