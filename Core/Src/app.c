#include "main.h"
/**************************************************************************************************************/
/**************************************************************************************************************/
/**************************************************************************************************************/
#define 	 VOLTAGE_AMP    3.0f
#define 	 SAMPLE_R				0.2f

/**************************************************************************************************************/
__attribute__((aligned(8)))
stUserPara userPara;
UserSet       userset = 
{
	0x12345678,      
	50,
	10,
	
	10,
	20,
	1,
	2,
	
	1200,
	10000,
	65000,
	30,
	5,
	0XFF,
};
	
/**************************************************************************************************************/
volatile u8  cleanFinished = 0;
enumTestState cleanState = state_reset;                //状态
volatile u8 cleancount = 0;
u32 stateR = 0;
enumCrystalPoint crystalPoint= crystal_before_clean;   //

u8 ErrorState = 1;

u16 crystalRes[CRYSTAL_NUM][max_crystal_point];        //24路电阻结果
u32 tick;
volatile u8 checkflag = 0;  //调试检测位
/**************************************************************************************************************/
#define 	ADC_BUFFER_SIZE	 (5*8)  

__attribute__((aligned(4)))
u16 ADCBuffer1[ADC_BUFFER_SIZE];                 //adc/DMA缓冲区

__attribute__((aligned(4)))
u16 ADCBuffer2[ADC_BUFFER_SIZE];

__attribute__((aligned(4)))
u16 ADCBuffer3[ADC_BUFFER_SIZE];

__attribute__((aligned(4)))
u16 ADCBuffer4[ADC_BUFFER_SIZE];

__attribute__((aligned(4)))
u16 ADCBuffer5[ADC_BUFFER_SIZE];

__attribute__((aligned(4)))
u16 ADCAllBuffer[CRYSTAL_NUM];                  //adc所有值

/**************************************************************************************************************/
u16 ADC1_2Value;

/**************************************************************************************************************/
extern u8 myAddr; 
extern u8 frameOutTxBuffer[UART_RX_SIZE]; 
/**************************************************************************************************************/

/**************************************************************************************************************/

u16 calcCrystalRes(u16 AD,u8 ch)    // mv, 传输值 = 偏差值+测量值,  一路的阻值
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

    if (voltage >= 1200.0f) voltage = 1199.0f; //消除除零错误

		res = (float)voltage  * 10000.0f / (float)(1200 - voltage)  - 100 - SAMPLE_R;

		//Trace_Print(" res = %0.2f   \r\n",res);


		if (res > 65000) res = 65000;  
		if (res <0) res = 0;
		
		return (u16)res;
}


/**************************************************************************************************************/
void startAllCh(void)   //打开所有通道
{
		//T1,T2,T8,T9
	LL_GPIO_SetOutputPin(GPIOC, EN1_Pin | EN2_Pin | EN8_Pin | EN9_Pin);

	//T3-7, T13-16
	LL_GPIO_SetOutputPin(GPIOG, EN3_Pin | EN4_Pin | EN5_Pin | EN6_Pin | EN7_Pin | EN13_Pin | EN14_Pin | EN15_Pin | EN16_Pin );

	//T10-12
	LL_GPIO_SetOutputPin(GPIOA, EN10_Pin | EN11_Pin | EN12_Pin);//修改，原来是PA8,9,10

	//T17-22
	LL_GPIO_SetOutputPin(GPIOD, EN17_Pin | EN18_Pin | EN19_Pin | EN20_Pin | EN21_Pin | EN22_Pin);
	// T23,24
}
void stopAllCh(void)   //关闭所有通道
{
	//T1,T2,T8,T9
	LL_GPIO_ResetOutputPin(GPIOC, EN1_Pin | EN2_Pin | EN8_Pin | EN9_Pin);

	//T3-7, T13-16
	LL_GPIO_ResetOutputPin(GPIOG, EN3_Pin | EN4_Pin | EN5_Pin | EN6_Pin | EN7_Pin |
	EN13_Pin | EN14_Pin | EN15_Pin | EN16_Pin 
	);

	//T10-12
	LL_GPIO_ResetOutputPin(GPIOA, EN10_Pin | EN11_Pin | EN12_Pin);//修改，原来是PA8,9,10

	//T17-22
	LL_GPIO_ResetOutputPin(GPIOD, EN17_Pin | EN18_Pin | EN19_Pin | EN20_Pin | EN21_Pin | EN22_Pin);

	// T23,24
	LL_GPIO_ResetOutputPin(GPIOB, EN23_Pin | EN24_Pin );
}

