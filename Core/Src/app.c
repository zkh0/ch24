#include "main.h"
/**************************************************************************************************************/
/**************************************************************************************************************/
/**************************************************************************************************************/
#define 	 VOLTAGE_AMP    3.0f
#define 	 SAMPLE_R				0.2f

/**************************************************************************************************************/
__attribute__((aligned(8)))
stUserPara userPara;

/**************************************************************************************************************/
u8  cleanFinished = 0;

enumTestState cleanState = state_reset;
enumCrystalPoint crystalPoint= crystal_before_clean;
u16 crystalRes[CRYSTAL_NUM][max_crystal_point];
u32 tick;
/**************************************************************************************************************/
#define 	ADC_BUFFER_SIZE	 (5*8)

__attribute__((aligned(4)))
u16 ADCBuffer1[ADC_BUFFER_SIZE];

__attribute__((aligned(4)))
u16 ADCBuffer2[ADC_BUFFER_SIZE];

__attribute__((aligned(4)))
u16 ADCBuffer3[ADC_BUFFER_SIZE];

__attribute__((aligned(4)))
u16 ADCBuffer4[ADC_BUFFER_SIZE];

__attribute__((aligned(4)))
u16 ADCBuffer5[ADC_BUFFER_SIZE];

__attribute__((aligned(4)))
u16 ADCAllBuffer[CRYSTAL_NUM];

/**************************************************************************************************************/
u16 ADC1_2Value;

/**************************************************************************************************************/
extern u8 myAddr; 
/**************************************************************************************************************/


/**************************************************************************************************************/


/**************************************************************************************************************/


/**************************************************************************************************************/

/**************************************************************************************************************/


/**************************************************************************************************************/

/**************************************************************************************************************/

u16 calcCrystalRes(u16 AD,u8 ch)    // mv, 返回电阻值,  
{
		int i;
		float voltage;
		float res;
		float m;


		if (userPara.magicData == MAGIC_DATA)			//使用校准参数
			{
					m = AD + userPara.ampOffset[ch];
			}
		else m = AD;

		voltage = (m * 3300.0f) / 4096.0f ;
		voltage = voltage / VOLTAGE_AMP;



		res = (float)voltage  * 10000.0f / (float)(1200 - voltage)  - 100 - SAMPLE_R;

		//Trace_Print(" res = %0.2f   \r\n",res);


		if (res > 65000) res = 65000;
		if (res <0) res = 0;
		
		return (u16)res;
}


/**************************************************************************************************************/
void startAllCh(void)
{
		//T1,T2,T8,T9
	LL_GPIO_SetOutputPin(GPIOC, EN1_Pin | EN2_Pin | EN8_Pin | EN9_Pin);

	//T3-7, T13-16
	LL_GPIO_SetOutputPin(GPIOG, EN3_Pin | EN4_Pin | EN5_Pin | EN6_Pin | EN7_Pin | EN13_Pin | EN14_Pin | EN15_Pin | EN16_Pin );

	//T10-12
	LL_GPIO_SetOutputPin(GPIOA, EN8_Pin | EN9_Pin | EN10_Pin);

	//T17-22
	LL_GPIO_SetOutputPin(GPIOD, EN17_Pin | EN18_Pin | EN19_Pin | EN20_Pin | EN21_Pin | EN22_Pin);

	// T23,24
	LL_GPIO_SetOutputPin(GPIOB, EN23_Pin | EN24_Pin );
}

