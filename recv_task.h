
#ifndef RECV_TASK_H
#define RECV_TASK_H

#include "dma-proxy.h"

#define RECV_ADDR "192.168.0.1"
#define RECV_PORT 10001

struct recvTaskParams {
    int dma_count;
    int dma_block_size;
    int* tx_proxy_fd;
    struct dma_proxy_channel_interface* tx_proxy_interface_p;
    int result;
};

struct blockHeader {
    unsigned int blockCount;
    unsigned short blockSize;
    unsigned short valid;
    unsigned short reserved;
};

void recv_task(void* params);

#endif // TX_TASK_H