/**************************************************************************************************************/
void OpenCh(void)      //打开合适通道
{
	 int loop = stateR;
	if(userset.updatecontrol.bits.CleanSuccessedNoExcitation)
   {
			loop = 0xff;
	 }
  	 for(int i =0;i<24;i++)
			{
				if((loop & 0x01) != 1){LL_GPIO_ResetOutputPin(GPIOC,EN1_Pin);}else{LL_GPIO_SetOutputPin(GPIOC,EN1_Pin);} loop = loop>>1;
				if((loop & 0x01) != 1){LL_GPIO_ResetOutputPin(GPIOC,EN2_Pin);}else{LL_GPIO_SetOutputPin(GPIOC,EN2_Pin);} loop = loop>>1;
				if((loop & 0x01) != 1){LL_GPIO_ResetOutputPin(GPIOG,EN3_Pin);}else{LL_GPIO_SetOutputPin(GPIOG,EN3_Pin);} loop = loop>>1;
				if((loop & 0x01) != 1){LL_GPIO_ResetOutputPin(GPIOG,EN4_Pin);}else{LL_GPIO_SetOutputPin(GPIOG,EN4_Pin);} loop = loop>>1;
				if((loop & 0x01) != 1){LL_GPIO_ResetOutputPin(GPIOG,EN5_Pin);}else{LL_GPIO_SetOutputPin(GPIOG,EN5_Pin);} loop = loop>>1;
				if((loop & 0x01) != 1){LL_GPIO_ResetOutputPin(GPIOG,EN6_Pin);}else{LL_GPIO_SetOutputPin(GPIOG,EN6_Pin);} loop = loop>>1;
				if((loop & 0x01) != 1){LL_GPIO_ResetOutputPin(GPIOG,EN7_Pin);}else{LL_GPIO_SetOutputPin(GPIOG,EN7_Pin);} loop = loop>>1;
				if((loop & 0x01) != 1){LL_GPIO_ResetOutputPin(GPIOC,EN8_Pin);}else{LL_GPIO_SetOutputPin(GPIOC,EN8_Pin);} loop = loop>>1;
				if((loop & 0x01) != 1){LL_GPIO_ResetOutputPin(GPIOC,EN2_Pin);}else{LL_GPIO_SetOutputPin(GPIOC,EN2_Pin);} loop = loop>>1;
				if((loop & 0x01) != 1){LL_GPIO_ResetOutputPin(GPIOA,EN10_Pin);}else{LL_GPIO_SetOutputPin(GPIOA,EN10_Pin);} loop = loop>>1;
				if((loop & 0x01) != 1){LL_GPIO_ResetOutputPin(GPIOA,EN11_Pin);}else{LL_GPIO_SetOutputPin(GPIOA,EN11_Pin);} loop = loop>>1;
				if((loop & 0x01) != 1){LL_GPIO_ResetOutputPin(GPIOA,EN12_Pin);}else{LL_GPIO_SetOutputPin(GPIOA,EN12_Pin);} loop = loop>>1;
				if((loop & 0x01) != 1){LL_GPIO_ResetOutputPin(GPIOG,EN13_Pin);}else{LL_GPIO_SetOutputPin(GPIOG,EN13_Pin);} loop = loop>>1;
				if((loop & 0x01) != 1){LL_GPIO_ResetOutputPin(GPIOG,EN14_Pin);}else{LL_GPIO_SetOutputPin(GPIOG,EN14_Pin);} loop = loop>>1;
				if((loop & 0x01) != 1){LL_GPIO_ResetOutputPin(GPIOG,EN15_Pin);}else{LL_GPIO_SetOutputPin(GPIOG,EN15_Pin);} loop = loop>>1;
				if((loop & 0x01) != 1){LL_GPIO_ResetOutputPin(GPIOG,EN16_Pin);}else{LL_GPIO_SetOutputPin(GPIOG,EN16_Pin);} loop = loop>>1;
				if((loop & 0x01) != 1){LL_GPIO_ResetOutputPin(GPIOD,EN17_Pin);}else{LL_GPIO_SetOutputPin(GPIOD,EN17_Pin);} loop = loop>>1;
				if((loop & 0x01) != 1){LL_GPIO_ResetOutputPin(GPIOD,EN18_Pin);}else{LL_GPIO_SetOutputPin(GPIOD,EN18_Pin);} loop = loop>>1;
				if((loop & 0x01) != 1){LL_GPIO_ResetOutputPin(GPIOD,EN19_Pin);}else{LL_GPIO_SetOutputPin(GPIOD,EN19_Pin);} loop = loop>>1;
				if((loop & 0x01) != 1){LL_GPIO_ResetOutputPin(GPIOD,EN20_Pin);}else{LL_GPIO_SetOutputPin(GPIOD,EN20_Pin);} loop = loop>>1;
				if((loop & 0x01) != 1){LL_GPIO_ResetOutputPin(GPIOD,EN21_Pin);}else{LL_GPIO_SetOutputPin(GPIOD,EN21_Pin);} loop = loop>>1;
				if((loop & 0x01) != 1){LL_GPIO_ResetOutputPin(GPIOD,EN22_Pin);}else{LL_GPIO_SetOutputPin(GPIOD,EN22_Pin);} loop = loop>>1;
				if((loop & 0x01) != 1){LL_GPIO_ResetOutputPin(GPIOB,EN23_Pin);}else{LL_GPIO_SetOutputPin(GPIOB,EN23_Pin);} loop = loop>>1;
				if((loop & 0x01) != 1){LL_GPIO_ResetOutputPin(GPIOB,EN24_Pin);}else{LL_GPIO_SetOutputPin(GPIOB,EN24_Pin);} loop = loop>>1;
			}
	
	
			
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
void clearOverCurrentFlag(void)    //清理过流标志位
{
	LL_GPIO_ResetOutputPin(D_CLR_GPIO_Port, D_CLR_Pin);
	hwDelayms(10);
	LL_GPIO_SetOutputPin(D_CLR_GPIO_Port, D_CLR_Pin);
	hwDelayms(10);
}
/**************************************************************************************************************/

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
					Trace_Print("come in \r\n");
						cleanState = state_idle;
						initVar();
						setOverCurrent();
						cleanFinished = 0;
					  //CountTimes = 0;
					}
				break;
			case state_idle:
				if (checkStart())
					{
						Trace_Print(" state_idle\r\n");
						cleanState = state_detect;
						AllADCMeasure();
					}
				break;
			case state_detect:
				 	cleanState = state_first_clean;
					
			    updateCrystalRes(crystal_before_clean);
					hwDelayms(1);
			    sendDataToPC(crystal_before_clean); 
		    	hwDelayms(20);
			
			  	traceCrystalRes();
			    Trace_Print(" updateCrystalRes successful\r\n");
				break;
      
			case state_first_clean:
					if(userset.updatecontrol.bits.FristAllClean)
				{
					clearOverCurrentFlag();
					hwDelayms(userset.CleanBeforeDelayTime);
					startAllCh();
					hwDelayms(userset.FirstCleanTime);
					stopAllCh();
					hwDelayms(userset.CleanAfterDelayTime);
				
					AllADCMeasure();
					updateCrystalRes(crystal_first_clean);	  //ADC转换阻值
					updateStateCh(crystal_first_clean);      //更新通道
					hwDelayms(1);  
				
					sendDataToPC(crystal_first_clean);  //发送
				
					hwDelayms(20);
					clearOverCurrentFlag();
				}
				 cleanState = state_clean;
			  break;
				
			case state_clean:
					
			    clean();  //清洗
			
					AllADCMeasure();
					updateCrystalRes(crystal_after_clean);	
					hwDelayms(1); 
			    sendDataToPC(crystal_after_clean);
				  hwDelayms(20);
	
					Trace_Print(" clean finished!  now ready to detect again\r\n");
					cleanState = state_wait;
					cleanFinished = 1;
				break;
			
			case state_detect_after_clean:
				
					hwDelayms(1);
					cleanState = state_wait;
				break;
					
			case state_calibration:
					stopAllCh();
					hwDelayms(10);
					AllADCMeasure();
					calibration();
					hwDelayms(100);
					cleanState = state_reset;
				break;
					
			case state_wait:
				 checkflag = 0;
				if (checkStart() == 0) cleanState = state_idle;
				break;
			
			case state_error:
				stopAllCh();
				cleanState = state_idle; 
			default:break;
		}

}


	