void stopAllCh(void)
{
	//T1,T2,T8,T9
	LL_GPIO_ResetOutputPin(GPIOC, EN1_Pin | EN2_Pin | EN8_Pin | EN9_Pin);

	//T3-7, T13-16
	LL_GPIO_ResetOutputPin(GPIOG, EN3_Pin | EN4_Pin | EN5_Pin | EN6_Pin | EN7_Pin |
	EN13_Pin | EN14_Pin | EN15_Pin | EN16_Pin 
	);

	//T10-12
	LL_GPIO_ResetOutputPin(GPIOA, EN8_Pin | EN9_Pin | EN10_Pin);

	//T17-22
	LL_GPIO_ResetOutputPin(GPIOD, EN17_Pin | EN18_Pin | EN19_Pin | EN20_Pin | EN21_Pin | EN22_Pin);

	// T23,24
	LL_GPIO_ResetOutputPin(GPIOB, EN23_Pin | EN24_Pin );
}
/**************************************************************************************************************/
void  setOverCurrent(void)   //DAC1 1.2v   DAC2   1
{
	u16 value = 0;
	if (value > 4095) value = 4095;
		 
		 // 设置DAC输出值（12位右对齐模式）

	LL_DAC_Enable(DAC1, LL_DAC_CHANNEL_1);
	LL_DAC_Enable(DAC2, LL_DAC_CHANNEL_1);

	LL_DAC_ConvertData12RightAligned(DAC1, LL_DAC_CHANNEL_1, 1494);			// 1.2v AD 测量值为1.2v
	LL_DAC_ConvertData12RightAligned(DAC2, LL_DAC_CHANNEL_1, 496);      // 0.4v, 2A
	clearOverCurrentFlag();

}

/**************************************************************************************************************/
void clearOverCurrentFlag(void)
{
	LL_GPIO_ResetOutputPin(D_CLR_GPIO_Port, D_CLR_Pin);
	hwDelayms(10);
	LL_GPIO_SetOutputPin(D_CLR_GPIO_Port, D_CLR_Pin);
	hwDelayms(10);
}

/**************************************************************************************************************/
void traceCrystalRes(void)
{
	int i;
	for (i=0;i<CRYSTAL_NUM;i++) 				
	{
			Trace_Print(" [%d] = %d \r\n",i+1,calcCrystalRes(ADCAllBuffer[i],i));
			delayms(2);
	}
}

/**************************************************************************************************************/


void cleanMain(void)
{

	switch(cleanState)
		{
			case state_reset:
				//Trace_Print("state reset \r\n");
				if (checkStart()) 
				{
						cleanState = state_idle;
						initVar();
						setOverCurrent();
						cleanFinished = 0;
					}
				break;
			case state_idle:
				if (checkStart())
					{
						Trace_Print(" start detect crystal\r\n");
						cleanState = state_detect;
						measureVoltage(ON);
						hwDelayms(20);				// 等待电源稳定
					}
				break;
			case state_detect:
				 	cleanState = state_clean;
					ADCMeasure();
					updateCrystalRes(crystal_before_clean);			
					measureVoltage(OFF);
					hwDelayms(1);
					traceCrystalRes();
				break;

			case state_clean:
				clean();
				
			
				updateCrystalRes(crystal_after_clean);
				hwDelayms(500);
				
					{
						Trace_Print(" clean finished!  now ready to detect again\r\n");
						cleanState = state_wait;
						cleanFinished = 1;
					}
				
				break;
			case state_detect_after_clean:
				
					hwDelayms(1);
					cleanState = state_wait;

				break;
					
			case state_calibration:
					stopAllCh();
					hwDelayms(10);
					measureVoltage(ON);
					hwDelayms(10);
					ADCMeasure();
					measureVoltage(OFF);
					calibration();
					hwDelayms(100);
					cleanState = state_reset;
				break;
					
			case state_wait:
				if (checkStart() == 0) cleanState = state_idle;
				break;
			
			default:break;
		}
	
}

/**************************************************************************************************************/
u8 checkStart(void)  // 1 is on  0 is off
{
	if (LL_GPIO_IsInputPinSet(START_GPIO_Port, START_Pin) == 0) return 1;
	return 0;
}


void getCrystalState(enumCrystalPoint crystalPoint)   //晶体状态时间点，清洗前，清洗后
{

	
}

void measureVoltage(u8 onoff)
{
	if (onoff == ON)
		{
			LL_GPIO_SetOutputPin(EN_1_2V_GPIO_Port, EN_1_2V_Pin);
		}
	else if (onoff == OFF)
		{
			LL_GPIO_ResetOutputPin(EN_1_2V_GPIO_Port, EN_1_2V_Pin);
		}
}
/**************************************************************************************************************/
void checkCrystal(void)			//判断晶体有无烧断，并记录时间，
{	
	int i;
	u32 m;
	u32 t;
	getCrystalState(crystal_after_clean);
	m = crystalRes[0][crystal_after_clean] ^ crystalRes[0][crystal_temp];
	
	crystalRes[0][crystal_temp] = crystalRes[0][crystal_after_clean];
}


