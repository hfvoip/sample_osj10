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
#include "app_bass.h"



/* ----------------------------------------------------------------------------
 * Global variables declaration
 * --------------------------------------------------------------------------*/



#define DMIC_CLK_DIO     15
#define  DMIC_DATA_DIO      10
#define OD_P_DIO          2
#define  OD_N_DIO          3



unsigned char  app_resetcode  __attribute__ ((section (".noinit")))  ;

void Button_EventCallback(uint32_t event) {

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

	  //IN OUR DEMO PCBA, DIO7 is to power on the DMIC,it has to be high
	//  Sys_DIO_Config(7,DIO_MODE_GPIO_OUT_1);

	  Sys_DIO_Config(LED_DIO,DIO_MODE_GPIO_OUT_0);


	  /* Configure DIO used for DMIC
	   * Disable JTAG data on DIO14 and DIO15 As they are used defined as
	   * DMIC_DATA_DIO and DMIC_CLK_DIO */
	  DIO_JTAG_SW_PAD_CFG->CM3_JTAG_DATA_EN_ALIAS = 0;
	  DIO_JTAG_SW_PAD_CFG->CM3_JTAG_TRST_EN_ALIAS = 0;


#if 0
	   /* Initialize gpio structure */
		    gpio = &Driver_GPIO;
		    /* Initialize gpio driver */
		  	gpio->Initialize(Button_EventCallback);

		    /* Initialize usart driver structure */
		    uart = &Driver_USART0;

		    /* Initialize usart, register callback function */
		    uart->Initialize(Usart_EventCallBack);
#endif

		    APP_BASS_SetBatMonAlarm(0);

		    Sys_DIO_Config(T3_BUTTON, INPUTGPIO_SETTING );
		     Sys_DIO_Config(LEFT_BLE_BUTTON,  INPUTGPIO_SETTING);
		     Sys_DIO_Config(RIGHT_BLE_BUTTON,  INPUTGPIO_SETTING);
		     Sys_DIO_Config(WIFI_AUDIO_BUTTON,  INPUTGPIO_SETTING);




      __set_PRIMASK(PRIMASK_ENABLE_INTERRUPTS);
      __set_FAULTMASK(FAULTMASK_ENABLE_INTERRUPTS);


}






void Burn_Parameters() {
	// We need read/write flash data,but for this sample application ,we don't do it
}
void  ADC_BUTTON_Handler() {

	int ad_val  = 0;

   //up: 16383
	//v- 15205
	//v+  13298
	//d 10598
	//c : 7337
	//b: 3650
	//A:501
	//PRINTF("\r\n ad val:%d",ad_val);
	ad_val = app_batt_read.batt_lvl_sum_mV ;
	ad_val = round(ad_val*0.01);

	if (ad_val >=160) {
		 Sys_Delay_ProgramROM(0.2 * SystemCoreClock);
		 return ;
	}else
	if  (ad_val >140) {
		// V-
		Dec_Volume();

	} else
	if  (ad_val >130) {
		// V +
		Inc_Volume();
	} else
	if (ad_val >100) {
		// D
		Change_Mode(3);
	} else
	if (ad_val >70) {
			// MODE C
		Change_Mode(2);
	} else
	if (ad_val >30) {
			// MODE B
		Change_Mode(1);
	} else
	if (ad_val >3) {
			// MODE A
		Change_Mode(0);
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
	/* in our sample PCBA ,we have 2 applications , HA or HA-BLECLIENT */
	// Let's check which application we should go ,by  WIFI_AUDIO_BUTTON
	   Sys_DIO_Config(WIFI_AUDIO_BUTTON,  INPUTGPIO_SETTING);
	   uint8_t btn_flag_4 = DIO_DATA->ALIAS[WIFI_AUDIO_BUTTON];
	   if (btn_flag_4 == 0) {
		   return main_bleclient();
	   }

	//MCU_Config_Parser();
	Fill_SmData_Buffer();

    /* Initialize the system */
    OSJ10_Initialize();




	 printf_init();
	 PRINTF("\r\n J10 START");




   Sys_Delay_ProgramROM(1.5 * SystemCoreClock);
   Start_Playtone(1000,2,-9,3);
#if 0
   ToggleLed(20, 500);
#endif
   PRINTF("\r\n if no sound ,please check license  ");

   //USE ADC DIO :DIO1 to monitor voltage
   APP_BASS_SetBatMonAlarm(0);
//   uart->Receive(rx_buffer, 3);


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
       ADC_BUTTON_Handler();
       Normal_BUTTON_Handler();

    	SYS_WAIT_FOR_INTERRUPT;
		/* Refresh the watchdog timer */
        Sys_Watchdog_Refresh();

       
    }
}


int main_bleclient() {
	//another application will start from addr :0x138000 ,pls confirm the sections.ld
	/*
	MEMORY
	{
	  ROM  (r) : ORIGIN = 0x00000000, LENGTH = 4K
	  FLASH (xrw) : ORIGIN = 0x00138000, LENGTH = 380K
	  PRAM (xrw) : ORIGIN = 0x00200000, LENGTH = 64K

	  DRAM (xrw) : ORIGIN = 0x20000000, LENGTH = 24K
	  DRAM_DSP (xrw) : ORIGIN = 0x20006000, LENGTH = 46K
	  DRAM_DSP_CM3 (xrw) : ORIGIN = 0x2100B800, LENGTH = 2K
	  DRAM_BB (xrw) : ORIGIN = 0x20012000, LENGTH = 16K
	}
	*/
	  uint32_t another_appaddress = 0x138000;
	   Sys_Delay_ProgramROM(1.0 * SystemCoreClock);

	   BootROMStatus status = Sys_BootROM_ValidateApp(another_appaddress);
	   if (status == BOOTROM_ERR_NONE || status == BOOTROM_ERR_BAD_CRC) {
		   Sys_BootROM_StartApp(another_appaddress);
	   }
	   return 0;
}