/**************************************************************************************************************/
void sendFlagToPC(enumCrystalPoint state)    //发送到pc
{
		int i;
		int Bdata;  
		u8 len = 12;               //(2帧头+ 1ID+ 1目标地址+ 1源地址+ 1长度)  6个+3数据+2个校验位
		u16 crc16;
	 
		frameOutTxBuffer[0] = HEAD1;
		frameOutTxBuffer[1] = HEAD2;
		
		frameOutTxBuffer[2] = CMD_ID_MEASURE | 0x80;   //0x04  ，发送id0x84
		frameOutTxBuffer[3] = PC_ADDR;
		frameOutTxBuffer[4] = myAddr;
		
		frameOutTxBuffer[5] = len; 
		frameOutTxBuffer[6] = state;


			for (i=0;i<CRYSTAL_NUM;i++) 				
					{
						
							if (crystalRes[CRYSTAL_NUM-1-i][state] > userset.MinOkR  )  
								Bdata |= 0x01;
							else Bdata &= ~0x01;
								Bdata = Bdata << 1;
					}
			
					 frameOutTxBuffer[7] = (Bdata>>16)&0xff;   // 17~24路
					 frameOutTxBuffer[8] = (Bdata>>8)&0xff;   // 9~16路
					 frameOutTxBuffer[9] = (Bdata)&0xff;   // 1~8路
			
				 crc16 = crc16_modbus(frameOutTxBuffer,len - 2);
				 frameOutTxBuffer[10] = (crc16>>8)&0xff;
				 frameOutTxBuffer[11] = crc16&0xff;
				 sendOutToNext(frameOutTxBuffer,len);
}

