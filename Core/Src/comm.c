#include "main.h"

/**************************************************************************************************************/


/**************************************************************************************************************/
//临时buffer
u8 frameInRxBuffer[UART_RX_SIZE];
u8 frameOutRxBuffer[UART_RX_SIZE];

u8 frameInTxBuffer[UART_RX_SIZE];				
u8 frameOutTxBuffer[UART_RX_SIZE];

/**************************************************************************************************************/
/**************************************************************************************************************/
/**************************************************************************************************************/
/**************************************************************************************************************/
/**************************************************************************************************************/
/**************************************************************************************************************/

// frame  
// 0xAA 0xBB len cmd datan  crc32

/**************************************************************************************************************/


/**************************************************************************************************************/
extern stUserPara userPara;
extern u16 ADCAllBuffer[];
extern u8 cleanFinished;
/**************************************************************************************************************/
u8 myAddr;


/**************************************************************************************************************/



/**************************************************************************************************************/
void testuart4Send(void)
{
	static u32 tick=0;
	int i;
	if ((HAL_GetTick() - tick) > 1000)
		{
			tick = HAL_GetTick();
			for (i=0;i<9;i++) EnFifo(&UartCommInTxFifo,i+'0');
			LL_USART_EnableIT_TXE(UART4);
		}
}


void testuart5Send(void)
{
	static u32 tick=0;
	int i;
	if ((HAL_GetTick() - tick) > 1000)
		{
			tick = HAL_GetTick();
			for (i=0;i<9;i++) EnFifo(&UartCommOutTxFifo,i+'0');
			LL_USART_EnableIT_TXE(UART5);
		}
}

/**************************************************************************************************************/

/**************************************************************************************************************/
void sendInToNext(u8 *buffer,u8 len)			 //转发数据
{
	int i;
	for (i=0;i<len;i++)  EnFifo(&UartCommOutTxFifo, buffer[i]);
	LL_USART_EnableIT_TXE(UART5);
}

void sendOutToNext(u8 *buffer,u8 len)			 //转发数据,串口不同
{
	int i;
	for (i=0;i<len;i++)  EnFifo(&UartCommInTxFifo, buffer[i]);
	LL_USART_EnableIT_TXE(UART4);
}


void doInFrame(u8 *buffer)			// PC ==> MCU
{
	u8 dstAddr,srcAddr;
	stCommHeader *pHead;
	pHead = (stCommHeader *)buffer;
	if (pHead->srcAddr != PC_ADDR)	return;			//源地址错误
	if (pHead->destAddr != myAddr)		//转发
		{
			sendInToNext(frameInRxBuffer,pHead->len);
			return;
		}
	//地址符合，处理数据并回复
	switch(pHead->id)
		{
			case CMD_ID_RESET:
				__NVIC_SystemReset();
				break;

			default: break;
		}
	
	
}
/**************************************************************************************************************/

void doOutFrame(u8 *buffer)			// PC ==> MCU
{
	u8 dstAddr,srcAddr;
	stCommHeader *pHead;
	pHead = (stCommHeader *)buffer;


	
}



/**************************************************************************************************************/


