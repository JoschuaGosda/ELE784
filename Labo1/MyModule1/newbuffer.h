#ifndef NEWBUFFER_H
#define NEWBUFFER_H 
//#include "MyModule.h"

typedef struct circular_buffer
{
    void *buffer;     // data buffer
    void *buffer_end; // end of data buffer
    size_t capacity;  // maximum number of items in the buffer
    size_t count;     // number of items in the buffer
    size_t sz;        // size of each item in the buffer
    void *head;       // pointer to head
    void *tail;       // pointer to tail
} circular_buffer;
/*
extern struct pData
{
  int8_t owner; // takes the owners id
  circular_buffer buf_rdwr;
  uint8_t numPort;
  bool fREAD;
  bool fWRITE;
  struct semaphore sem;
  struct spinlock splock;
  struct mutex mutex;
  wait_queue_head_t RdQ, WrQ;
} pData;*/

void cb_init(circular_buffer *cb, size_t capacity, size_t sz);

void cb_free(circular_buffer *cb);

void cb_push(circular_buffer *cb, const void *item);

void cb_pop(circular_buffer *cb, void *item);

size_t cb_count(circular_buffer *cb);

size_t cb_getBufferSize(circular_buffer *cb);

circular_buffer* cb_setBufferSize(circular_buffer *cb, size_t newcapacity);

size_t cb_count(circular_buffer *cb);

#endif