void updateCrystalRes(enumCrystalPoint state) 
{	
	int i;

	for (i=0;i<CRYSTAL_NUM;i++)
		crystalRes[i][state] = calcCrystalRes(ADCAllBuffer[i],i);

}





u16 getCleanTime(void)
{
	return 1;
}


/**************************************************************************************************************/
void initVar(void)
{
	int i;

	readpara();

}

/**************************************************************************************************************/
void initADC(void)   //初始化
{
	LL_DMA_SetMemoryAddress(DMA1, LL_DMA_CHANNEL_1, (uint32_t)&ADCBuffer1);
	LL_DMA_SetMemoryAddress(DMA1, LL_DMA_CHANNEL_2, (uint32_t)&ADCBuffer2);
	LL_DMA_SetMemoryAddress(DMA1, LL_DMA_CHANNEL_3, (uint32_t)&ADCBuffer3);
	LL_DMA_SetMemoryAddress(DMA1, LL_DMA_CHANNEL_4, (uint32_t)&ADCBuffer4);
	LL_DMA_SetMemoryAddress(DMA1, LL_DMA_CHANNEL_5, (uint32_t)&ADCBuffer5);

	LL_DMA_SetPeriphAddress(DMA1, LL_DMA_CHANNEL_1, LL_ADC_DMA_GetRegAddr(ADC1, LL_ADC_DMA_REG_REGULAR_DATA));
	LL_DMA_SetPeriphAddress(DMA1, LL_DMA_CHANNEL_2, LL_ADC_DMA_GetRegAddr(ADC2, LL_ADC_DMA_REG_REGULAR_DATA));
	LL_DMA_SetPeriphAddress(DMA1, LL_DMA_CHANNEL_3, LL_ADC_DMA_GetRegAddr(ADC3, LL_ADC_DMA_REG_REGULAR_DATA));
	LL_DMA_SetPeriphAddress(DMA1, LL_DMA_CHANNEL_4, LL_ADC_DMA_GetRegAddr(ADC4, LL_ADC_DMA_REG_REGULAR_DATA));
	LL_DMA_SetPeriphAddress(DMA1, LL_DMA_CHANNEL_5, LL_ADC_DMA_GetRegAddr(ADC5, LL_ADC_DMA_REG_REGULAR_DATA));
	
	
	LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_1, ADC_BUFFER_SIZE);
	LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_2, ADC_BUFFER_SIZE);
	LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_3, ADC_BUFFER_SIZE);
	LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_4, ADC_BUFFER_SIZE);
	LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_5, ADC_BUFFER_SIZE);


	LL_ADC_REG_SetDMATransfer(ADC1,LL_ADC_REG_DMA_TRANSFER_LIMITED);
	LL_ADC_REG_SetDMATransfer(ADC2,LL_ADC_REG_DMA_TRANSFER_LIMITED);
	LL_ADC_REG_SetDMATransfer(ADC3,LL_ADC_REG_DMA_TRANSFER_LIMITED);
	LL_ADC_REG_SetDMATransfer(ADC4,LL_ADC_REG_DMA_TRANSFER_LIMITED);
	LL_ADC_REG_SetDMATransfer(ADC5,LL_ADC_REG_DMA_TRANSFER_LIMITED);


	LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_1);
	LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_2);
	LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_3);
	LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_4);
	LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_5);

	LL_ADC_Enable(ADC1);
	LL_ADC_Enable(ADC2);
	LL_ADC_Enable(ADC3);
	LL_ADC_Enable(ADC4);
	LL_ADC_Enable(ADC5);
	
}

u16 calcAverage(u16 *buffer,u16 index)
{
	u32 sum= 0,i;
	for (i=0;i<8;i++)
		{
			sum += buffer[index + 5 * i];
		}
	return sum >> 3;
}

