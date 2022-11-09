#include <stddef.h>
#include "newbuffer.h"



void cb_init(circular_buffer *cb, size_t capacity, size_t sz)
{
  cb->buffer = kmalloc(capacity * sz, GFP_KERNEL); // GFP_KERNEL - Allocate normal kernel ram. May sleep.
  if (cb->buffer == NULL)
    // handle error
    cb->buffer_end = (char *)cb->buffer + capacity * sz;
  cb->capacity = capacity;
  cb->count = 0;
  cb->sz = sz;
  cb->head = cb->buffer;
  cb->tail = cb->buffer;
}

void cb_free(circular_buffer *cb)
{
  kfree(cb->buffer);
  // clear out other fields too, just to be safe
}

void cb_push(circular_buffer *cb, const void *item) // push back
{
  if (cb->count == cb->capacity)
  {
    // handle error
    printk(KERN_WARNING"MyMod: Buffer is full \n");
  }
  memcpy(cb->head, item, cb->sz);
  cb->head = (char *)cb->head + cb->sz;
  if (cb->head == cb->buffer_end)
    cb->head = cb->buffer;
  cb->count++;
}

void cb_pop(circular_buffer *cb, void *item) // pop front
{
  if (cb->count == 0)
  {
    // handle error
    printk(KERN_WARNING"MyMod: Buffer is empty \n");
  }
  memcpy(item, cb->tail, cb->sz);
  cb->tail = (char *)cb->tail + cb->sz;
  if (cb->tail == cb->buffer_end)
    cb->tail = cb->buffer;
  cb->count--;
}

int cb_setBufferSize(struct pData *pdata_p, size_t newcapacity) {

  circular_buffer new_b;
  int i;
  int count = pdata_p->buf_rdwr.count;
  if (count > newcapacity) {
    return 0;
  }

  printk(KERN_WARNING "MyMod: function setBufferSize: value of newcapacity %d \n", newcapacity);
  cb_init(&new_b, newcapacity, BUFFER_ELEMENTSIZE);
  printk(KERN_WARNING "MyMod: function setBufferSize: new_b capacity %d \n", new_b.capacity);

  for (i = 0; i < count; i++) {
    printk(KERN_WARNING "MyMod: function setBufferSize: for loop \n");
    int a;
    cb_pop(&pdata_p->buf_rdwr, &a);      
    cb_push(&new_b, &a);
  }
  cb_free(&pdata_p->buf_rdwr);
  printk(KERN_WARNING "MyMod: function setBufferSize: size of new buffer is %d \n", new_b.capacity);
  pdata_p->buf_rdwr = new_b;
  return 1;
}

size_t cb_getBufferSize(circular_buffer *cb) {
  return cb->capacity * cb->sz;
}


size_t cb_count(circular_buffer *cb)
{
    return  cb->count;
}






