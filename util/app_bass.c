/* ----------------------------------------------------------------------------
 * Copyright (c) 2018 Semiconductor Components Industries, LLC (d/b/a
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
 * app_bass.c
 * - BASS Application-specific source file
 * ------------------------------------------------------------------------- */

#include <app.h>
#include <app_bass.h>
#include <printf.h>

/* Struct containing values needed for battery level calculation */
 struct app_batt_read_t app_batt_read;

/* ----------------------------------------------------------------------------
 * Function      : void APP_BASS_SetBatMonAlarm(uint32_t supplyThresholdCfg)
 * ----------------------------------------------------------------------------
 * Description   : Configure ADC to read VBAT/2 and trigger a BATMON alarm
 *                 interrupt when adc level < supplyThresholdCfg. The interrupt
 *                 is handled by ADC_BATMON_IRQHandler.
 * Inputs        : supplyThresholdCfg  - BATMON supply threshold configuration
 * Outputs       : None
 * Assumptions   : Alarm count value is set to 100. ADC channels used are:
 *                 ADC_BATMON_CH and ADC_GND_CH.
 * ------------------------------------------------------------------------- */
void APP_BASS_SetBatMonAlarm(uint32_t supplyThresholdCfg)
{
#define  DIO_ADCBUTTON 1

	 Sys_DIO_Config(DIO_ADCBUTTON , DIO_MODE_DISABLE|DIO_NO_PULL);
	 Sys_ADC_Set_Config(ADC_VBAT_DIV2_NORMAL | ADC_NORMAL | ADC_PRESCALE_1280H);

	/* Set the battery monitor interrupt configuration */
	Sys_ADC_Set_BATMONIntConfig(INT_EBL_ADC |
								6 <<
								ADC_BATMON_INT_ENABLE_ADC_INT_CH_NUM_Pos |
								INT_EBL_BATMON_ALARM);

	/* Set the battery monitor configuration, use channel ADC_CHANNEL to battery
	 * monitoring. */
	Sys_ADC_Set_BATMONConfig((100 << ADC_BATMON_CFG_ALARM_COUNT_VALUE_Pos) |
							 (THRESHOLD_CFG <<
							  ADC_BATMON_CFG_SUPPLY_THRESHOLD_Pos) |
							 BATMON_CH(6));


	/* Configure ADC_CHANNEL input selection to VBAT/2 */
	Sys_ADC_InputSelectConfig(6, ADC_POS_INPUT_VBAT_DIV2 |
							  ADC_NEG_INPUT_GND);

	/////////////////////////
	//DIO1
	Sys_ADC_InputSelectConfig(4, ADC_POS_INPUT_DIO1 | ADC_NEG_INPUT_GND);

	/* Configure both input selection for an ADC channel to GND so the OFFSET is
	   * subtracted automatically to result. */
	  Sys_ADC_InputSelectConfig(0, ADC_POS_INPUT_GND |
								ADC_NEG_INPUT_GND);


   NVIC_EnableIRQ(ADC_BATMON_IRQn);
}



/* ----------------------------------------------------------------------------
 * Function      : void ADC_BATMON_IRQHandler(void)
 * ----------------------------------------------------------------------------
 * Description   : Handle BATMON alarm interrupt when the VBAT falls under the
 *                 configured threshold. Trigger an APP_BATT_LEVEL_LOW event.
 * Inputs        : None
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
void ADC_BATMON_IRQHandler(void)
{

	  uint8_t batt_lvl_percent;
	  uint32_t batt_lvl_mV;

	 /* Get status of ADC */
	 uint32_t adc_status = Sys_ADC_Get_BATMONStatus();
	 if ((adc_status & (1 << ADC_BATMON_STATUS_BATMON_ALARM_STAT_Pos)) ==
		 BATMON_ALARM_TRUE)
	 {
		 /* Battery monitor alarm status is set */

		 /* Clear the battery monitor status and counter */
		 Sys_ADC_Clear_BATMONStatus();

	 }
	 else
	 {



		 app_batt_read.batt_lvl_sum_mV =  ADC->DATA_TRIM_CH[4];

 	 }

}

