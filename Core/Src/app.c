#include "main.h"
/**************************************************************************************************************/
enumTestState cleanState = state_reset;
enumCrystalPoint crystalPoint= crystal_before_clean;
u8 crystalState[CRYSTAL_NUM][max_crystal_point];
int crystalCleanTime[CRYSTAL_NUM];
u8 autoRunFlag=1;				// 如果autorun =0, 表示手动操作
u32 tick;
/**************************************************************************************************************/
#define 	ADC_BUFFER_SIZE	 6

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
u16 ADCAllBuffer[ADC_BUFFER_SIZE*5];

/**************************************************************************************************************/


/**************************************************************************************************************/



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


	LL_DAC_ConvertData12RightAligned(DAC1, LL_DAC_CHANNEL_1, 1489);
	LL_DAC_ConvertData12RightAligned(DAC2, LL_DAC_CHANNEL_1, 496);      // 0.4v, 2A
//	LL_DAC_TrigSWConversion(DAC1, LL_DAC_CHANNEL_1);
//	LL_DAC_TrigSWConversion(DAC2, LL_DAC_CHANNEL_1);
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

void cleanMain(void)
{

	switch(cleanState)
		{
			case state_reset:
				//Trace_Print("state reset \r\n");
				if (checkStart()) cleanState = state_idle;
				initVar();
				setOverCurrent();
				break;
			case state_idle:
				if (checkStart())
					{
						Trace_Print(" start detect crystal\r\n");
						cleanState = state_detect;
						//measureVoltage(ON);
					}
				break;
			case state_detect:
				 	cleanState = state_clean;
				  tick = getTick();
					getCrystalState(crystal_before_clean);
					getCrystalState(crystal_temp);
				break;

			case state_clean:
				clean();
				checkCrystal();
				if ((getTick() - tick) >= CLEAN_TIME_MS) 
					{
						tick = getTick();
						Trace_Print(" clean finished!  now ready to detect again\r\n");
						delayms(1);
						if (autoRunFlag) cleanState = state_detect_after_clean;
						else cleanState = state_wait;
					}
				
				break;
			case state_detect_after_clean:
					delayms(1);
					Trace_Print(" detect after clean, go to idle\r\n");
					if (autoRunFlag) cleanState = state_reset;
					else cleanState = state_wait;
					traceCrystalState();
				break;

					
			case state_wait:
				//wait start signal deactive
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

// 24 ch total
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
	m = crystalState[0][crystal_after_clean] ^ crystalState[0][crystal_temp];
	
	if (m==0)	 return;

	// m != 0
	t = getCleanTime();
	for (i=0;i<8;i++)  
		{
			if (m & 0x01) crystalCleanTime[i] = t;
			m = m >> 1;
			//Trace_Print(" t = %d \r\n",t);
		}
	crystalState[0][crystal_temp] = crystalState[0][crystal_after_clean];
	
	
}

u16 getCleanTime(void)
{
	return 1;
}


/**************************************************************************************************************/
void initVar(void)
{
	int i;
	for (i=0;i<CRYSTAL_NUM;i++)  crystalCleanTime[i] = -1;
	
}

/**************************************************************************************************************/
void initADC(void)   //初始化
{
	
	LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_1);
	
	





}

void startADC(void)
{
	LL_ADC_REG_StartConversion(ADC1);

	hwDelayms(5);			//等待测量完成
	
	
}

/**************************************************************************************************************/
void delayms(u32 ms)
{
	u64 tick = getTick();
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
//	startAllCh();
	hwDelayms(50);
	stopAllCh();

	measureVoltage(ON);
	hwDelayms(50);

	
}

/**************************************************************************************************************/
void traceCrystalState(void)
{
	int i;
	u8 m;
	delayms(10);
	Trace_Print(" before Clean: \r\n");
	m = crystalState[0][crystal_before_clean];
	for (i=0;i<8;i++)
		{
			Trace_Print("Y%d: ",i);
			if (m & 0x01) Trace_Print("good\r\n");
			else Trace_Print("short\r\n");
			m = m >> 1;
		}
	
	Trace_Print(" after Clean: \r\n");
	m = crystalState[0][crystal_after_clean];
	for (i=0;i<8;i++)
		{
			Trace_Print("Y%d: ",i);
			if (m & 0x01) Trace_Print("good\r\n");
			else Trace_Print("short\r\n");
			m = m >> 1;
		}
	delayms(10);

	Trace_Print(" Clean time: \r\n");
	m = crystalState[0][crystal_after_clean] ^ crystalState[0][crystal_before_clean];

	for (i=0;i<8;i++)
		{
			Trace_Print("Y%d: ",i);
			if (m & 0x01) Trace_Print(" %dus\r\n",crystalCleanTime[i]*100);
			else Trace_Print(" not changed\r\n ");
			m = m >> 1;
		}

	
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








