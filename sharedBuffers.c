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
#if  0
#include "sharedBuffers.h"
#include <math.h>

/* We define the shared memory in one place and it's location is controlled
 * by the linker file. This buffer is located in DRAM which is shared between
 * the LPDSP32 and the ARM
 */
//struct _sharedMemory Buffer;
struct _sharedMemory Buffer __attribute__ ((section (".shared")));

ShareMemoryData SM_Data  ;

ShareMemoryData* SM_Ptr = &Buffer;

MCU_Config_WDRC MCU_WDRC=
{
	120,	   //Full_Scale
	{0.4, 0.3, 0.3, 0.3, 0.3, 0.3, 0.3, 0.3},				  // compression ratio for low-SPL region (ie, the expander..values should be < 1.0)
	{55.0, 55.0, 55.0, 55.0, 55.0, 55.0, 55.0, 55.0}, 				// expansion-end kneepoint
	{35.f, 35.f, 35.f, 35.f, 35.f, 35.f, 35.f, 35.f}, 				// compression-start gain
	{95.0, 95.0, 95.0, 95.0, 95.0, 95.0, 95.0, 95.0}, 				// compression-start kneepoint (input dB SPL)
	{1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f},			   // compression ratio
	{110.f, 110.f, 110.f, 110.f, 110.f, 110.f, 110.f, 110.f},					// output limiting threshold (comp ratio 10)
 {200,200,200,200,200,200,200,200},
 {200,200,200,200,200,200,200,200},
 8,    //BandNum
	{250 ,500, 1000, 2000 ,3000, 4500, 6000 },   
};



MCU_Config_EQ MCU_EQ=
{
	8,		//BandNum
	{	
		3,	3,	-0,	-0,
		0, -0,  -0,  -0,
	},		//Gain dB
};


MCU_Config_AGC MCU_AGC =
{
	6.0,
	-0.9
};

MCU_Config_MULTI_GAIN MCU_MULTI_GAIN = 
{
	0,
	0,
};
	
MCU_Config_AI_NS MCU_AI_NS = 
{
	1,
	1,
	1
};


MCU_Config_DTMF MCU_DTMF = 
{
	0,			// On/Off
	0,			// 	Mix
	6,			// Gain 
	3000,		// Freq
	4000,
};


MCU_Config_FILTER MCU_FILTER = 
{
	// HP CUTOFF = 250Hz Ellip Order = 2 Astop = 80 Apass = 1
	{0.847924150315670	,-1.69584746762183,	0.847924150315670,
	1,	-1.89860757413309,	0.906937669224840},
	
	// LP
	//{0.284015935923063,	0.567896591321167,	0.284015935923063,
	//1,	-0.0391362755049835 ,0.313668973945555},	//4kHz
	//{0.569035593728849,1.13807118745770,0.569035593728849,
	//1,0.942809041582064,0.333333333333333}		//6kHz
	//In Use
	// Low Shelf Filter
	{1.05452993961425,	-1.72997286674978,	0.714227567688170,
	1	,-1.73964600367320	,0.759084370379006},
	
	//Notch Mid 3800  BandWdith 3000
	//{0.746268217390591,	-0.117103059013793,	0.746268217390591,
	//	1,	-0.117103059013793,	0.492536434781182},
};
MCU_Config_VOLUME MCU_VOLUME = 
{
	0,			// 0dB
};


void setAttackRelease_msec( float* atk_msec, float* rel_msec, SM_CONFIG_WDRC* WDRC_Cfg) {
	int i;
	for(i=0;i<8;i++)
	{
		float ansi_atk = 0.001f * atk_msec[i] * 16000/ 2.425f; 
		float Float_alfa;
		Float_alfa = (float) (ansi_atk / (1.0f + ansi_atk));

	    float ansi_rel = 0.001f * rel_msec[i] * 16000/ 1.782f; 
		float Float_beta;
	    Float_beta = (float) (ansi_rel / (1.0f + ansi_rel));

		int Fix_alfa,Fix_beta;
		Fix_alfa = (int)(round(Float_alfa*Q31));
		Fix_beta = (int)(round(Float_beta*Q31));
		WDRC_Cfg->alfa[i] = Fix_alfa;
		WDRC_Cfg->beta[i] = Fix_beta;
	}
}

