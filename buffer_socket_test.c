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


//circular buffer type
typedef struct {
	uint8_t* const buffer;
	int head;
	int tail;
	const int maxlen;
} circbuff_t;

//circular buffer push
int circbuff_push(circbuff_t *c, uint8_t data)
{
	int next;

	next = c->head +1; //next is whhead will point after this write
	if (next >= c->maxlen)
		next = 0;
	if (next == c->tail) //if the head+1 == tail, circular buffer is full
		return -1;
	c->buffer[c->head] = data;
	c->head = next;
	return 0; //success
}

//circular buffer pop
int circbuff_pop(circbuff_t *c, uint8_t data)
{
	int next;
	if (c->head ==c->tail) //if head == tail we don't have any data
		return -1;
	next = c->tail + 1; //next is where tail will point to after this read
	if (next >= c->maxlen)
		next = 0;
	*data = c->buffer[c->tail];
	c->tail = next;
	return 0; //success
}


