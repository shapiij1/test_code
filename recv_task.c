
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <sys/time.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "recv_task.h"

void recv_task(void* params) {
    struct recvTaskParams* taskParams;
    int recvSocket;
    struct sockaddr_in sockAddr;
    fd_set recvSet;
    struct timeval selectTimeout;
    unsigned int dmaOpCnt;
    int continueRecv;
    int dummy, result, nready, len;

    taskParams = (struct recvTaskParams*)params;
    dmaOpCnt = 0;
    continueRecv = 1;

    // create recv socket
    if ((recvSocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
        perror("socket creation failed"); 
        taskParams->result = recvSocket;
        return;
    } 

    memset(&sockAddr, 0, sizeof(sockAddr)); 
      
    // Filling server information 
    sockAddr.sin_family = AF_INET; 
    sockAddr.sin_port = htons(RECV_PORT); 
    sockAddr.sin_addr.s_addr = INADDR_ANY; 
    len = sizeof(sockAddr);

    if ((result = bind(recvSocket, (const struct sockaddr *)&sockAddr, sizeof(sockAddr))) < 0) { 
        perror("bind failed"); 
        taskParams->result = result;
        return;
    } 

    FD_ZERO(&recvSet);

    // set select() timeout to 1ms
    selectTimeout.tv_sec = 0;
    selectTimeout.tv_usec = 1000;

    while(continueRecv) {
        // select on recv socket
        FD_SET(recvSocket, &recvSet);

        select((recvSocket+1), &recvSet, NULL, NULL, &selectTimeout);

        // if recv socket is not readable 
        if (!FD_ISSET(recvSocket, &recvSet)){
            // do a DMA transfer and mark valid = FALSE
            struct blockHeader* header;
            header = (struct blockHeader*)taskParams->tx_proxy_interface_p->buffer;
            taskParams->tx_proxy_interface_p->length = sizeof(struct blockHeader);
            header->valid = 0;

        }
        else {
            // recv socket is readable
            // do a DMA transfer and mark valid = TRUE
            struct blockHeader* header;
            int recvBytes;

            // read data block from recv socket into DMA memory block
            recvBytes = recvfrom(recvSocket, taskParams->tx_proxy_interface_p->buffer, 
                                    taskParams->dma_block_size, MSG_WAITALL, (struct sockaddr*)&sockAddr, &len);

            header = (struct blockHeader*)taskParams->tx_proxy_interface_p->buffer;
            header->valid = 1;
            taskParams->tx_proxy_interface_p->length = taskParams->dma_block_size;
            
            // printf("recvBytes: %d\n", recvBytes);
            // printf("received block %d(%d bytes) of size: %d\n", header->blockCount, header->blockSize, taskParams->tx_proxy_interface_p->length);

            dmaOpCnt++;
        } 

        // initiate DMA operation
        ioctl(*taskParams->tx_proxy_fd, 0, &dummy);


        if (taskParams->tx_proxy_interface_p->status != PROXY_NO_ERROR) {
            printf("Proxy tx transfer error\n");
        }

        continueRecv = ((taskParams->dma_count == 0) || (dmaOpCnt < taskParams->dma_count)) ? 1 : 0;
    }

    printf("exiting recv task\n");

}