/**************************************************************************************************************/
void sendDataToPC(enumCrystalPoint date)    //发送阻值到pc
{
	int i;
	u16 ret = 0;
	int Bdata;  
	u8 len = 58;  //2帧头+ 1ID+ 1目标地址+ 1源地址+ 1长度+ 2测量计数值+ 24通道*2+ 2个校验位
	u16 crc16;

	frameOutTxBuffer[0] = HEAD1;
	frameOutTxBuffer[1] = HEAD2;
	frameOutTxBuffer[2] = CMD_ID_IMPEDANCE | 0x80;  // 0x05 ，发送ID 0x85
	frameOutTxBuffer[3] = PC_ADDR;
	frameOutTxBuffer[4] = myAddr;
	frameOutTxBuffer[5] = len;
  frameOutTxBuffer[6] = date ;
	frameOutTxBuffer[7] = 0 ;

	for (i=0;i<CRYSTAL_NUM;i++) 				
	{
		ret = crystalRes[i][date];

		frameOutTxBuffer[2*i+8] = ret & 0xff;
		frameOutTxBuffer[2*i+9] = (ret >> 8) & 0xff;

	}
		 crc16 = crc16_modbus(frameOutTxBuffer,len - 2);
		 frameOutTxBuffer[56] = crc16&0xff;     //先发低位
		 frameOutTxBuffer[57] = (crc16>>8)&0xff;  //高位
	
	sendOutToNext(frameOutTxBuffer,len);
}

