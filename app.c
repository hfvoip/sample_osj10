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
 * ----------------------------------------------------------------------------
 * app.c
 * - Receive audio from DMIC0 and DMIC1 in 18-bit mode using ARM Cortex-M3
 *   processor.
 * - User can switch among DMIC0, DMIC1, DMIC0+1 (mixed of DMIC channels) and
 *   DMIC0-1 (with a delay applied to DMIC1 to do beam forming) streams on OD
 *   by pressing on the push button.
 * - User can change the gain of DMIC channels and OD in the code.
 * ----------------------------------------------------------------------------
 * $Revision: 1.8 $
 * $Date: 2018/03/02 17:05:01 $
 * ------------------------------------------------------------------------- */




#include <app.h>
#include <printf.h>
#include "playtone.h"
#include "sharedBuffers.h"


#include <rsl10_flash_rom.h>



uint8_t* arr_flags   ;

//loop 模式:不loop, DSP处理后LOOP, DSP_LOOP:DSP执行中LOOP,DMIC_LOOP: DMIC中断中LOOP(不调用DSP)
enum LOOP_MODE
{
    NO_LOOP=0,DMIC_LOOP,DSP_LOOP,AFTERDSP_LOOP
};

/* ----------------------------------------------------------------------------
 * Global variables declaration
 * --------------------------------------------------------------------------*/



#define DMIC_CLK_DIO     15
#define  DMIC_DATA_DIO      10
#define OD_P_DIO          2
#define  OD_N_DIO          3



unsigned char  app_resetcode  __attribute__ ((section (".noinit")))  ;




/* ----------------------------------------------------------------------------
 * Function      : void DIO0_IRQHandler(void)
 * ----------------------------------------------------------------------------
 * Description   : Modify the od_mode
 * Inputs        : None
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */

void DIO0_IRQHandler(void)
{
	uint8_t  targetval = 0;

    static uint8_t ignore_next_dio_int = 1;
    if (ignore_next_dio_int == 1)
    {
        ignore_next_dio_int = 0;
    }
    else if (DIO_DATA->ALIAS[BUTTON_DIO] == targetval)
    {
        /* Button is pressed: Ignore next interrupt.
         * This is required to deal with the debounce circuit limitations. */
        ignore_next_dio_int = 1;
        //Button_DIO 按下去的处理
        Button_inc_volume_btn1();


    }


}
extern ShareMemoryData* SM_Ptr;


