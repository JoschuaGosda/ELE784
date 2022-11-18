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

circular_buffer* cb_setBufferSize(circular_buffer *cb, size_t newcapacity) {
  uint8_t tempVal;
  circular_buffer *new_b;
  int i;
  int count = cb->count;
  
  new_b = (circular_buffer *) kmalloc(sizeof(circular_buffer), GFP_KERNEL);

  printk(KERN_WARNING "MyMod: function setBufferSize: value of newcapacity %ld \n", newcapacity);
  cb_init(new_b, newcapacity, BUFFER_ELEMENTSIZE);
  printk(KERN_WARNING "MyMod: function setBufferSize: new_b capacity %ld \n", new_b->capacity);

  for (i = 0; i < count; i++) {
    printk(KERN_WARNING "MyMod: function setBufferSize: for loop \n");
    
    cb_pop(cb, &tempVal);      
    cb_push(new_b, &tempVal);
  }
  cb_free(cb);
  printk(KERN_WARNING "MyMod: function setBufferSize: size of new buffer is %ld \n", new_b->capacity);
  
  return new_b;
}

size_t cb_getBufferSize(circular_buffer *cb) {
  return cb->capacity * cb->sz;
}


size_t cb_count(circular_buffer *cb)
{
    return  cb->count;
}