/**************************************************************************************************************/
void getInFrame(void)      // 从数据包中取出一帧数据，处理或者转发,  PC => MCU
{
	u8 len,i;
	u16 cmd;
	u16 crc16_rcv;
	u16 crc16;
	stCommHeader *pHead;

	while (FifoLen(&UartCommInRxFifo) >= MIN_FRAME_LENGTH)
		{
      CheckFifo(&UartCommInRxFifo, &frameInRxBuffer[0], 6);
			pHead = (stCommHeader *)&frameInRxBuffer;
      if (pHead->head1 == HEAD1 && pHead->head2 == HEAD2)    // Get Head 
      {

				len = FifoLen(&UartCommInRxFifo);
				if (len >= pHead->len) 
					{
						for (i = 0; i < pHead->len; i++) frameInRxBuffer[i] = DeFifo(&UartCommInRxFifo);
						//check crc16
						crc16 = crc16_modbus(frameInRxBuffer,pHead->len - 2);
						crc16_rcv = frameInRxBuffer[pHead->len - 2] + (frameInRxBuffer[pHead->len - 1] * 256);
						if (crc16 == crc16_rcv)
							{
								doInFrame(frameInRxBuffer);
								Trace_Print(" crc OK \r\n");
							}
						else
							{
								Trace_Print(" crc ERROR, calc=0x%x,rcv=0x%x \r\n",crc16,crc16_rcv);;				//这里crc错误
							}
					}
	      else   
	      {
	      	return;
	      }
      }
      else
      {
          DeFifo(&UartCommInRxFifo);    // Throw one byte and try again
      }
  }
	return ;
}
/**************************************************************************************************************/
void getOutFrame(void)   //  MCU => PC
{
	u8 len,i;
	u16 cmd;
	u16 crc16_rcv;
	u16 crc16;
	stCommHeader *pHead;

	while (FifoLen(&UartCommOutRxFifo) >= MIN_FRAME_LENGTH)
		{
      CheckFifo(&UartCommOutRxFifo, &frameOutRxBuffer[0], 6);
			pHead = (stCommHeader *)&frameOutRxBuffer;
      if (pHead->head1 == HEAD1 && pHead->head2 == HEAD2)    // Get Head 
      {

				len = FifoLen(&UartCommOutRxFifo);
				if (len >= pHead->len) 
					{
						for (i = 0; i < pHead->len; i++) frameOutRxBuffer[i] = DeFifo(&UartCommOutRxFifo);
						//check crc16
						crc16 = crc16_modbus(frameOutRxBuffer,pHead->len - 2);
						crc16_rcv = frameOutRxBuffer[pHead->len - 2] + (frameOutRxBuffer[pHead->len - 1] * 256);
						if (crc16 == crc16_rcv)
							{
								doOutFrame(frameOutRxBuffer);
							}
					}
	      else   
	      {
	      	return;
	      }
      }
      else
      {
          DeFifo(&UartCommOutRxFifo);    // Throw one byte and try again
      }
  }
	return ;
}
/**************************************************************************************************************/
void commFunc(void)			//负责处理所有通讯，在 1ms tick 中调用
{
		//1. uart4 接收从PC端来的数据，并转发
		getInFrame();


		//2. uart5 接收从MCU端来的数据，并转发
		getOutFrame();

}

/**************************************************************************************************************/

/**************************************************************************************************************/

/**************************************************************************************************************/

/**************************************************************************************************************/
/**************************************************************************************************************/

/**************************************************************************************************************/

//uart debug
void displayCmd(void)
{
	Trace_Print("\r\n *********** cmd list begin ************\r\n");
	Trace_Print(" ??: help cmd \r\n");
	Trace_Print(" 00: reset mcu \r\n");
	Trace_Print(" 11: state idle \r\n");
	Trace_Print(" 22: state detect \r\n");
	Trace_Print(" 44: state clean \r\n");
	Trace_Print(" 55: state detect after clean \r\n");
	Trace_Print(" 99: auto run \r\n");
	Trace_Print("\r\n *********** cmd list end************\r\n\r\n");
}

 


void debugFunc(void)
{
	int i;
	u8 ch[2];
	if (FifoLen(&UartTraceRxFifo)<2) return;

	
	ch[0] = DeFifo(&UartTraceRxFifo);
	ch[1] = DeFifo(&UartTraceRxFifo);
	while (1)	 
		{
			if (FifoLen(&UartTraceRxFifo)>0) DeFifo(&UartTraceRxFifo);
			else break;
		}
		
	if (ch[0] != ch[1]) return;


	switch(ch[0])
		{
			case '?':
				displayCmd();
				break;
			case '0':
				NVIC_SystemReset();
				break;
			case '1':
				cleanState = state_idle;
				userPara.magicData = MAGIC_DATA;
				Trace_Print(" measure with offset \r\n");

				break;
			case '2':
				cleanState = state_idle;
				userPara.magicData = 0;		
				Trace_Print(" measure without offset \r\n");

				break;
			case '4':
				cleanState = state_calibration;
				Trace_Print(" calibration finished \r\n");
				break;
			case '5':
				cleanState = state_detect_after_clean;	
				break;
			case '9':
				cleanState = state_idle;
				break;


			
			default: break;
		}

	
}

/**************************************************************************************************************/

/**************************************************************************************************************/

/**************************************************************************************************************/
/**************************************************************************************************************/

