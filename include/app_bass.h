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
 * app_bass.h
 * - BASS Application-specific header file
 * ------------------------------------------------------------------------- */

#ifndef APP_BASS_H
#define APP_BASS_H

/* ----------------------------------------------------------------------------
 * If building with a C++ compiler, make all of the definitions in this header
 * have a C binding.
 * ------------------------------------------------------------------------- */
#ifdef __cplusplus
extern "C"
{
#endif    /* ifdef __cplusplus */

/* ----------------------------------------------------------------------------
 * Defines
 * --------------------------------------------------------------------------*/

#define CONCAT(x, y)                     x##y
#define BATMON_CH(x)                     CONCAT(BATMON_CH, x)

#define APP_BAS_NB                       1  /* Number of batteries (1 or 2) */
#define BATT_LEVEL_LOW_THRESHOLD_PERCENT 15 /* Battery level low at 15% of 1.1V to 1.4V range */

/* ADC, VBAT and BATMON alarm configuration */
#define VBAT_1p1V_MEASURED               0x11BF
#define VBAT_1p4V_MEASURED               0x168C
#define BATMON_SUPPLY_THRESHOLD_CFG      (((VBAT_1p4V_MEASURED - VBAT_1p1V_MEASURED) * \
                                           BATT_LEVEL_LOW_THRESHOLD_PERCENT / 100 +   \
                                           VBAT_1p1V_MEASURED) * 256 / 0x4000 + 1)
#define ADC_BATMON_CH                    6
#define ADC_GND_CH                       0

#define THRESHOLD                       1.6
#define THRESHOLD_CFG                   ((uint32_t)(THRESHOLD / (2 * 0.0078)))

void APP_BASS_SetBatMonAlarm(uint32_t supplyThresholdCfg);

uint8_t APP_BASS_ReadBatteryLevel(uint8_t bas_nb);

void APP_BASS_ReadBatteryLevel_Handler(ke_msg_id_t const msg_id,
									   void const *param,
									   ke_task_id_t const dest_id,
									   ke_task_id_t const src_id);

void APP_BASS_BattLevelLow_Handler(ke_msg_id_t const msg_id,
                                   void const *param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id);

extern void ADC_BATMON_IRQHandler(void);

/* ----------------------------------------------------------------------------
 * Close the 'extern "C"' block
 * ------------------------------------------------------------------------- */

struct app_batt_read_t
{
    uint32_t batt_lvl_sum_mV;
    uint8_t prev_batt_lvl_percent;
    uint8_t read_cnt;
};
extern  struct app_batt_read_t app_batt_read;

#ifdef __cplusplus
}
#endif    /* ifdef __cplusplus */

#endif    /* APP_H */
