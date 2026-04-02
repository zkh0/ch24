#include "fifo.h"


void InitFifo(FifoStruct *fifo,unsigned char *buffer,int buffersize)
{
    fifo->head = fifo->rear = 0;
    fifo->buffer = buffer;
    fifo->len = buffersize;
}


int FifoIsFull(FifoStruct *fifo)
{
    if (fifo->head < fifo->rear)
    {
        if ((fifo->rear - fifo->head) == 1) return (1);
        else return (0);
    } 
    else
    {
        if (fifo->rear == 0 && fifo->head == (fifo->len-1))  return (1);
        else return (0);
    }  
}

int FifoIsEmpty(FifoStruct *fifo)
{
    if (fifo->head == fifo->rear) return (1);
    else return (0);
}

int FifoLen(FifoStruct *fifo)
{
    if (fifo->head >= fifo->rear) return (fifo->head - fifo->rear);
    // head < rear
    else  return (fifo->len - fifo->rear + fifo->head);
}

int FifoFreeSize(FifoStruct *fifo)
{
	int ret;
	ret = fifo->len - FifoLen(fifo);
	if (ret == 0) return 0;
	else return (ret - 1);
}


void EnFifo(FifoStruct *fifo,unsigned char dat)
{
    fifo->buffer[fifo->head] = dat;
    fifo->head++;
    if (fifo->head >= fifo->len) 
        fifo->head = 0;
}


unsigned char DeFifo(FifoStruct *fifo)
{
    unsigned char tmp;
    tmp = fifo->buffer[fifo->rear];
    fifo->rear ++;
    if (fifo->rear >= fifo->len) 
        fifo->rear = 0;
    return tmp;
}


int CheckFifo(FifoStruct *fifo,unsigned char *buffer,unsigned short len)
{
	int tmp,i;
	tmp = fifo->rear;
	for (i=0;i<len;i++) buffer[i] = DeFifo(fifo);
	i = fifo->rear;
	fifo->rear = tmp;
	return i;
}



void FifoClear(FifoStruct *fifo)
{
    fifo->head = fifo->rear = 0;
}

void FifoGetDMAPara(FifoStruct *fifo,unsigned int *addr,unsigned int *length)
{
	unsigned int a = (unsigned int)fifo->buffer;
	unsigned int len;
	a += fifo->rear;
	*addr = a;

	if (fifo->head < fifo->rear)    // 
	{
		len = fifo->len - fifo->rear;
	}
	else len = FifoLen(fifo);

	*length = len;
}

void FifoRemove(FifoStruct *fifo,unsigned int len)
{
	fifo->rear += len;
	if (fifo->rear >= fifo->len) 
        fifo->rear -= fifo->len;
}

void FifoAdd(FifoStruct *fifo,unsigned int len)
{
	fifo->head += len;
	if (fifo->head >= fifo->len)  fifo->head -= fifo->len;
}



void UpdateFifoFromDMA(FifoStruct *fifo,unsigned int headpos)
{
	fifo->head = headpos;
}

