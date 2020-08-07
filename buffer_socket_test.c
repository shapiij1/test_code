#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <string.h>

#define CIRC_BBUF_DEF(x,y)                \
    uint8_t x##_data_space[y];            \
    circbuff_t x = {                     \
        .buffer = x##_data_space,         \
        .head = 0,                        \
        .tail = 0,                        \
        .maxlen = y                       \
    }

/* circular buffer type */
typedef struct {
	uint8_t* const buffer;
	int head;
	int tail;
	const int maxlen;
} circbuff_t;

/* circular buffer push */
int circbuff_push(circbuff_t *c, uint8_t data)
{
	int next;

	next = c->head +1; //next is where head will point after this write
	if (next >= c->maxlen)
		next = 0;
	if (next == c->tail) //if the head+1 == tail, circular buffer is full
		return 0;
	c->buffer[c->head] = data;
	c->head = next;
	return 1; //success
}

/* circular buffer pop */
int circbuff_pop(circbuff_t *c, uint8_t *data)
{
	int next;
	if (c->head ==c->tail) //if head == tail we don't have any data
		return 0;
	next = c->tail + 1; //next is where tail will point to after this read
	if (next >= c->maxlen)
		next = 0;
	*data = c->buffer[c->tail];
	c->tail = next;
	return 1; //success
}

/* initialize circular buffer */
CIRC_BBUF_DEF(test_buffer,1000000);

void fill_buffer()
{
	/* fill circular buffer */
	uint8_t data_in = 0;
	int check = 1;
	while (check) {
		/* make sure data_in is within range of uint8_t */
		if (!(0 <= data_in && data_in <= 255)) {
			data_in = 0;
		}
		/* push */
		check = circbuff_push(&test_buffer, data_in);
		data_in++;
	}
}

int main()
{
	fill_buffer();	
}
