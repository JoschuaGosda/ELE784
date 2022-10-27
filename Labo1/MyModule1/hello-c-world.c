#include <stdio.h>

#include "buffer.h"

BUFFER_NEW(buf, 8);

int main() {
    printf("Hello World!\n");
    
    
    buffer_t buf;
    uint8_t buf_data[8];
    
    //buffer_new(&buf, buf_data, 8);
    
    
    //printf("buff size: %d\n", buffer_size(&buf));
    //printf("buff count: %d\n", buffer_count(&buf));
    
    
    buffer_push(&buf, 'h');
    buffer_push(&buf, 'e');
    buffer_push(&buf, 'i');
    buffer_push(&buf, 'k');
    buffer_push(&buf, 'g');
    buffer_push(&buf, 'l');
    buffer_push(&buf, 'u');
    buffer_push(&buf, 'x');
    printf("buff error push: %d\n", buffer_push(&buf, 'u'));
    
    printf("buff size: %d\n", buffer_size(&buf));
    printf("buff count: %d\n", buffer_count(&buf));
    
    uint8_t ch = 0;
    
    while(buffer_count(&buf)) {
        buffer_pull(&buf, &ch);
        printf("buff data: %c\n", ch);
        printf("buff size: %d\n", buffer_size(&buf));
        printf("buff count: %d\n", buffer_count(&buf));
    }
    
    printf("buff error pull: %d\n", buffer_pull(&buf, &ch));
    return 0;
}