/**************************************************************************************************************/
//void SendFlagORDate(enumCrystalPoint stateOrdate)
//{
//	if(userset.updatecontrol.bits.CleanImpedanceNoUpdata)
//   {
//		 sendFlagToPC(stateOrdate);
//	 }
//	  sendDataToPC(stateOrdate);
//}
/**************************************************************************************************************/
void SendFlagORDate(enumCrystalPoint stateOrdate)
{
	if(userset.updatecontrol.bits.CleanImpedanceNoUpdata)   //判断上传阻值还是状态 1，状态 0，阻值
		{
			if(userset.updatecontrol.bits.UpdateAllS)           //判断是否上传所有状态
			{
				sendFlagToPC(stateOrdate);
			}
			if(userset.updatecontrol.bits.UpDateFirAndLastS && cleancount == userset.CleanTimes)  //判断是否只上传刚开始的状态和是否已经清理完
			{
				sendFlagToPC(crystal_before_clean);
				sendFlagToPC(userset.CleanTimes);
			}
	  }
		else
    {
			if(userset.updatecontrol.bits.UpdateAllR)
			{
				sendDataToPC(stateOrdate);
			}
			if(userset.updatecontrol.bits.UpDateFirAndLastR && cleancount == userset.CleanTimes)
			{
				sendDataToPC(crystal_before_clean);
				sendDataToPC(userset.CleanTimes);
			}
		}
	  
}
//	u8          FristAllClean  :1;                 //第一次是否全部激励
//	
//	u8          CleanSuccessedNoExcitation :1;   //清洗成功后不激励   
//	u8          CleanImpedanceNoUpdata :1;       //阻抗不上传，只上传结果  为1上传，为0，不上传
//	
//	u8          UpdateAllR :1;	                    //上传所有阻值    
//	u8          UpDateFirAndLastR :1;              //上传第一次和最后一次阻值
// 	u8          UpdateAllS :1;                     //上传所有状态          为1上传，为0不上传          
//	u8          UpDateFirAndLastS :1;              //上传第一次和最后一次状态  
//	
//	u8          UpDateBadNumber :1;                 //上传每次清洗后的不良件个数 
u8 checkStart(void)  // 1 is on  0 is off    检测输入引脚 
{
	if (LL_GPIO_IsInputPinSet(START_GPIO_Port, START_Pin) == 0 || checkflag  ) return 1;  
	return 0;
}


void getCrystalState(enumCrystalPoint crystalPoint)   //晶体状态时间点，清洗前，清洗后
{

	
}