void Debug_LED(int led_dio,int cnt) {
	for (uint8_t i=0;i<cnt;i++) {

	    Sys_GPIO_Set_High(led_dio);

	    /* Delay for LED_ON_DURATION seconds
	     * The number of cycles is calculated as follows:
	     * LED_ON_DURATION [s] * SystemCoreClock [cycle/s] */
	    Sys_Delay_ProgramROM(0.5 * SystemCoreClock);
	    Sys_Watchdog_Refresh();

	    Sys_GPIO_Set_Low(led_dio);
	    Sys_Delay_ProgramROM(0.5 * SystemCoreClock);
	       Sys_Watchdog_Refresh();
	}
}
/* ----------------------------------------------------------------------------
 * Function      : void Initialize(void)
 * ----------------------------------------------------------------------------
 * Description   : Initialize the system by disabling interrupts, switching to
 *                 the 8 MHz clock (divided from the 48 MHz crystal) and
 *                 configuring the DIOs required for the DMIC and OD.
 *                 Enabling interrupts
 * Inputs        : None
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
void OSJ10_Initialize(void)
{
    /* Mask all interrupts */
    __set_PRIMASK(PRIMASK_DISABLE_INTERRUPTS);

    /* Disable all existing interrupts, clearing all pending source */
    Sys_NVIC_DisableAllInt();
    Sys_NVIC_ClearAllPendingInt();


    /* Test DIO12 to pause the program to make it easy to re-flash */
    DIO->CFG[RECOVERY_DIO] = DIO_MODE_INPUT  | DIO_WEAK_PULL_UP |
                             DIO_LPF_DISABLE | DIO_6X_DRIVE;
    while (DIO_DATA->ALIAS[RECOVERY_DIO] == 0);


	   /* Start 48 MHz XTAL oscillator */
	   /* Enable VDDRF supply without changing trimming settings */
	   ACS_VDDRF_CTRL->ENABLE_ALIAS = VDDRF_ENABLE_BITBAND;
	   ACS_VDDRF_CTRL->CLAMP_ALIAS = VDDRF_DISABLE_HIZ_BITBAND;

	   /* Wait until VDDRF supply has powered up */
	   while (ACS_VDDRF_CTRL->READY_ALIAS != VDDRF_READY_BITBAND);

	   /* Enable RF power switches */
	   SYSCTRL_RF_POWER_CFG->RF_POWER_ALIAS = RF_POWER_ENABLE_BITBAND;

	   /* Remove RF isolation */
	   SYSCTRL_RF_ACCESS_CFG->RF_ACCESS_ALIAS = RF_ACCESS_ENABLE_BITBAND;

	   /* Start the 48 MHz oscillator without changing the other register
		* bits */
	   RF->XTAL_CTRL = ((RF->XTAL_CTRL & ~XTAL_CTRL_DISABLE_OSCILLATOR) |
						XTAL_CTRL_REG_VALUE_SEL_INTERNAL);

	   /* Enable 48 MHz oscillator divider at desired prescale value  48/2=24 */
	       RF_REG2F->CK_DIV_1_6_CK_DIV_1_6_BYTE = CK_DIV_1_6_PRESCALE_1_BYTE;

	       /* Wait until 48 MHz oscillator is started */
	       while (RF_REG39->ANALOG_INFO_CLK_DIG_READY_ALIAS !=
	              ANALOG_INFO_CLK_DIG_READY_BITBAND);

	       /* Switch to 48 MHz oscillator clock */
	       Sys_Clocks_SystemClkConfig(JTCK_PRESCALE_1   |
	                                  EXTCLK_PRESCALE_1 |
	                                  SYSCLK_CLKSRC_RFCLK);


    /* Configure clock dividers. */
      CLK->DIV_CFG0 = (SLOWCLK_PRESCALE_48 |
                       BBCLK_PRESCALE_6   |
                       USRCLK_PRESCALE_1);

      CLK->DIV_CFG2 = (CPCLK_PRESCALE_8 | DCCLK_PRESCALE_12);


      /* Wake-up and apply clock to the BLE base-band interface. */

      BBIF->CTRL    = (BB_CLK_ENABLE | BBCLK_DIVIDER_8 | BB_WAKEUP);



		  BLE_Initialize();
		  App_Env_Initialize();

		  //in below function ,we intiialize DMIC,OD,DMA,ENABLE INTERTUPTS


	       J10_Initialize_DMICOD_DMA_INT(DMIC_CLK_DIO,DMIC_DATA_DIO,OD_P_DIO,OD_N_DIO);
	  /* Setup DIO5 as a GPIO input with interrupts on transitions, DIO6 as a
	   * GPIO output. Use the integrated debounce circuit to ensure that only a
	   * single interrupt event occurs for each push of the pushbutton.
	   * The debounce circuit always has to be used in combination with the
	   * transition mode to deal with the debounce circuit limitations.
	   * A debounce filter time of 50 ms is used. */

	  Sys_DIO_Config(BUTTON_DIO, DIO_MODE_GPIO_IN_0 | DIO_WEAK_PULL_UP |
	                 DIO_LPF_DISABLE);



	  Sys_DIO_IntConfig(0, DIO_EVENT_TRANSITION | DIO_SRC(BUTTON_DIO) |
	                    DIO_DEBOUNCE_ENABLE,
	                    DIO_DEBOUNCE_SLOWCLK_DIV1024, 49);

	  //IN OUR DEMO PCBA, DIO7 is to power on the DMIC,it has to be high
	  Sys_DIO_Config(7,DIO_MODE_GPIO_OUT_1);


	  /* Enable interrupts */
	  NVIC_EnableIRQ(DIO0_IRQn);
	//   NVIC_EnableIRQ(DMIC_OUT_OD_IN_IRQn);



	  /* Configure DIO used for DMIC
	   * Disable JTAG data on DIO14 and DIO15 As they are used defined as
	   * DMIC_DATA_DIO and DMIC_CLK_DIO */
	  DIO_JTAG_SW_PAD_CFG->CM3_JTAG_DATA_EN_ALIAS = 0;
	  DIO_JTAG_SW_PAD_CFG->CM3_JTAG_TRST_EN_ALIAS = 0;


      __set_PRIMASK(PRIMASK_ENABLE_INTERRUPTS);
      __set_FAULTMASK(FAULTMASK_ENABLE_INTERRUPTS);





}





