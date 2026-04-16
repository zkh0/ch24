#ifndef __APP_H__
#define __APP_H__
/**************************************************************************************************************/
#define 	SW_VER			1.00f
#define 	HW_VER			1.00f


/**************************************************************************************************************/

#define		CRYSTAL_NUM					24

#define 	CLEAN_TIME_MS				(200)			// ms

#define   PROBDECTIONMAXR     110
#define   PROBDECTIONMINR     90

/**************************************************************************************************************/
#define 	ON    1
#define 	OFF   2


/**************************************************************************************************************/
				
#define 	PAGE_SIZE   (1024*2)
#define 	PARA_PAGES	(60)
#define 	PARA_ADDR   (PAGE_SIZE * PARA_PAGES+0x08000000UL)

#define   SET_PAGES   (61)
#define   SET_ADDR    (PAGE_SIZE * SET_PAGES +0x08000000UL)


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
	crystal_first_clean = 1,
	crystal_second_clean,
	crystal_third_clean,
	crystal_fourth_clean,
	crystal_fifth_clean,
	crystal_sixth_clean,
	crystal_seventh_clean,
	crystal_before_clean,
	crystal_after_clean,
	crystal_temp,
	max_crystal_point,
}enumCrystalPoint;


typedef struct
{
	u32 magicData;					//
	float ampOffset[24];					// AD value, 
	
}stUserPara;
typedef struct 
{
	u8          FristAllClean  :1;                 //第一次是否全部激励 1   从0到1
	
	u8          CleanSuccessedNoExcitation :1;     //清洗成功后不激励   
	u8          CleanImpedanceNoUpdata :1;         //阻值不上传，只上传结果  为1上传，为0，不上传
	
	u8          UpdateAllR :1;	                    //上传所有阻值    
	u8          UpDateFirAndLastR :1;              //上传第一次和最后一次阻值
 	u8          UpdateAllS :1;                     //上传所有状态          为1上传，为0不上传          
	u8          UpDateFirAndLastS :1;              //上传第一次和最后一次状态  
	
	u8          UpDateBadNumber :1;                 //上传每次清洗后的不良件个数 
}UpStatePare;

typedef union
{
	u8 upstate;
	UpStatePare bits;
}UpStatePareReg;

typedef struct    //5个帧头 +1长度+2清洗时间+2重复清洗间隔+2测量前延时+2测量后延时+2最大电流+2最小成功阻值+2最大失败阻值+1清洗次数+1清洗成功后不激励+1阻抗不上传，只上传结果23+2校验 =25+6=29
{
	u32                Magic_data;                   //魔术字  
	u16                CleanTime;                    //清洗时间  
	u16                IntervalBetweenCleanTime;     //重复清洗间隔 
	u16                CleanBeforeDelayTime;         //激励前延时    
	u16                CleanAfterDelayTime;          //激励后延时 	
	u16                DelayBeforeMeasureTime;       //测量前延时    
	u16                DelayAfterMeasureTime;        //测量后延时    
	  
  u16	               maxCurrent;                   //最大电流        
	u16                MinOkR;                       //最小成功阻值  
	u16                MaxFaultR;                    //最大失败阻值   
	u16                FirstCleanTime;                //第一次清洗时间
	u8                 CleanTimes;                  	//清洗次数  
	UpStatePareReg     updatecontrol;                 // 上传状态控制
	
}UserSet __attribute__((aligned(8))); 


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
	state_error,
	state_first_clean,
}enumTestState;


/**************************************************************************************************************/
extern enumTestState cleanState;

extern UserSet       userset;
//volatile u8  CountTimes = 0;
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
void AllADCMeasure(void);
void initADC(void);

void OpenCh(void);

void readpara(void);
void readset(void);
void writePara(void);
void writeSet(void);

uint32_t HAL_GetTick(void);
float calibrationOne(u16 AD);
void calibration(void);

void updateCrystalRes(enumCrystalPoint state) ;
void updateStateCh(enumCrystalPoint state) ;

u16 calcCrystalRes(u16 AD,u8 ch);
void readMyAddr(void);
void sendFlagToPC(enumCrystalPoint state) ;
void sendDataToPC(enumCrystalPoint date);
void SendFlagORDate(enumCrystalPoint stateOrdate);

void hwDelayms(u32 ms);
void ProbDetection(void);
/**************************************************************************************************************/

/**************************************************************************************************************/

/**************************************************************************************************************/

/**************************************************************************************************************/
#endif