void measureVoltage(u8 onoff)    //引脚输出1.2v电压
{
	if (onoff == ON)
		{
			LL_GPIO_SetOutputPin(EN_1_2V_GPIO_Port, EN_1_2V_Pin);  //PB7
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
/**************************************************************************************************************/
void ProbDetection(void)   // 探头检测，输出结果，1为0k，0为bad
{
		int i;
		
	int Bdata;  
	u8 len = 12;               //(2帧头+ 1ID+ 1目标地址+ 1源地址+ 1长度)  6个+3数据+2个校验位
	u16 crc16;
	
	measureVoltage(1);   //打开1.2v
 	ADCMeasure();        //值存进  ADCAllBuffer[0-23],读出来的值是ad值
	measureVoltage(2);   //关闭1.2v
	
	for (i=0;i<CRYSTAL_NUM;i++)
	crystalRes[i][crystal_after_clean] = calcCrystalRes(ADCAllBuffer[i],i);

 
	frameOutTxBuffer[0] = HEAD1;
	frameOutTxBuffer[1] = HEAD2;
	
	frameOutTxBuffer[2] = CMD_ID_PROBDETECTION | 0x80;   //0x06  ，发送id0x86
	frameOutTxBuffer[3] = PC_ADDR;
	frameOutTxBuffer[4] = myAddr;
	
	frameOutTxBuffer[5] = len;   
  frameOutTxBuffer[6] = 0x00;


	for(int i=0;i<CRYSTAL_NUM;i++) 				
	{
		
		if ( crystalRes[CRYSTAL_NUM-1-i][crystal_after_clean] > PROBDECTIONMINR || crystalRes[CRYSTAL_NUM-1-i][crystal_after_clean] < PROBDECTIONMAXR)  
			{     //值正常为1，不正常为0
		Bdata |= 0x01;
		}
		else
			{
				Bdata &= ~0x01;
			}
			  Bdata = Bdata << 1;
	}
			 //Bdata = 0x0A0B0C;
	
	   frameOutTxBuffer[7] = (Bdata>>16)&0xff;  // 24-17
       frameOutTxBuffer[8] = (Bdata>>8)&0xff;   // 16-9
       frameOutTxBuffer[9] = (Bdata)&0xff;      // 8-1
	
	   crc16 = crc16_modbus(frameOutTxBuffer,len - 2);
	   frameOutTxBuffer[10] = (crc16>>8)&0xff;
	   frameOutTxBuffer[11] = crc16&0xff;
		 sendOutToNext(frameOutTxBuffer,len);
	
}
/**************************************************************************************************************/
void updateCrystalRes(enumCrystalPoint state)  //更新电阻值
{	
	int i;

	for (i=0;i<CRYSTAL_NUM;i++)
		crystalRes[i][state] = calcCrystalRes(ADCAllBuffer[i],i);
}
void updateStateCh(enumCrystalPoint state)  //更新通道状态
{	
	int Bdata;
			for (int i=0;i<CRYSTAL_NUM;i++) 				
					{
						
							if (crystalRes[CRYSTAL_NUM-1-i][state] > userset.MinOkR  )  
								Bdata |= 0x01;
							else Bdata &= ~0x01;
								Bdata = Bdata << 1;
					}
		   stateR = Bdata;
	
}

/**************************************************************************************************************/
u16 getCleanTime(void)  //清理时间
{
	return 1;
}


/**************************************************************************************************************/
void initVar(void)  //初始读参数
{
	int i;

	readpara();
	readset();

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

u16 calcAverage(u16 *buffer,u16 index)   //8次平均
{
	u32 sum= 0,i;
	for (i=0;i<8;i++)
		{
			sum += buffer[index + 5 * i];
		}
	return sum >> 3;
}

/**************************************************************************************************************/
void calibration(void)  //校准所有通道 ，写进flash
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

void ADCMeasure(void)    //ADC采样，值存入adcallbuff
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
void AllADCMeasure(void)  //前后有延时
{     stopAllCh();
	    hwDelayms(userset.CleanAfterDelayTime);
			measureVoltage(ON);
			hwDelayms(userset.DelayBeforeMeasureTime);
			ADCMeasure();
			hwDelayms(userset.DelayAfterMeasureTime);
			measureVoltage(OFF);; 
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
	//clearOverCurrentFlag();
	//hwDelayms(10);
	//startAllCh();
	//hwDelayms(100);
	//stopAllCh();
	//hwDelayms(5);
	//startAllCh();
	//hwDelayms(100);
	//stopAllCh();
	//clearOverCurrentFlag();
		if(userset.updatecontrol.bits.FristAllClean)
		{
				clearOverCurrentFlag();
				hwDelayms(userset.CleanBeforeDelayTime);
				startAllCh();
				hwDelayms(userset.FirstCleanTime);
				stopAllCh();
				hwDelayms(userset.CleanAfterDelayTime);
			
				AllADCMeasure();
				updateCrystalRes(crystal_first_clean);	  //ADC转换阻值
				updateStateCh(crystal_first_clean);      //更新通道
				hwDelayms(1);  
			
				sendDataToPC(crystal_first_clean);  //发送
			
				hwDelayms(20);
				clearOverCurrentFlag();
			
				
						for(int i = 2;i<userset.CleanTimes;i++)         //循环清理，把得到的值送入updateCrystalRes，并发送，从第二次开始
					{
							clearOverCurrentFlag();
							hwDelayms(userset.CleanBeforeDelayTime);
							OpenCh();
							hwDelayms(userset.CleanTime);
							hwDelayms(userset.CleanAfterDelayTime);
						
							AllADCMeasure();
							updateCrystalRes(i);
							updateStateCh(i);			
							hwDelayms(1); 
						
							SendFlagORDate(i);
							hwDelayms(20);
							clearOverCurrentFlag();
					}
		  }
		  else
      {
					for(int i = 1;i<userset.CleanTimes;i++)         //循环清理，把得到的值送入updateCrystalRes，并发送，从第二次开始
					{
							clearOverCurrentFlag();
							hwDelayms(userset.CleanBeforeDelayTime);
							OpenCh();
							hwDelayms(userset.CleanTime);
							hwDelayms(userset.CleanAfterDelayTime);
						
							AllADCMeasure();
							updateCrystalRes(i);
							updateStateCh(i);			
							hwDelayms(1); 
						
							SendFlagORDate(i);
							hwDelayms(20);
							clearOverCurrentFlag();
					}
			}

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
					if(ErrorState == 0)
           {
						 cleanState = state_error;
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

void readset(void)  //读设置
{
	uint32_t PageError,Address;
	uint32_t *p;

  Address = SET_ADDR;
	p = (uint32_t *)&userset;

	if ((*(__IO uint32_t *)Address) != MAGIC_DATA) return;

  while (Address < (SET_ADDR + sizeof(UserSet)))
  {
    *p++ = *(__IO uint32_t *)Address;
    Address = Address + 4;
  }

}

volatile u32 errorParaNo = 0;
volatile u32 errorSetNo = 0;


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
  EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;  //按页擦除
  EraseInitStruct.Banks       = FLASH_BANK_1;           //存储块
  EraseInitStruct.Page        = PARA_PAGES;             //要擦除的页数
  EraseInitStruct.NbPages     = 1;                      //只擦除一页

  /* Note: If an erase operation in Flash memory also concerns data in the data or instruction cache,
     you have to make sure that these data are rewritten before they are accessed during code
     execution. If this cannot be done safely, it is recommended to flush the caches by setting the
	DCRST and ICRST bits in the FLASH_CR register. 没有cache，不需要擦除 */
  if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK)
  {
		errorParaNo = HAL_FLASH_GetError();
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
			errorParaNo = HAL_FLASH_GetError();
    }
  }

  /* Lock the Flash to disable the flash control register access (recommended
     to protect the FLASH memory against possible unwanted operation) *********/
  HAL_FLASH_Lock();

}

void writeSet(void)
{
	FLASH_EraseInitTypeDef EraseInitStruct;
	uint32_t PageError,Address;
	uint32_t count =0;
  uint64_t *p;
	
  HAL_FLASH_Unlock();
 /* Clear OPTVERR bit set on virgin samples */
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPTVERR);  //强制清除错误


  /* Fill EraseInit structure*/
  EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;  //按页擦除
  EraseInitStruct.Banks       = FLASH_BANK_1;           //存储块
  EraseInitStruct.Page        = SET_PAGES;             //要擦除的页数
  EraseInitStruct.NbPages     = 1;                      //只擦除一页

  /* Note: If an erase operation in Flash memory also concerns data in the data or instruction cache,
     you have to make sure that these data are rewritten before they are accessed during code
     execution. If this cannot be done safely, it is recommended to flush the caches by setting the
     DCRST and ICRST bits in the FLASH_CR register. */
  if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK)
  {
		errorSetNo = HAL_FLASH_GetError();
  }

  /* Program the user Flash area word by word
    (area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/

  Address = SET_ADDR;
	p = (uint64_t * )&userset;  //指向设置结构体，8字节

  while (count < (sizeof(UserSet)))
  {
    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, Address, *p) == HAL_OK)
    {
      Address = Address + 8;  /* increment to next double word*/
			p++;
			count += 8;
    }
   else
    {
			errorSetNo = HAL_FLASH_GetError();
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



