void WDRC_Parser(MCU_Config_WDRC* Cfg,SM_CONFIG_WDRC* WDRC_Cfg)
{
	//exp_cr = compression ratio for expansion
	//exp_end_knee = kneepoint for end of the expansion region
	//tkgn = gain (dB?) at start of compression (ie, gain for linear behavior?)
	//tk = compression start kneepoint (pre-compression, dB SPL?)
	//cr = compression ratio
	//bolt = broadband output limiting threshold (post-compression, dB SPL?)

	int i;
	float gdb, tkgo, pblt;
	int k;
	 //float *pdb = env_dB; //just rename it to keep the code below unchanged (input SPL dB)

	float maxdB = Cfg->maxdB;
	WDRC_Cfg->BandNum = Cfg->BandNum;
	setAttackRelease_msec(Cfg->Attack_time,Cfg->Release_time,WDRC_Cfg);
 	for(i = 0; i<8 ; i++)
 	{
		float exp_end_knee[8];
    	float tkgain[8];
    	float cr[8];
    	float bolt[8];
		float tk_tmp[8];
    	float cr_const[8];
    	float tkgo[8];
    	float pblt[8];
    	float gain_at_exp_end_knee[8];
    	float exp_cr_const[8];
    	float state_ppk[8];
		float exp_cr[8];
		exp_cr[i] = Cfg->exp_cr[i];
		tkgain[i] = Cfg->tkgain[i];
		bolt[i] = Cfg->bolt[i];
		cr[i] = Cfg->cr[i];
		exp_end_knee[i] = Cfg->exp_end_knee[i];
		tk_tmp[i] = Cfg->tk[i];   //temporary, threshold for start of compression (input SPL dB)	
		  if ((tk_tmp[i] + tkgain[i]) > bolt[i]) { //after gain, would the compression threshold be above the output-limitting threshold ("bolt")
			  tk_tmp[i] = bolt[i] - tkgain[i];  //if so, lower the compression threshold to be the pre-gain value resulting in "bolt"
		  }
	    cr_const[i] = ((1.0f / cr[i]) - 1.0f); //pre-calc a constant that we'll need later
	  	tkgo[i] = tkgain[i] + tk_tmp[i] * (-cr_const[i]);  //intermediate calc
	    pblt[i] = cr[i] * (bolt[i] - tkgo[i]); //calc input level (dB) where we need to start limiting, not just compression
	  //compute gain at transition between expansion and linear/compression regions
	  gain_at_exp_end_knee[i] = tkgain[i];
	  if (tk_tmp[i] < exp_end_knee[i]) {
		gain_at_exp_end_knee[i]  = cr_const[i] * exp_end_knee[i] + tkgo[i];
	  }
	  exp_cr_const[i] = 1.0f/MAX(0.01f,exp_cr[i]) - 1.0f;

	  WDRC_Cfg->maxdB = (int)(round(Cfg->maxdB*Q23));
	  WDRC_Cfg->exp_end_knee[i] = (int)(round(exp_end_knee[i]*Q23));
	  WDRC_Cfg->tkgain[i] = (int)(round(tkgain[i]*Q23));
	  WDRC_Cfg->cr[i] = (int)(round(cr[i]*Q23));
	  WDRC_Cfg->bolt[i] = (int)(round(bolt[i]*Q23));
	  WDRC_Cfg->tk_tmp[i] = (int)(round(tk_tmp[i]*Q23));
	  WDRC_Cfg->cr_const[i] = (int)(round(cr_const[i]*Q23));
	  WDRC_Cfg->tkgo[i] = (int)(round(tkgo[i]*Q23));
	  WDRC_Cfg->pblt[i] = (int)(round(pblt[i]*Q23));
	  WDRC_Cfg->gain_at_exp_end_knee[i] = (int)(round(gain_at_exp_end_knee[i]*Q23));
	  WDRC_Cfg->exp_cr_const[i] = (int)(round(exp_cr_const[i]*Q23));
#ifdef DEBUG_MODE
	  printf("maxdB = %f , exp_end_knee = %f\n",((float)WDRC_Cfg->maxdB)/Q23,((float)WDRC_Cfg->exp_end_knee[i])/Q21);
	  printf("tkgain = %f , cr = %f\n",((float)WDRC_Cfg->tkgain[i])/Q23,((float)WDRC_Cfg->cr[i])/Q21);
	  printf("bolt = %f , tk_tmp = %f\n",((float)WDRC_Cfg->bolt[i])/Q23,((float)WDRC_Cfg->tk_tmp[i])/Q21);
	  printf("cr_const = %f , tkgo = %f\n",((float)WDRC_Cfg->cr_const[i])/Q23,((float)WDRC_Cfg->tkgo[i])/Q21);
	  printf("pblt = %f , gain_at_exp_end_knee = %f\n",((float)WDRC_Cfg->pblt[i])/Q23,((float)WDRC_Cfg->gain_at_exp_end_knee[i])/Q21);
	  printf("exp_cr_const = %f \n",((float)WDRC_Cfg->exp_cr_const[i])/Q23);
#endif
 	}
	
}




