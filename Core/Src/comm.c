#include "main.h"

/**************************************************************************************************************/


/**************************************************************************************************************/
//临时buffer
u8 frameInRxBuffer[UART_RX_SIZE];  // PC ==> MCU  4口
u8 frameInTxBuffer[UART_RX_SIZE];	

u8 frameOutRxBuffer[UART_RX_SIZE];  // mcu->pc   5口
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
extern u8 checkflag;
/**************************************************************************************************************/
u8 myAddr;


/**************************************************************************************************************/



/**************************************************************************************************************/
void testuart4Send(void)
{
	static u32 tick=0;
	int i;
	if ((HAL_GetTick() - tick) > 100)
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
void sendInToNext(u8 *buffer,u8 len)			 //pc->mcu发送
{
	int i;
	for (i=0;i<len;i++)  EnFifo(&UartCommOutTxFifo, buffer[i]);
	LL_USART_EnableIT_TXE(UART5);
}

void sendOutToNext(u8 *buffer,u8 len)			 //mcu->pc发送
{
	int i;
	for (i=0;i<len;i++)  EnFifo(&UartCommInTxFifo, buffer[i]);
	LL_USART_EnableIT_TXE(UART4);
}


/**************************************************************************************************************/


void Response(void)    //回复
{
	u16 crc16;
	u16 crc16_rcv;
  u8 len = 10;                          //2帧头+ 1ID+ 1目标地址+ 1源地址+ 1长度 +2是否收到正确的值+ 2个校验位
	stCommHeader *pHead;	              
  pHead = (stCommHeader *)frameInRxBuffer ;
  
	
	frameInRxBuffer[0] = HEAD1;
	frameInRxBuffer[1] = HEAD2;
	frameInRxBuffer[2] = pHead->id | 0x80; // 0x05 ，发送ID 0x85
	frameInRxBuffer[3] = PC_ADDR;
	frameInRxBuffer[4] = myAddr;
	frameInRxBuffer[5] = len;


	frameInRxBuffer[6] = 0x00;
	frameInRxBuffer[7] = 0x00;
		
  crc16 = crc16_modbus(frameInRxBuffer,len - 2);

	
	frameInRxBuffer[8] = crc16&0xff;
	frameInRxBuffer[9] = (crc16>>8)&0xff;
		sendOutToNext(frameInRxBuffer,len);
	
//	u8 frameInRxBuffer[UART_RX_SIZE];  // PC ==> MCU  4口
//u8 frameInTxBuffer[UART_RX_SIZE];	

//u8 frameOutRxBuffer[UART_RX_SIZE];  // mcu->pc   5口
//u8 frameOutTxBuffer[UART_RX_SIZE];   
}
/**************************************************************************************************************/
void SendSetToPC(void)      //发送设置参数到pc
{
	u16 crc16;
  u8 len = 30;                          //2帧头+ 1ID+ 1目标地址+ 1源地址+ 1长度 +1是否收到正确的值+ 2个校验位
  u8 tempbuff[32] = {0};
	
	tempbuff[0] = HEAD1;
	tempbuff[1] = HEAD2;
	tempbuff[2] = 0x07 | 0x80;
	tempbuff[3] = PC_ADDR;
	tempbuff[4] = myAddr;
	tempbuff[5] = len;

	tempbuff[6] = (userset.CleanTime>>8)&0xff;
	tempbuff[7] = (userset.CleanTime)&0xff;
	
	tempbuff[8] = (userset.IntervalBetweenCleanTime>>8)&0xff;
	tempbuff[9] = (userset.IntervalBetweenCleanTime)&0xff;
	
	tempbuff[10] = (userset.CleanBeforeDelayTime>>8)&0xff;
	tempbuff[11] = (userset.CleanBeforeDelayTime)&0xff;
	
	tempbuff[12] = (userset.CleanAfterDelayTime>>8)&0xff;
	tempbuff[13] = (userset.CleanAfterDelayTime)&0xff;
	
	tempbuff[14] = (userset.DelayBeforeMeasureTime>>8)&0xff;
	tempbuff[15] = (userset.DelayBeforeMeasureTime)&0xff;
	
	tempbuff[16] = (userset.DelayAfterMeasureTime>>8)&0xff;
	tempbuff[17] = (userset.DelayAfterMeasureTime)&0xff;
	
	tempbuff[18] = (userset.maxCurrent>>8)&0xff;
	tempbuff[19] = (userset.maxCurrent)&0xff;
	
	tempbuff[20] = (userset.MinOkR>>8)&0xff;
	tempbuff[21] = (userset.MinOkR)&0xff;
	
	tempbuff[22] = (userset.MaxFaultR>>8)&0xff;
	tempbuff[23] = (userset.MaxFaultR)&0xff;
	
	tempbuff[24] = (userset.FirstCleanTime>>8)&0xff;
	tempbuff[25] = (userset.FirstCleanTime)&0xff;
	
	tempbuff[26] = (userset.CleanTimes)&0xff;
	tempbuff[27] = (userset.updatecontrol.upstate)&0xff;
	
	crc16 = crc16_modbus(tempbuff,len - 2);
	tempbuff[28] = crc16&0xff;
	tempbuff[29] = (crc16>>8)&0xff;
	
		sendOutToNext(tempbuff,len);

}
/**************************************************************************************************************/
void ParseSetInstruction(void)      //解析指令。将值放入userset，小端解析
{
	u8 *p = &frameInRxBuffer[10];
	userset.Magic_data                 = MAGIC_DATA;
	userset.CleanTime                  =(u16)p[1]<<8 |p[0];p+=2;
	userset.IntervalBetweenCleanTime   =(u16)p[1]<<8 |p[0];p+=2;
	userset.CleanBeforeDelayTime       =(u16)p[1]<<8 |p[0];p+=2;
	userset.CleanAfterDelayTime        =(u16)p[1]<<8 |p[0];p+=2;
	userset.DelayBeforeMeasureTime     =(u16)p[1]<<8 |p[0];p+=2;
	userset.DelayAfterMeasureTime      =(u16)p[1]<<8 |p[0];p+=2;
	userset.maxCurrent                 =(u16)p[1]<<8 |p[0];p+=2;
	userset.MinOkR                     =(u16)p[1]<<8 |p[0];p+=2;
	userset.MaxFaultR                  =(u16)p[1]<<8 |p[0];p+=2;
	userset.FirstCleanTime             =(u16)p[1]<<8 |p[0];p+=2;
	userset.CleanTimes                 = frameInRxBuffer[26];
	
	userset.updatecontrol.upstate = frameInRxBuffer[27];
}
/**************************************************************************************************************/
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
		  case CMD_ID_USERSET:                 // 0x01设置参数
				Response();                        //回复
			  ParseSetInstruction();		         //将指令存入userset
		    writeSet();                        //写userset到flash 
		                                      //	初始化 //将userset结构体嵌入程序
				break;
			
		  case CMD_ID_RESET:     				       //0x02 复位    数据长度6
				Response();
			  hwDelayms(10);
				__NVIC_SystemReset();
			  break;
			
			case CMD_ID_CALIBRATION:            //0x03 校准
           Response();				 
		    cleanState = state_calibration;

			  break;
			
			
			case CMD_ID_MEASURE:   				//0x04 上传状态 
				Response();
			 sendFlagToPC(crystal_first_clean); 
			  break;
			case CMD_ID_IMPEDANCE:                //0x05 上传阻抗
				Response();
		  sendDataToPC(crystal_after_clean); 
			  break;
			
     
			case CMD_ID_PROBDETECTION:             //0x06 探头检测
		   Response();
			 ProbDetection();                 //检测  //返回三个字节的值分别代表24个通道		                 
			  break;
			
			case CMD_ID_READOUT:   
	    Response();				                   //0x07 读出参数
			readset();
		  SendSetToPC();
			  break;
			
			case CMD_ID_CHECKGLAG:   
	    Response();				                   //0x08 检测标志位
			 checkflag = 1;  //全局变量
			  break;
			default: break;
		}
	
	
}


