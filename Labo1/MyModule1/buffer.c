/**
 * Author 			: Louis-Philippe Majeau and Shan Meunier
 * Date   			: August 2018
 *
 * brief : Basic functions to manipulate a circular buffer
 */

#include "buffer.h"

// Private prototypes ---------------------------------------------------------

// Public functions -----------------------------------------------------------
/**
 * buffer_new
 *
 * @brief Creates a new buffer and initializes it.
 *
 * @parameters : 	buffer_t *buffer 	Pointer to the buffer to create
 * 				 	uint8_t *data    	Data structure that holds the data
 * 				 	uint32_t size		Size of the buffer to create
 *
 * @return     :	none
 *
 * @Deprecated, use BUFFER_NEW macro
 */
void buffer_new(buffer_t *buffer, uint8_t *data, uint32_t size)
{
    buffer->idx_in = 0;
    buffer->idx_out = 0;
    buffer->count = 0;
    buffer->data = data;
    buffer->size = size;
}

/**
 * buffer_push
 *
 * @brief Pushes new data into the buffer
 *
 * @parameters : 	buffer_t *buffer 	Pointer to the buffer to add data
 * 				 	uint8_t ch			Data to push
 *
 * @return     :	int32_t				Number of elements in the buffer after the push
 */
int32_t buffer_push(buffer_t *buffer, uint8_t ch)
{
    int32_t retval = -1;

    // Check if buffer is full
    if(buffer->count < buffer->size) {
    	// Push data into the buffer and adjust count and indexer
        buffer->data[buffer->idx_in++] = ch;
        retval = ++buffer->count;
        buffer->idx_in %= buffer->size;
    }

    return retval;
}

/**
 * buffer_pull
 *
 * @brief Pulls data from the buffer
 *
 * @parameters : 	buffer_t *buffer 	Pointer to the buffer to pull data
 * 				 	uint8_t *ch			Receiver of pulled data
 *
 * @return     :	int32_t				Number of elements in the buffer after the pull
 */
int32_t buffer_pull(buffer_t *buffer, uint8_t *ch)
{
    int32_t retval = -1;

    // Check if buffer is empty
    if (buffer->count != 0) {
    	// Pull data from the buffer and adjust count and indexer
        *ch = buffer->data[buffer->idx_out++];
        retval = --buffer->count;
        buffer->idx_out %= buffer->size; 
    }

    return retval;
}

/**
 * buffer_count
 *
 * @brief Gets the actual count of the buffer
 *
 * @parameters : 	buffer_t *buffer 	Pointer to the buffer to get the count
 *
 * @return     :	int32_t				Number of elements in the buffer
 */
int32_t buffer_count(buffer_t *buffer)
{
    return buffer->count;
}

/**
 * buffer_size
 *
 * @brief Gets the size of the buffer
 *
 * @parameters : 	buffer_t *buffer 	Pointer to the buffer to get the size
 *
 * @return     :	int32_t				Size of the buffer
 */
int32_t buffer_size(buffer_t *buffer)
{
    return buffer->size;
}

/**
 * buffer_flush
 *
 * @brief Gets the size of the buffer
 *
 * @parameters : 	buffer_t *buffer 	Pointer to the buffer to flush
 *
 * @return     :	none
 */
void buffer_flush(buffer_t *buffer)
{
	// Flush the buffer
	buffer->idx_out = buffer->idx_in;
	buffer->count = 0;
}

// Private functions ----------------------------------------------------------

// EOF