void EQ_Parser(MCU_Config_EQ* Cfg,SM_CONFIG_EQ* EQ_Cfg)
{
	float EQ_Gain;
	int T32;
	int   i;
	EQ_Cfg->BandNum = Cfg->BandNum;
	for(i = 0;i<Cfg->BandNum;i++)
	{
		EQ_Gain = pow(10,(Cfg->dB_Gain_float[i])/20);
		EQ_Gain = EQ_Gain*Q31;
		T32  =  (int)EQ_Gain;
		EQ_Cfg->Fix_Gain[i] = T32;
#ifdef DEBUG_MODE
		printf("EQ = %f \n",((float)EQ_Cfg->Fix_Gain[i])/Q31);
#endif
	}
	
}

void NC_Config_Init(MCU_Config_NC* Cfg,SM_CONFIG_NC* NC_Cfg)
{
	int i ;
	for(i= 0;i<16;i++)
		NC_Cfg->nc_common_param[i] = Cfg->nc_common_param[i];
	float Gain = Cfg->nc_common_param[15];			//ADC Gain before NC
	Gain = pow(10,Gain/20);
	NC_Cfg->nc_common_param[15] = (int)(Gain*Q21);	//Gain
	NC_Cfg->nc_common_param[14] = (int)((float)Cfg->nc_common_param[14]*Gain/2);	//Noise Level
	for(i = 0;i<3;i++)
		NC_Cfg->nc_personal_param[i] = Cfg->nc_personal_param[i];
	NC_Cfg->noiselevel = Cfg->noiselevel;
//	printf("nc_common_param[14] = %x \n",NC_Cfg->nc_common_param[14]);
//	printf("nc_common_param[15] = %x \n",NC_Cfg->nc_common_param[15]);
}


void AGC_Config_Init(MCU_Config_AGC* Cfg,SM_CONFIG_AGC* AGC_Cfg)
{
	float normal = Cfg->dB_gain;
	float peakdB = Cfg->dB_peak;
	AGC_Cfg->point = (int)(pow(10,(-(normal-peakdB)*1.1)/20)*Q31);
	AGC_Cfg->drcgaininv =(int)(pow(10,(-(normal-peakdB))/20)*Q31);
	AGC_Cfg->drcpeakline = (int)(pow(10,(peakdB)/20)*Q31);

	float drcgaindb = normal-peakdB;
	float RangeFactor = 0.1;
	float drcgaininv = pow(10,(-drcgaindb/20));
	float drcpeakline = pow(10,(peakdB)/20);
	float point = pow(10,(-(1+RangeFactor)*drcgaindb)/20);
	int i;
	float k;
	float loc;
	float btn;
#if 0
	printf("drcgaininv = %f \n",drcgaininv);
	printf("drcpeakline = %f \n",drcpeakline);
	printf("point = %f \n",point);
	printf("\n Drc Table\n");
#endif
	for(i=0;i<=64;i++)	//65 sample 0:1/64:1
	{
		loc = ((float)i)/64;
		k = (loc-point)/(1-point);
		k = MAX(0,k);
		btn = drcgaininv+(1-drcgaininv)*(1-drcgaininv)*k+(1-drcgaininv)*drcgaininv*k*k;
		AGC_Cfg->drc_Table[i] = (int)((drcpeakline/btn)*Q21);
#if 0		
		printf("%f ",((float)AGC_Cfg->drc_Table[i])/Q21);
		if((i%8)==0)
			printf("\n");
#endif
	}
	
}

void Multi_Gain_Init(MCU_Config_MULTI_GAIN* Cfg,SM_CONFIG_MULTI_GAIN* Multi_Gain_Cfg)
{
	Multi_Gain_Cfg->ADC_GAIN = (int)(pow(10,Cfg->ADC_GAIN/20)*Q21);
	Multi_Gain_Cfg->DAC_GAIN = (int)(pow(10,Cfg->DAC_GAIN/20)*Q21);
}


void AI_NS_Init(MCU_Config_AI_NS* Cfg,SM_CONFIG_AI_NS* AI_NS_Cfg)
{
	AI_NS_Cfg->NS_LEVEL = (int)(Cfg->NS_LEVEL*Q31);
	AI_NS_Cfg->ATTACK = (int)(Cfg->ATTACK*Q31);
	AI_NS_Cfg->RELEASE = (int)(Cfg->RELEASE*Q31);
}


