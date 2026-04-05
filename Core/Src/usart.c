#include "main.h"


/**************************************************************************************************/

u8 UartTraceSendBuffer[UART_TX_SIZE];
u8 UartTraceReceiveBuffer[UART_RX_SIZE];

u8 UartInSendBuffer[UART_TX_SIZE];
u8 UartInReceiveBuffer[UART_RX_SIZE];

u8 UartOutSendBuffer[UART_TX_SIZE];
u8 UartOutReceiveBuffer[UART_RX_SIZE];




FifoStruct UartCommInTxFifo;			//接收串口      from PC
FifoStruct UartCommInRxFifo;

FifoStruct UartTraceTxFifo;				//调试串口
FifoStruct UartTraceRxFifo;


FifoStruct UartCommOutTxFifo;			//发送串口      to MCU
FifoStruct UartCommOutRxFifo;

/**************************************************************************************************/

/**************************************************************************************************/



//解决HAL库使用时,某些情况可能报错的bug
int _ttywrch(int ch)    
{
    ch=ch;
	return ch;
}
//标准库需要的支持函数      
#if 1
__asm(".global __use_no_semihosting");

struct FILE
{
        int handle;
};

#else
#pragma import(__use_no_semihosting)  

struct __FILE 
{ 
	int handle; 
	/* Whatever you require here. If the only file you are using is */ 
	/* standard output using printf() for debugging, no file handling */ 
	/* is required. */ 
}; 
#endif

FILE __stdout;       
void _sys_exit(int x) 
{ 
	x = x; 
} 

/**************************************************************************************************/



/**
*\*\name    USARTy_IRQHandler.
*\*\fun     This function handles USARTy global interrupt request.
*\*\param   none
*\*\return  none 
**/
void USART1_IRQHandler(void)
{

    if (LL_USART_IsActiveFlag_RXNE(USART1))
    {
        while (1)
        	{
        		 if (LL_USART_IsActiveFlag_RXNE(USART1))
		        	EnFifo(&UartTraceRxFifo, LL_USART_ReceiveData8(USART1));
						 else break;
        	}
    }

    if (LL_USART_IsActiveFlag_TXE(USART1))
    {
        while (1)
        	{
	        	if (LL_USART_IsActiveFlag_TXE(USART1))
	        		{
								if (FifoLen(&UartTraceTxFifo)>0)
									{
				       	 			LL_USART_TransmitData8(USART1, DeFifo(&UartTraceTxFifo));
									}
								else 
									{
										LL_USART_DisableIT_TXE(USART1);
										break;
									}
	        		}
						else break;
        	}
    }

		if (LL_USART_IsActiveFlag_IDLE(USART1))			//注意，可能还有数据，接收完
			{
					LL_USART_ClearFlag_IDLE(USART1);
				  while (1)
        	{
        		 if (LL_USART_IsActiveFlag_RXNE(USART1))
		        	EnFifo(&UartTraceRxFifo, LL_USART_ReceiveData8(USART1));
						 else break;
        	}
					
			}
		
}

/**************************************************************************************************/
int fputc(int ch, FILE *f)
{        
  EnFifo(&UartTraceTxFifo,ch);
	LL_USART_EnableIT_TXE(USART1);
	return ch;
}




/**************************************************************************************************/
void initUartFifo(void)
{
	//init fifo
	InitFifo(&UartTraceTxFifo, UartTraceSendBuffer, UART_TX_SIZE);
	InitFifo(&UartTraceRxFifo, UartTraceReceiveBuffer, UART_RX_SIZE);

	InitFifo(&UartCommOutTxFifo, UartOutSendBuffer, UART_TX_SIZE);
	InitFifo(&UartCommOutRxFifo, UartOutReceiveBuffer, UART_RX_SIZE);

	InitFifo(&UartCommInTxFifo, UartInSendBuffer, UART_TX_SIZE);
	InitFifo(&UartCommInRxFifo, UartInReceiveBuffer, UART_RX_SIZE);

}

/**************************************************************************************************/

/**************************************************************************************************/

/**************************************************************************************************/

/**************************************************************************************************/
/**************************************************************************************************/

/**************************************************************************************************/
// PC ==> MCU
void uart4Func(void)
{

    if (LL_USART_IsActiveFlag_RXNE(UART4))
    {
        while (1)
        	{
        		 if (LL_USART_IsActiveFlag_RXNE(UART4))
		        	EnFifo(&UartCommInRxFifo, LL_USART_ReceiveData8(UART4));
						 else break;
        	}
    }

    if (LL_USART_IsActiveFlag_TXE(UART4))
    {
        while (1)
        	{
	        	if (LL_USART_IsActiveFlag_TXE(UART4))
	        		{
								if (FifoLen(&UartCommInTxFifo)>0)
									{
				       	 			LL_USART_TransmitData8(UART4, DeFifo(&UartCommInTxFifo));
									}
								else 
									{
										LL_USART_DisableIT_TXE(UART4);
										break;
									}
	        		}
						else break;
        	}
    }

		
		if (LL_USART_IsActiveFlag_IDLE(UART4))			//注意，可能还有数据，接收完
			{
					LL_USART_ClearFlag_IDLE(UART4);
				  while (1)
        	{
        		 if (LL_USART_IsActiveFlag_RXNE(UART4))
		        	EnFifo(&UartCommInRxFifo, LL_USART_ReceiveData8(UART4));
						 else break;
        	}
					
			}

		
}


/**************************************************************************************************/
// MCU ==> PC
void uart5Func(void)
{

    if (LL_USART_IsActiveFlag_RXNE(UART5))
    {
        while (1)
        	{
        		 if (LL_USART_IsActiveFlag_RXNE(UART5))
		        	EnFifo(&UartCommOutRxFifo, LL_USART_ReceiveData8(UART5));
						 else break;
        	}
    }
		
    if (LL_USART_IsActiveFlag_TXE(UART5))
    {
        while (1)
        	{
	        	if (LL_USART_IsActiveFlag_TXE(UART5))
	        		{
								if (FifoLen(&UartCommOutTxFifo)>0)
									{
				       	 			LL_USART_TransmitData8(UART5, DeFifo(&UartCommOutTxFifo));
									}
								else 
									{
										LL_USART_DisableIT_TXE(UART5);
										break;
									}
	        		}
						else break;
        	}
    }		
		if (LL_USART_IsActiveFlag_IDLE(UART5))			//注意，可能还有数据，接收完
			{
					LL_USART_ClearFlag_IDLE(UART5);
				  while (1)
        	{
        		 if (LL_USART_IsActiveFlag_RXNE(UART5))
		        	EnFifo(&UartCommOutRxFifo, LL_USART_ReceiveData8(UART5));
						 else break;
        	}
			}

}



/**************************************************************************************************/



/**************************************************************************************************/



/**************************************************************************************************/