void Button_inc_volume_btn1() {


	float max_vol = MCU_VOLUME.Volume;
	float volume_step =3;


	bool bcontinue = (max_vol<= (0 -  volume_step));
	//音量到最大，长度一声 ,一共8档
	if (!bcontinue) {
				MCU_VOLUME.Volume = 0 -   volume_step *7;

				//调到最小音量
				Start_Playtone(1000,2,6,1);




	}else {
		 MCU_VOLUME.Volume +=  volume_step;
		//在这里看是否到了最大音量，如果到了，就播最大音量
		//再次检查下是否到了最大
		max_vol  =  MCU_VOLUME.Volume;
		//下一步已经不能增加，这就是最大
		if (max_vol > (0-app_env.volume_step) )
			Start_Playtone(1000,1,6,3);
		else
			Start_Playtone(400,1,6,1);
	}


	Fill_SmData_Buffer();

	SM_Ptr->Control |= MASK16(UPDATE_CFG);

	uint8_t notifydata[3] = {0};
	notifydata[0] = cs_env[0].arr_params[0];
	notifydata[1] =  (uint8_t) (0 -MCU_VOLUME.Volume);
	notifydata[2] = app_env.batt_lvl;
	if (cs_env[0].sent_success)
		CustomService_SendNotification(ble_env[0].conidx,CS_IDX_TX_VALUE_VAL,notifydata,3);



}


void Button_inc_memory() {

}

void Burn_Parameters() {
	// We need read/write flash data,but for this sample application ,we don't do it
}
void  ADC_BUTTON_Handler() {

	int ad_val  = app_env.batt_lvl;
	if (ad_val >15000) {
		 Sys_Delay_ProgramROM(0.2 * SystemCoreClock);
		 return ;
	}else
	if  (ad_val >10000) {
		// V -
	} else
	if  (ad_val >7500) {
		// V +
	} else
	if (ad_val >5000) {
		// MODE A
	} else
	if (ad_val >5000) {
			// MODE A
	} else
	if (ad_val >5000) {
			// MODE B
	} else
	if (ad_val >5000) {
			// MODE C
	}  else
	if (ad_val >5000) {
			// MODE D
	}
    //We need
	 //Process_buttonevt();
	//处理完，等待用户松手，不然再次执行到了这里
	 Sys_Delay_ProgramROM(0.2 * SystemCoreClock);

	 Burn_Parameters();

}
/* ----------------------------------------------------------------------------
 * Function      : int main(void)
 * ----------------------------------------------------------------------------
 * Description   : Initialize the system, the DMIC and the OD. Blinks the LED
 *                 depending on the current mode.
 * Inputs        : None
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */

int main(void)
{



	//MCU_Config_Parser();
	Fill_SmData_Buffer();

    /* Initialize the system */
    OSJ10_Initialize();



	 printf_init();



   Sys_Delay_ProgramROM(1.5 * SystemCoreClock);
   Start_Playtone(1000,2,-9,3);







    /* Spin loop */
    while (1)
    {


       Kernel_Schedule();

       if ( cs_env[0].rx_value_changed ) {

          	   //save to flash

          	   if (app_env.playtone_freq == 0 && app_env.playtone_gain == 0) {
          		   Stop_Playtone();
          	   } else {
          		   Start_Playtone(app_env.playtone_freq,100,app_env.playtone_gain,1);

          	   }
      			 Fill_SmData_Buffer();

				 if (cs_env[0].wdrc_mask  & 0x10)  SM_Ptr->Control |= MASK16(WDRC);
				 else
					 SM_Ptr->Control &= ~MASK16(WDRC);

				 if (cs_env[0].wdrc_mask  & 0x8)  SM_Ptr->Control |= MASK16(EQ);
				 else
					 SM_Ptr->Control &= ~MASK16(EQ);

				 if (cs_env[0].wdrc_mask  & 0x4)  SM_Ptr->Control |= MASK16(AFC);
				 else
					 SM_Ptr->Control &= ~MASK16(AFC);



      			 SM_Ptr->Control |= MASK16(UPDATE_CFG);
      		     cs_env[0].rx_value_changed = false;

         }
     //  ADC_BUTTON_Handler();

    	SYS_WAIT_FOR_INTERRUPT;
		/* Refresh the watchdog timer */
        Sys_Watchdog_Refresh();

       
    }
}