void Filter_Init(MCU_Config_FILTER* Cfg,SM_CONFIG_FILTER* FILTER_Cfg)
{
	int i;
	for(i=0;i<3;i++)
	{
		FILTER_Cfg->HP[i] = (int)(round(Cfg->HP[i]*Q29));
		FILTER_Cfg->HP[3+i] = (int)(round(Cfg->HP[3+i]*Q30));
	}
	for(i=0;i<3;i++)
	{
		FILTER_Cfg->LP[i] = (int)(round(Cfg->LP[i]*Q29));
		FILTER_Cfg->LP[3+i] = (int)(round(Cfg->LP[3+i]*Q30));
	}
}

void DTMF_Init(MCU_Config_DTMF* Cfg,SM_CONFIG_DTMF* DTMF_Cfg)
{
	int i;
	DTMF_Cfg->OnOff = Cfg->OnOff;
	DTMF_Cfg->Mix = Cfg->Mix;
	DTMF_Cfg->Gain = Cfg->Gain;
	// 697	   0x7b3c		   0xeeb3
	//770		 0x7a31 		 0xecf1
	// Coefficient   =	  floor(2*cos(2*PI*freq/fs) * Q14 + 0.5)
	// Initial Value = -1 * floor(  sin(2*PI*freq/fs) * Q14 + 0.5)
	short C1   =	floor(2*cos(2*3.141592653*Cfg->FreqLow/16000) * 16384 + 0.5);
	short V1 = -1 * floor(sin(2*3.141592653*Cfg->FreqLow/16000) * 16384 + 0.5);
	short C2   =	floor(2*cos(2*3.141592653*Cfg->FreqHigh/16000) * 16384 + 0.5);
	short V2 = -1 * floor(sin(2*3.141592653*Cfg->FreqHigh/16000) * 16384 + 0.5);
	DTMF_Cfg->FirstToneA1Coef_WB  = C1;
	DTMF_Cfg->SecondToneA1Coef_WB = C2;
	DTMF_Cfg->FirstToneB0Coef_WB  = V1;
	DTMF_Cfg->SecondToneB0Coef_WB = V2;
}

void Volume_Init(MCU_Config_VOLUME* Cfg,SM_CONFIG_VOLUME* Vol_Cfg)
{
	int i;
	Vol_Cfg->Volume = (int)(round(powf(10,Cfg->Volume/20)*Q21));
}
/********************************* Only Test **************************************/
int Random_Vec[256]=
{
168608,-567479,-512857,-224132,421536,1337038,520034,-697168,-133536,-360889,-190678,768217,245648,645419,392789,-516092,605872,-878972,520585,390306,716273,-924114,-132547,11660,65319,570423,456084,424399,116206,1090920,-211607,65837,-375371,-40049,472287,-798881,124277,-478296,513162,-1279273,-1272857,1057258,-97258,-1281152,-1097512,-56578,200079,219709,694508,216713,-47831,681988,1340120,17375,-156558,675947,-330155,-965341,-69313,159643,94897,-727765,-388551,-442281,621648,-85138,422699,389575,-125299,-1289308,236238,458203,-489868,-11012,-287481,1107404,-19898,132731,-682023,-478655,-882991,474239,395220,-539134,1879577,598268,264129,1247250,-1598415,-59273,-16064,130139,-675842,344567,-1443130,69918,285688,-26683,237130,-497093,357687,-243853,-1079952,-201929,-403523,-666910,328705,26344,-165805,-356523,1091300,763511,-123061,56350,984530,1167246,717699,-212095,-379356,489210,-562207,-697036,-231225,622185,103677,159433,732103,152720,-741094,548121,310052,-57533,-268846,-354471,-15626,-521057,-333386,-967682,-337586,-451225,375995,947043,674000,681381,440148,355780,488170,-475054,542334,-780140,402762,-541820,-631698,-295192,-847936,10398,722932,-162318,322654,105471,788778,-103048,190696,1606243,746087,-607841,728876,-601582,6448,188350,566288,-258579,350093,1151927,-1240286,-700859,777791,-191332,-24850,601392,1446997,-847408,-591665,-50445,332,69553,-203384,793872,-592036,132769,644230,244219,-404944,78470,-206594,-390336,880772,-128662,-376223,-492063,78889,-973648,-1071490,125558,-839515,-260263,504712,600230,1005343,-657127,557633,211730,815216,143302,530614,741653,-267984,-300109,-865082,-56954,231361,-483711,46885,-231984,-280990,507267,531348,747435,-706204,-88602,426401,-841918,1254121,151824,-354757,672261,245479,71115,-442737,-1149099,-627189,-420244,294552,399979,-511155,539062,434424,-81689,-619158,580632,-84436,-659681,164214,340803,-951379,1214849};

#endif
