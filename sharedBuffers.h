/* ----------------------------------------------------------------------------
 * Copyright (c) 2017 Semiconductor Components Industries, LLC (d/b/a
 * ON Semiconductor), All Rights Reserved
 *
 * This code is the property of ON Semiconductor and may not be redistributed
 * in any form without prior written permission from ON Semiconductor.
 * The terms of use and warranty for this code are covered by contractual
 * agreements between ON Semiconductor and the licensee.
 *
 * This is Reusable Code.
 *
 * ------------------------------------------------------------------------- */

#ifndef SHAREDBUFFERS_H_
#define SHAREDBUFFERS_H_

/* ----------------------------------------------------------------------------
 * If building with a C++ compiler, make all of the definitions in this header
 * have a C binding.
 * ------------------------------------------------------------------------- */
#ifdef __cplusplus
extern "C"
{
#endif /* ifdef __cplusplus */

#define CODEC_CONFIGURATION_SIZE    (0x100)
#define CODEC_SCRATCH_MEMORY_SIZE   (0x100)
#define CODEC_INPUT_SIZE            (0x300)
#define CODEC_OUTPUT_SIZE           (0x300)
#define COEF_SIZE					(0x800)
#define FFT_BIN_SIZE			    (0x200)


//要同时修改sections.ld

typedef struct _sharedMemory
{
#if 0
	//0x800 =2k 个 word SMDATA
	unsigned char smdata[0x800];
    unsigned char configuration[CODEC_CONFIGURATION_SIZE];
    unsigned char scratch[CODEC_SCRATCH_MEMORY_SIZE];
    unsigned char input[2][CODEC_INPUT_SIZE];
    unsigned char output[CODEC_OUTPUT_SIZE];
#else
	int smdata[0x100];		//Bits :0x800->0x400
    unsigned char configuration[CODEC_CONFIGURATION_SIZE];
    unsigned char scratch[CODEC_SCRATCH_MEMORY_SIZE];
    int input[2][CODEC_INPUT_SIZE/4];
    int output[CODEC_OUTPUT_SIZE/4];
#endif
} *pSharedMemory;

extern struct _sharedMemory Buffer;



typedef enum
{
	HPF = 0,
	LPF = 1,
	NC = 2,
	EQ = 3,
	WDRC = 4,
	UPLOAD = 5,
	AUDIO_DUMP = 6,
	AFC = 7,
	TONE_GEN = 12,
	NOISE_GEN = 13,
	UPDATE_CFG = 15,
}Control_Bit;

#define Q10 0x3ff
#define Q11 0x7ff
#define Q13	0x1fff
#define Q15	0x7fff
#define Q21 0x1fffff
#define Q23 0x7fffff
#define Q24 0xffffff
#define Q25 0x1ffffff
#define Q26 0x3ffffff
#define Q28 0xfffffff
#define Q29 0x1fffffff
#define Q30 0x3fffffff
#define Q31 0x7fffffff


typedef signed char Word8;
typedef	short	Word16;
typedef	short	INT16;
typedef int 	Word32;
typedef	int		INT32;

typedef	unsigned short 	UWord16;
typedef	unsigned short 	UINT16;
typedef unsigned int	UWord32;
typedef unsigned int	UINT32;

#define MAX_32 ((Word32)0x7fffffffL)
#define MIN_32 ((Word32)0x80000000L)

#define MAX_16 (Word16)0x7fff
#define MIN_16 (Word16)0x8000

#define MAX(a,b)		((a>b) ? (a):(b))
#define MIN(a,b)		((a<b) ? (a):(b))
#define MASK32(bit)		((UWord32)(1<<bit))
#define MASK16(bit)		((UWord16)(1<<bit))

#define MaxBandNum 4

typedef struct
{
	Word16 BandNum;
	Word32 Fix_Gain[8];
}SM_CONFIG_EQ;

typedef struct
{
	Word16 BandNum;
	Word32 Filter_Coef[3*2*8];
	int alfa[8];
	int beta[8];
	int maxdB;
    int exp_end_knee[8];
    int tkgain[8];
    int cr[8];
    int bolt[8];
	int tk_tmp[8];
    int cr_const[8];
    int tkgo[8];
    int pblt[8];
    int gain_at_exp_end_knee[8];
    int exp_cr_const[8];
}SM_CONFIG_WDRC;


typedef struct
{
	Word32 sm_max[MaxBandNum];
	Word32 x_max[MaxBandNum];
	Word32 gain[MaxBandNum];
}WDRC_T;

/********************************* Share Memory **************************************/
typedef struct {
	INT32	Buffer[10]; 	// VAD\SNR 
	INT32   FreqPower[8];
} SM_UPLOAD_DATA;		

typedef struct {
	INT32   nc_common_param[16];
	INT32   nc_personal_param[3];
	INT32   noiselevel;
} SM_CONFIG_NC;	

typedef struct {
	INT32   point;		
	INT32   drcgaininv;
	INT32   drcpeakline;
	INT32   drc_Table[65];
} SM_CONFIG_AGC;	

typedef struct {
	INT32 ADC_GAIN;
	INT32 DAC_GAIN;
} SM_CONFIG_MULTI_GAIN;	

typedef struct {
	INT32 NS_LEVEL;
	INT32 ATTACK;
	INT32 RELEASE;
} SM_CONFIG_AI_NS;	

typedef struct {
	INT32 HP[6];
	INT32 LP[6];
} SM_CONFIG_FILTER;	

typedef struct {
	short OnOff;
    short Mix;
	short Gain;
    short FirstToneA1Coef_WB;
    short SecondToneA1Coef_WB;
    short FirstToneB0Coef_WB;
    short SecondToneB0Coef_WB;
} SM_CONFIG_DTMF;	

typedef struct {
	int Volume;
} SM_CONFIG_VOLUME;	

typedef struct {
	SM_UPLOAD_DATA  UPLOAD;
	SM_CONFIG_WDRC  WDRC_ShareMem;
	SM_CONFIG_EQ 	EQ_ShareMem;
	SM_CONFIG_FILTER FILTER_ShareMem;
	SM_CONFIG_AI_NS AI_NS_ShareMem;
	SM_CONFIG_MULTI_GAIN 	MULTI_GAIN_ShareMem;
	SM_CONFIG_DTMF 	DTMF_ShareMem;
	SM_CONFIG_VOLUME VOLUME_ShareMem;
	UINT16  Control;
	//bit 0: HPF On/Off
	//bit 1: NC On/Off
	//bit 2: EQ On/Off
	//bit 3: WDRC On/Off
	//bit 4: FreqDomain Upload On/Off
	//bit 5: Config has change
} ShareMemoryData;	

extern ShareMemoryData* SM_Ptr;

/********************************* MCU ONLY **************************************/
//MCU Float Value
typedef struct {
	float maxdB;			// MPO default 110
	float exp_cr[8];			// Expansion: Ratio (exp_cr)
	float exp_end_knee[8];		// Expansion: End Kneepoint (exp_end_knee)
	float tkgain[8];			// Linear: Gain (tkgain)
	float tk[8];				// Compression Start Kneepoint (tk)
	float cr[8];				// Compression: Ratio (cr)
	float bolt[8];				// Limiter: Start Kneepoint (bolt)
	float Attack_time[8];			// 200ms 
	float Release_time[8];			// 200ms
	short BandNum;				// default = 8
	float CrossOverFreq[7];		// CrossOver Frequency
} MCU_Config_WDRC;		

typedef struct {
	short BandNum;
	float dB_Gain_float[8];			
}MCU_Config_EQ;

typedef struct {
	INT32   nc_common_param[16];
	INT32   nc_personal_param[3];
	INT32   noiselevel;
} MCU_Config_NC;	

typedef struct {
	float dB_gain;
	float dB_peak;
} MCU_Config_AGC;	

typedef struct {
	float ADC_GAIN;
	float DAC_GAIN;
} MCU_Config_MULTI_GAIN;	


typedef struct {
	float NS_LEVEL;
	float ATTACK;
	float RELEASE;
} MCU_Config_AI_NS;	

typedef struct {
	double HP[6];
	double LP[6];
} MCU_Config_FILTER;	


typedef struct {
	short OnOff;
    short Mix;
	short Gain;
    float FreqLow;
	float FreqHigh;
} MCU_Config_DTMF;	

typedef struct {
	float Volume;		// 0dB  ~ -N dB
} MCU_Config_VOLUME;	

extern ShareMemoryData SM_Data;
extern MCU_Config_WDRC MCU_WDRC;
extern MCU_Config_EQ MCU_EQ ;
extern MCU_Config_NC MCU_NC ;
extern MCU_Config_AI_NS MCU_AI_NS ;
extern MCU_Config_FILTER MCU_FILTER ;
extern MCU_Config_MULTI_GAIN MCU_MULTI_GAIN;
extern MCU_Config_DTMF MCU_DTMF;
extern MCU_Config_VOLUME MCU_VOLUME;

void EQ_Parser(MCU_Config_EQ* Cfg,SM_CONFIG_EQ* EQ_Cfg);
void WDRC_Parser(MCU_Config_WDRC* Cfg,SM_CONFIG_WDRC* WDRC_Cfg);
void NC_Config_Init(MCU_Config_NC* Cfg,SM_CONFIG_NC* NC_Cfg);
void MCU_Config_Parser();
void Multi_Gain_Init(MCU_Config_MULTI_GAIN* Cfg,SM_CONFIG_MULTI_GAIN* Multi_Gain_Cfg);
void AI_NS_Init(MCU_Config_AI_NS* Cfg,SM_CONFIG_AI_NS* AI_NS_Cfg);
void Filter_Init(MCU_Config_FILTER* Cfg,SM_CONFIG_FILTER* FILTER_Cfg);
void DTMF_Init(MCU_Config_DTMF* Cfg,SM_CONFIG_DTMF* DTMF_Cfg);
void Volume_Init(MCU_Config_VOLUME* Cfg,SM_CONFIG_VOLUME* Vol_Cfg);

/* ----------------------------------------------------------------------------
 * Close the 'extern "C"' block
 * ------------------------------------------------------------------------- */
#ifdef __cplusplus
}
#endif /* ifdef __cplusplus */

#endif    /* SHAREDBUFFERS_H_ */
