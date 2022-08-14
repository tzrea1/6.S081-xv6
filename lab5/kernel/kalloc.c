// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
   struct spinlock lock;
   struct run *freelist;
   uint8 ref_count[(PHYSTOP - KERNBASE) / PGSIZE]; 
   // just use KERBASE ~ PHYSTOP memory(128M)
   // 128*1024*1024 / 4096 = 32768 pages
 } kmem;

// Lab5:Fork时，引用数++
void
increment_refcount(uint64 pa){
   acquire(&kmem.lock);
   kmem.ref_count[(pa - KERNBASE) / PGSIZE]++;
   release(&kmem.lock);
}

int
get_refcount(uint64 pa)
{
  return kmem.ref_count[(pa - KERNBASE) / PGSIZE];
}

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE){
    acquire(&kmem.lock);
    kmem.ref_count[((uint64)p - KERNBASE) / PGSIZE] = 1;
    release(&kmem.lock);
    kfree(p);    
  }
}

void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  acquire(&kmem.lock);
  if(--kmem.ref_count[((uint64)pa - KERNBASE) / PGSIZE] == 0){
    release(&kmem.lock);
    memset(pa, 1, PGSIZE);

    r = (struct run*)pa;
    
    acquire(&kmem.lock);
    r->next = kmem.freelist;
    kmem.freelist = r;
    release(&kmem.lock);
  }
  else
    release(&kmem.lock);
}



void *
kalloc(void)
{
  struct run *r;
  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r){
    kmem.ref_count[((uint64)r - KERNBASE) / PGSIZE] = 1;
    kmem.freelist = r->next;
  }
  release(&kmem.lock);

  if(r)
    memset((char*)r, 5, PGSIZE); 
  return (void*)r;
}