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

<<<<<<< HEAD
void set_cbbuffer_size(size_t newcapacity {

  cb->buffer = krealloc(size * sz, GFP_KERNEL); // GFP_KERNEL - Allocate normal kernel ram. May sleep.
  if (cb->buffer == NULL)
    // handle error
    cb->buffer_end = (char *)cb->buffer + size * sz;
  cb->capacity = capacity;
  cb->count = 0;
  cb->sz = sz;
  cb->head = cb->buffer;
  cb->tail = cb->buffer;
}

}



=======
/**
 * circ buffer_count
 *
 * @brief Gets the actual count of the buffer
 *
 * @parameters : 	circular_buffer *cb 	Pointer to the buffer to get the count
 *
 * @return     :	size_t				Number of elements in the buffer
 */
size_t cb_count(circular_buffer *cb)
{
    return  cb->count;
}
>>>>>>> 8187266431f4f5c1ec4f069dd907e7d69c554284