/**************************************************************************************************************/
void calibration(void)
{
	int i;
	for (i=0;i<24;i++)
		userPara.ampOffset[i] = calibrationOne(ADCAllBuffer[i]);

	userPara.magicData = MAGIC_DATA;
	writePara();
	
}

volatile float offset;
volatile float voltage;

float calibrationOne(u16 AD)   //返回偏差值，校准电阻用100欧
{
		int i;


		float m;

		
		//voltage = (m * 3300.0f) / 4096.0f ;
		//voltage = voltage / VOLTAGE_AMP;
		

		//100欧校准电阻时，AD值        v = 1200*R2/(R1+R2) * VOLTAGE_AMP   R1=10K,R2=200 (100+100)
		
		voltage = 1200.0f * 200.0f / (10000.0f + 200.0f) * VOLTAGE_AMP;
		
		m = voltage / 3300.0f * 4095.0f;

		offset = (m - (float)AD );


		return offset;	





}

/**************************************************************************************************************/

void ADCMeasure(void)
{
	int i;


	LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_1, ADC_BUFFER_SIZE);
	LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_2, ADC_BUFFER_SIZE);
	LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_3, ADC_BUFFER_SIZE);
	LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_4, ADC_BUFFER_SIZE);
	LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_5, ADC_BUFFER_SIZE);


	LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_1);
	LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_2);
	LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_3);
	LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_4);
	LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_5);


	LL_ADC_REG_StartConversion(ADC1);
	LL_ADC_REG_StartConversion(ADC2);
	LL_ADC_REG_StartConversion(ADC3);
	LL_ADC_REG_StartConversion(ADC4);
	LL_ADC_REG_StartConversion(ADC5);
	
	hwDelayms(2);			//等待测量完成
	
	LL_ADC_REG_StopConversion(ADC1);
	LL_ADC_REG_StopConversion(ADC2);
	LL_ADC_REG_StopConversion(ADC3);
	LL_ADC_REG_StopConversion(ADC4);
	LL_ADC_REG_StopConversion(ADC5);

	LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_1);
	LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_2);
	LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_3);
	LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_4);
	LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_5);

	


	//combine data
	// 0-23 ==> 1-24 ch

	//ADC5
	for (i=0;i<4;i++)
		ADCAllBuffer[i] = calcAverage(ADCBuffer5,i);

	//ADC4
	for (i=0;i<5;i++)
	ADCAllBuffer[4+i] = calcAverage(ADCBuffer4,i);

	//ADC3
	ADCAllBuffer[9] =  calcAverage(ADCBuffer3,0);
	ADCAllBuffer[10] = calcAverage(ADCBuffer3,1);
	ADCAllBuffer[11] = calcAverage(ADCBuffer3,2);
	ADCAllBuffer[12] = calcAverage(ADCBuffer3,3);
	ADCAllBuffer[13] = calcAverage(ADCBuffer3,4);

	//ADC2
	ADCAllBuffer[14] = calcAverage(ADCBuffer2,0);
	ADCAllBuffer[15] = calcAverage(ADCBuffer2,1);
	ADCAllBuffer[16] = calcAverage(ADCBuffer2,2);
	ADCAllBuffer[22] = calcAverage(ADCBuffer2,3);
	ADCAllBuffer[23] = calcAverage(ADCBuffer2,4);

	//ADC1
	ADCAllBuffer[17] = calcAverage(ADCBuffer1,0);
	ADCAllBuffer[18] = calcAverage(ADCBuffer1,1);
	ADCAllBuffer[19] = calcAverage(ADCBuffer1,2);
	ADCAllBuffer[20] = calcAverage(ADCBuffer1,3);
	ADCAllBuffer[21] = calcAverage(ADCBuffer1,4);

	ADC1_2Value =calcAverage(ADCBuffer5,4);


	
}

/**************************************************************************************************************/
void delayms(u32 ms)
{
	u32 tick = getTick();
	while (1)
		{
			if ((getTick() - tick) >= ms) break;
		}
}

/**************************************************************************************************************/