/**************************************************************************************************************/

void doOutFrame(u8 *buffer)			// MCU ==> PC   ，判断frameOutRxBuffer地址是否是否是给我的，如果不是，就转发
{
	u8 dstAddr,srcAddr;
	stCommHeader *pHead;
	pHead = (stCommHeader *)buffer;
  if (pHead->destAddr != myAddr)   //转发
	{
		sendOutToNext(frameOutRxBuffer,pHead->len);
		return;
	}

}
//u8 frameInRxBuffer[UART_RX_SIZE];  // PC ==> MCU  4口
//u8 frameInTxBuffer[UART_RX_SIZE];	

//u8 frameOutRxBuffer[UART_RX_SIZE];  // mcu->pc   5口
//u8 frameOutTxBuffer[UART_RX_SIZE];   

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
						for (i = 0; i < pHead->len; i++) 
						frameInRxBuffer[i] = DeFifo(&UartCommInRxFifo);
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
void getOutFrame(void)   //  MCU => PC  得到数据将UartCommOutRxFifo数据存入frameOutRxBuffer
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
				len = FifoLen(&UartCommOutRxFifo);  //fifo队列总共多长
				if (len >= pHead->len) 
					{
						for (i = 0; i < pHead->len; i++) frameOutRxBuffer[i] = DeFifo(&UartCommOutRxFifo);
						//check crc16
						crc16 = crc16_modbus(frameOutRxBuffer,pHead->len - 2);
						crc16_rcv = frameOutRxBuffer[pHead->len - 2] + (frameOutRxBuffer[pHead->len - 1] * 256);
						if (crc16 == crc16_rcv)
							{
								doOutFrame(frameOutRxBuffer);  
								Trace_Print(" SEND OK \r\n");
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

