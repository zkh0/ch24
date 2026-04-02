#ifndef __FIFO_H__
#define __FIFO_H__

typedef struct 
{
    int     head;            //指向队列开始,从这里取出
    int     rear;            //指向队列结束，从这里增加      
    unsigned char    *buffer;
    int     len;
}FifoStruct;



void InitFifo(FifoStruct *fifo,unsigned char *buffer,int buffersize);
int FifoIsFull(FifoStruct *fifo);
int FifoIsEmpty(FifoStruct *fifo);
int FifoLen(FifoStruct *fifo);
void EnFifo(FifoStruct *fifo,unsigned char dat);
unsigned char DeFifo(FifoStruct *fifo);
int FifoFreeSize(FifoStruct *fifo);
int CheckFifo(FifoStruct *fifo,unsigned char *buffer,unsigned short len);
void FifoClear(FifoStruct *fifo);
void FifoGetDMAPara(FifoStruct *fifo,unsigned int *addr,unsigned int *length);
void UpdateFifoFromDMA(FifoStruct *fifo,unsigned int headpos);

void FifoRemove(FifoStruct *fifo,unsigned int len);

#endif