/**************************************************************************************************************/
void clean(void)
{
	clearOverCurrentFlag();
	hwDelayms(1);
	startAllCh();
	hwDelayms(50);
	stopAllCh();
	clearOverCurrentFlag();


}

/**************************************************************************************************************/
void traceCrystalState(void)
{

	
}

/**************************************************************************************************************/
// 使用定时器1， 10us tick, 最大定时 650ms
void hwDelayms(u32 ms)
{
		u32 count;
		if (ms>650) ms = 650;
		count = 65535 - ms * 100;
		
		LL_TIM_DisableCounter(TIM1);
		LL_TIM_ClearFlag_UPDATE(TIM1);
		LL_TIM_SetCounter(TIM1, count);
		LL_TIM_EnableCounter(TIM1);

		while (1)
			{
				if (LL_TIM_IsActiveFlag_UPDATE(TIM1))
					{
						LL_TIM_ClearFlag_UPDATE(TIM1);
						break;
					}
			}
}

/**************************************************************************************************************/
extern __IO uint32_t uwTick;
u32 getTick(void)
{
	return uwTick;
}




/**************************************************************************************************************/



/**************************************************************************************************************/

void readpara(void)
{
	uint32_t PageError,Address;
	uint32_t *p;

  Address = PARA_ADDR;
	p = (uint32_t *)&userPara;

	if ((*(__IO uint32_t *)Address) != MAGIC_DATA) return;

  while (Address < (PARA_ADDR + sizeof(stUserPara)))
  {
    *p++ = *(__IO uint32_t *)Address;
    Address = Address + 4;
  }
	


}


volatile u32 errorNo = 0;

void writePara(void)
{
	FLASH_EraseInitTypeDef EraseInitStruct;
	uint32_t PageError,Address;
	uint32_t count =0;


	uint64_t *p;
	
  HAL_FLASH_Unlock();
 /* Clear OPTVERR bit set on virgin samples */
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPTVERR);


  /* Fill EraseInit structure*/
  EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
  EraseInitStruct.Banks       = FLASH_BANK_1;
  EraseInitStruct.Page        = PARA_PAGES;
  EraseInitStruct.NbPages     = 1;

  /* Note: If an erase operation in Flash memory also concerns data in the data or instruction cache,
     you have to make sure that these data are rewritten before they are accessed during code
     execution. If this cannot be done safely, it is recommended to flush the caches by setting the
     DCRST and ICRST bits in the FLASH_CR register. */
  if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK)
  {
		errorNo = HAL_FLASH_GetError();
  }

  /* Program the user Flash area word by word
    (area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/

  Address = PARA_ADDR;
	p = (uint64_t * )&userPara;

  while (count < (sizeof(userPara)))
  {
    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, Address, *p) == HAL_OK)
    {
      Address = Address + 8;  /* increment to next double word*/
			p++;
			count += 8;
    }
   else
    {
			errorNo = HAL_FLASH_GetError();
    }
  }

  /* Lock the Flash to disable the flash control register access (recommended
     to protect the FLASH memory against possible unwanted operation) *********/
  HAL_FLASH_Lock();

}
/**************************************************************************************************************/
uint32_t HAL_GetTick(void)
{
  return uwTick;
}

/**************************************************************************************************************/
void readMyAddr(void)
{
	u8 addr = 0;
	if (LL_GPIO_IsInputPinSet(AR1_GPIO_Port, AR1_Pin)==0) addr = 1;
	if (LL_GPIO_IsInputPinSet(AR2_GPIO_Port, AR2_Pin)==0) addr |= 0x02;
	if (LL_GPIO_IsInputPinSet(AR3_GPIO_Port, AR3_Pin)==0) addr |= 0x04;
	if (LL_GPIO_IsInputPinSet(AR4_GPIO_Port, AR4_Pin)==0) addr |= 0x08;
	myAddr = addr;
}
/**************************************************************************************************************/

/**************************************************************************************************************/

/**************************************************************************************************************/

/**************************************************************************************************************/

/**************************************************************************************************************/

/**************************************************************************************************************/

/**************************************************************************************************************/

/**************************************************************************************************************/

/**************************************************************************************************************/



















