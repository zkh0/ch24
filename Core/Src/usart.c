#include "main.h"


/**************************************************************************************************/

u8 UartSendBuffer[UART_TX_SIZE];
u8 UartReceiveBuffer[UART_RX_SIZE];


FifoStruct UartTxFifo;
FifoStruct UartRxFifo;


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
        /* Read one byte from the receive data register */
        EnFifo(&UartRxFifo, LL_USART_ReceiveData8(USART1));
				rcvTimeCount = RECEIVE_TIMEOUT;
        
    }

    if (LL_USART_IsActiveFlag_TXE(USART1))
    {
        /* Write one byte to the transmit data register */
				if (FifoLen(&UartTxFifo)>0)
       	 	LL_USART_TransmitData8(USART1, DeFifo(&UartTxFifo));
				else LL_USART_DisableIT_TXE(USART1);

    }


}

/**************************************************************************************************/
int fputc(int ch, FILE *f)
{        
  EnFifo(&UartTxFifo,ch);
	LL_USART_EnableIT_TXE(USART1);
	return ch;
}




/**************************************************************************************************/
void initUartFifo(void)
{
	//init fifo
	InitFifo(&UartTxFifo, UartSendBuffer, UART_TX_SIZE);
	InitFifo(&UartRxFifo, UartReceiveBuffer, UART_RX_SIZE);

}





/**************************************************************************************************/



/**************************************************************************************************/




/**************************************************************************************************/



/**************************************************************************************************/



/**************************************************************************************************/











