#ifndef __APP_H__
#define __APP_H__
/**************************************************************************************************************/
#define 	SW_VER			1.00f
#define 	HW_VER			1.00f


/**************************************************************************************************************/

#define		CRYSTAL_NUM					24

#define 	CLEAN_TIME_MS				(200)			// ms

/**************************************************************************************************************/
#define 	ON    1
#define 	OFF   2


/**************************************************************************************************************/
				
#define 	PAGE_SIZE   (1024*2)
#define 	PARA_PAGES	(60)
#define 	PARA_ADDR   (PAGE_SIZE * PARA_PAGES+0x08000000UL)

#define 	MAGIC_DATA	(0x12345678)
/**************************************************************************************************************/
/**************************************************************************************************************/
/**************************************************************************************************************/
/**************************************************************************************************************/


typedef enum
{
	state_good,							//完全不断路
	state_short,						//阻值很低，<100R
	state_lowZ,						//中间阻值，
	state_highZ,						//阻值高，>1K
	state_short_after_clean, //清洗后继续短路
	state_highZ_after_clean, //清洗后高阻
	
	
}enumCrystalState;

// 晶体在不同时间的
typedef enum
{
	crystal_before_clean=0,
	crystal_after_clean,
	crystal_retry1,
	crystal_retry2,
	crystal_temp,
	max_crystal_point,
}enumCrystalPoint;


typedef struct
{
	u32 magicData;					//
	float ampOffset[24];					// AD value, 
	
}stUserPara;


typedef enum
{
	state_reset,
	state_idle,
	state_detect,		//检测晶体是否短路
	state_clean,
	state_detect_after_clean,
	state_wait,
	state_finished,
	state_calibration,
	state_detect_probe,
	
}enumTestState;

/**************************************************************************************************************/
extern enumTestState cleanState;
/**************************************************************************************************************/
/**************************************************************************************************************/

u8 checkStart(void);

void getCrystalState(enumCrystalPoint crystalPoint);
void delayms(u32 ms);
void charge(void);
void clean(void);
void initVar(void);
u16 getCleanTime(void);
void traceCrystalState(void);
void checkCrystal(void);
void hwDelayms(u32 ms);
void cleanMain(void);
u32 getTick(void);
void clearOverCurrentFlag(void);
void measureVoltage(u8 onoff);
void ADCMeasure(void);
void initADC(void)   ;
void readpara(void);
void writePara(void);
uint32_t HAL_GetTick(void);
float calibrationOne(u16 AD);
void calibration(void);
void updateCrystalRes(enumCrystalPoint state) ;
u16 calcCrystalRes(u16 AD,u8 ch);
void readMyAddr(void);
void sendAdcDataToPC(void);
void sendDataToPC(void);

/**************************************************************************************************************/

/**************************************************************************************************************/

/**************************************************************************************************************/

/**************************************************************************************************************/
#endif
