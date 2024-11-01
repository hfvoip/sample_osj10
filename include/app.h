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
 * ----------------------------------------------------------------------------
 * app.h
 * - Overall application header file for the DMIC OD sample application
 * ------------------------------------------------------------------------- */

#ifndef APP_H_
#define APP_H_

/* ----------------------------------------------------------------------------
 * If building with a C++ compiler, make all of the definitions in this header
 * have a C binding.
 * ------------------------------------------------------------------------- */
#ifdef __cplusplus
extern "C"
{
#endif    /* ifdef __cplusplus */

#define ENABLE_CMSIS_FUNC  1
/* ----------------------------------------------------------------------------
 * Include files
 * --------------------------------------------------------------------------*/

//#include <Driver_GPIO.h>
//#include <GPIO_RSLxx.h>
//#include <USART_RSLxx.h>


/* ----------------------------------------------------------------------------
 * Defines
 * --------------------------------------------------------------------------*/
//#if !RTE_USART
//    #error "Please configure USART0 in RTE_Device.h"
//#endif    /* if !RTE_USART0 */

#include <rsl10.h>

#define BLE_FITTING
//在改为button_mode的时候同时要修改 onebtn.h, threebtn.h等的if 0/1 ,以及app.h的include
//#define BUTTON_MODE		 3 //3或1

//GZ_VERSION :骨传导眼镜
//YN_VERSION : 一诺
//DA_VERSION : 大耳朵


//需要加一些预定义的变量，要通过GUI加，不要在此文件中加


//#define CFG_ALLROLES  1
//#define CFG_BLE    1
//#define CFG_APP    1
//#define CFG_ATTS   1
//#define CFG_CON    1
//#define CFG_NB_PRF   2
//#define CFG_PRF    1
//#define CFG_EMB    1

//需要在.rteconfig里禁掉RTT
//以及 #define  OUTPUT_INTERFACE  -1或0
//以及ble libary中需要添加


#ifdef BLE_FITTING
#include <rsl10_ke.h>
#include <rsl10_ble.h>
#include <rsl10_profiles.h>
#include <rsl10_map_nvr.h>
#include <stdbool.h>
#include <rsl10_flash_rom.h>
#include <rsl10_protocol.h>
#include "ble_std.h"
#include "ble_custom.h"



/* Set timer to 200 ms (20 times the 10 ms kernel timer resolution) */
#define TIMER_200MS_SETTING             20

/* Set timer to 2s (200 times the 10 ms kernel timer resolution) */
#define TIMER_2S_SETTING                200

typedef void (*appm_add_svc_func_t)(void);
#define DEFINE_SERVICE_ADD_FUNCTION(func) (appm_add_svc_func_t)func
#define DEFINE_MESSAGE_HANDLER(message, handler) { message, \
                                                   (ke_msg_func_t)handler }

/* List of message handlers that are used by the different profiles/services */
#define APP_MESSAGE_HANDLER_LIST                       \
    DEFINE_MESSAGE_HANDLER(APP_TEST_TIMER, APP_Timer), \
    DEFINE_MESSAGE_HANDLER(LED_TIMER, LED_Timer)

/* List of functions used to create the database */
#define SERVICE_ADD_FUNCTION_LIST                        \
    DEFINE_SERVICE_ADD_FUNCTION(CustomService_ServiceAdd)

typedef void (*appm_enable_svc_func_t)(uint8_t);
#define DEFINE_SERVICE_ENABLE_FUNCTION(func) (appm_enable_svc_func_t)func

/* List of functions used to enable client services */
#define SERVICE_ENABLE_FUNCTION_LIST \
    DEFINE_SERVICE_ENABLE_FUNCTION(Batt_ServiceEnable_Server)

/* ----------------------------------------------------------------------------
 * Global variables and types
 * --------------------------------------------------------------------------*/
extern const struct ke_task_desc TASK_DESC_APP;

/* APP Task messages */
enum appm_msg
{
    APPM_DUMMY_MSG = TASK_FIRST_MSG(TASK_ID_APP),

    /* Timer used to have a tick periodically for application */
    APP_TEST_TIMER,

    /* Timer used to control the behavior of the LED_DIO_NUM according to
     * the connection states  */
    LED_TIMER,
	APP_LED_TIMEOUT,
	 APP_BATT_LEVEL_LOW,
	APP_BATT_LEVEL_READ_TIMEOUT,
	APP_TIMEOUT_WHITELIST /* Timeout to accept connections from not previously bonded clients */

};

struct app_env_tag
{
    /* Battery service */
    uint8_t batt_lvl;
    uint8_t batt_lvl_low1;
    uint32_t sum_batt_lvl;
    uint32_t sum_batt_lvl_0v;
    uint32_t sum_batt_lvl_3v;

    uint16_t num_batt_read;
    bool send_batt_ntf[NUM_MASTERS];
    uint8_t  ear_side;
    uint8_t  ble_key[10];
    uint8_t  upd_ble_key;
    uint8_t  min_volume;
    uint8_t  max_volume;
    uint8_t  volume_step;
    uint8_t  poweron_delay;
    short  playtone_gain;
    int playtone_freq;
    uint8_t sleep_mode;
    uint8_t arr_temperature[2];
    uint8_t arr_heartrate[2];


};

/* Support for the application manager and the application environment */
extern struct app_env_tag app_env;

/* List of functions used to create the database */
extern const appm_add_svc_func_t appm_add_svc_func_list[];


extern int Msg_Handler(ke_msg_id_t const msgid, void *param,
                       ke_task_id_t const dest_id,
                       ke_task_id_t const src_id);




#endif




/* ----------------------------------------------------------------------------
 * Defines
 * --------------------------------------------------------------------------*/
#define CONCAT(x, y)                    x##y
#define DIO_SRC(x)                      CONCAT(DIO_SRC_DIO_, x)


#if ((GZ_VERSION==1) && (PRODUCTION_MODE==1))
	#define BUTTON_DIO                      12
	#define LED_DIO                         6
	/* DIO number that is used for easy re-flashing (recovery mode) */
	#define RECOVERY_DIO                    7
    #define DIO_ISACTIVE					0

#else
#if ((YN_VERSION ==1) && (PRODUCTION_MODE==1) )
	#define BUTTON_DIO                       1
	#define LED_DIO                         11
	/* DIO number that is used for easy re-flashing (recovery mode) */
	#define RECOVERY_DIO                    1
    #define DIO_ISACTIVE					1

#else
	#define BUTTON_DIO                      5
	#define LED_DIO                         6
	/* DIO number that is used for easy re-flashing (recovery mode) */
	#define RECOVERY_DIO                    13
    #define DIO_ISACTIVE					0

#endif
#endif


/* DIO used for the I2C interface to interface the SI7042 sensor */
#define I2C_SDA_DIO_NUM                 11
#define I2C_SCL_DIO_NUM                 13
#define I2C_GND_DIO_NUM                 10
#define I2C_PWR_DIO_NUM                 0



#define AUDIO_DMIC0_GAIN                0xFFF
#define AUDIO_DMIC1_GAIN                0xFFF
#define AUDIO_OD_GAIN                   0xAFF

#define AUDIO_CONFIG                    (OD_AUDIOSLOWCLK            | \
                                         DMIC_AUDIOCLK              | \
                                         DECIMATE_BY_64             | \
                                         OD_UNDERRUN_PROTECT_ENABLE | \
                                         OD_DATA_MSB_ALIGNED        | \
                                         DMIC0_DATA_MSB_ALIGNED     | \
                                         DMIC1_DATA_MSB_ALIGNED     | \
                                         OD_DMA_REQ_DISABLE         | \
                                         DMIC0_DMA_REQ_DISABLE      | \
                                         DMIC1_DMA_REQ_DISABLE      | \
                                         OD_INT_GEN_DISABLE         | \
                                         DMIC0_INT_GEN_ENABLE       | \
                                         DMIC1_INT_GEN_DISABLE      | \
                                         OD_ENABLE                  | \
                                         DMIC0_ENABLE               | \
                                         DMIC1_ENABLE)




#define DECIMATE_BY_200                 ((uint32_t)(0x11U << \
                                                    AUDIO_CFG_DEC_RATE_Pos))
#define DECIMATE_BY_120                 ((uint32_t)(0x7U << \
														AUDIO_CFG_DEC_RATE_Pos))
#define DECIMATE_BY_96                 ((uint32_t)(0x4U << \
														AUDIO_CFG_DEC_RATE_Pos))
#define DECIMATE_BY_80                 ((uint32_t)(0x2U << \
															AUDIO_CFG_DEC_RATE_Pos))
#define DECIMATE_BY_160                 ((uint32_t)(0xCU << \
																AUDIO_CFG_DEC_RATE_Pos))
#define DECIMATE_BY_64                 ((uint32_t)(0x0U << \
															AUDIO_CFG_DEC_RATE_Pos))


#define AUDIO_CONFIG_PLAY                  (OD_AUDIOCLK                        | \
                                         OD_UNDERRUN_PROTECT_ENABLE         | \
                                         OD_DMA_REQ_ENABLE                  | \
                                         OD_INT_GEN_DISABLE                 | \
                                         DECIMATE_BY_200                    | \
                                         OD_ENABLE)

#define RX_DMA_PLAY                      (DMA_LITTLE_ENDIAN |        \
                                        DMA_ENABLE |               \
                                        DMA_DISABLE_INT_DISABLE |  \
                                        DMA_ERROR_INT_DISABLE |    \
                                        DMA_COMPLETE_INT_ENABLE | \
                                        DMA_COUNTER_INT_ENABLE |  \
                                        DMA_START_INT_DISABLE |    \
                                        DMA_DEST_WORD_SIZE_16 |    \
                                        DMA_SRC_WORD_SIZE_32 |     \
                                        DMA_SRC_ADDR_INC |         \
                                        DMA_TRANSFER_M_TO_P |      \
                                        DMA_DEST_ADDR_STATIC |     \
                                        DMA_DEST_OD |              \
                                        DMA_PRIORITY_0 |           \
                                        DMA_ADDR_CIRC)

#if AUDIO_BAK
#define AUDIO_CONFIG_16K                 (DMIC_AUDIOCLK                      | \
										  OD_AUDIOCLK                        | \
										 OD_UNDERRUN_PROTECT_ENABLE         | \
										 OD_DATA_LSB_ALIGNED       			| \
										 OD_DMA_REQ_ENABLE                  | \
										 OD_INT_GEN_DISABLE                 | \
										 DECIMATE_BY_200                    | \
										 OD_ENABLE                          | \
                                         DMIC1_DMA_REQ_DISABLE              | \
                                         DMIC1_INT_GEN_DISABLE              | \
                                         DMIC1_DATA_LSB_ALIGNED             | \
                                         DMIC1_DISABLE                      | \
                                         DMIC0_DMA_REQ_ENABLE              | \
                                         DMIC0_INT_GEN_DISABLE              | \
                                         DMIC0_DATA_LSB_ALIGNED             | \
                                         DMIC0_ENABLE)
#else
#define AUDIO_CONFIG_16K                 (DMIC_AUDIOCLK                      | \
										  OD_AUDIOCLK                    | \
										 OD_UNDERRUN_PROTECT_DISABLE         | \
										 OD_DATA_MSB_ALIGNED       			| \
										 OD_DMA_REQ_ENABLE                  | \
										 OD_INT_GEN_DISABLE                 | \
										 DECIMATE_BY_200                    | \
										 OD_ENABLE                          | \
                                         DMIC1_DMA_REQ_DISABLE              | \
                                         DMIC1_INT_GEN_DISABLE              | \
                                         DMIC1_DATA_MSB_ALIGNED             | \
                                         DMIC1_DISABLE                      | \
                                         DMIC0_DMA_REQ_ENABLE              | \
                                         DMIC0_INT_GEN_DISABLE              | \
                                         DMIC0_DATA_MSB_ALIGNED             | \
                                         DMIC0_ENABLE)
#endif
#define AUDIO_CONFIG_48K                 (DMIC_AUDIOCLK                      | \
										  OD_AUDIOCLK                    | \
										 OD_UNDERRUN_PROTECT_DISABLE         | \
										 OD_DATA_MSB_ALIGNED       			| \
										 OD_DMA_REQ_ENABLE                  | \
										 OD_INT_GEN_DISABLE                 | \
										 DECIMATE_BY_112                    | \
										 OD_ENABLE                          | \
                                         DMIC1_DMA_REQ_DISABLE              | \
                                         DMIC1_INT_GEN_DISABLE              | \
                                         DMIC1_DATA_MSB_ALIGNED             | \
                                         DMIC1_DISABLE                      | \
                                         DMIC0_DMA_REQ_ENABLE              | \
                                         DMIC0_INT_GEN_DISABLE              | \
                                         DMIC0_DATA_MSB_ALIGNED             | \
                                         DMIC0_ENABLE)
#if PRESCALE_3
#define AUDIO_CONFIG_32K                 (DMIC_AUDIOCLK                      | \
										  OD_AUDIOCLK                    | \
										 OD_UNDERRUN_PROTECT_DISABLE         | \
										 OD_DATA_MSB_ALIGNED       			| \
										 OD_DMA_REQ_ENABLE                  | \
										 OD_INT_GEN_DISABLE                 | \
										 DECIMATE_BY_168                    | \
										 OD_ENABLE                          | \
                                         DMIC1_DMA_REQ_DISABLE              | \
                                         DMIC1_INT_GEN_DISABLE              | \
                                         DMIC1_DATA_MSB_ALIGNED             | \
                                         DMIC1_DISABLE                      | \
                                         DMIC0_DMA_REQ_ENABLE              | \
                                         DMIC0_INT_GEN_DISABLE              | \
                                         DMIC0_DATA_MSB_ALIGNED             | \
                                         DMIC0_ENABLE)
#else
#define AUDIO_CONFIG_32K                 (DMIC_AUDIOCLK                      | \
											  OD_AUDIOCLK					 | \
											 OD_UNDERRUN_PROTECT_DISABLE		 | \
											 OD_DATA_MSB_ALIGNED				| \
											 OD_DMA_REQ_ENABLE					| \
											 OD_INT_GEN_DISABLE 				| \
											 DECIMATE_BY_96					| \
											 OD_ENABLE							| \
											 DMIC1_DMA_REQ_DISABLE				| \
											 DMIC1_INT_GEN_DISABLE				| \
											 DMIC1_DATA_MSB_ALIGNED 			| \
											 DMIC1_DISABLE						| \
											 DMIC0_DMA_REQ_ENABLE			   | \
											 DMIC0_INT_GEN_DISABLE				| \
											 DMIC0_DATA_MSB_ALIGNED 			| \
											 DMIC0_ENABLE)
#endif

#if 0
#define DMIC_CFG_NO_DELAY               (DMIC0_DCRM_CUTOFF_20HZ | \
                                         DMIC1_DCRM_CUTOFF_20HZ | \
                                         DMIC1_DELAY_DISABLE    | \
                                         DMIC0_FALLING_EDGE     | \
                                         DMIC1_RISING_EDGE)

#define DMIC_CFG_DELAY                  (DMIC0_DCRM_CUTOFF_20HZ | \
                                         DMIC1_DCRM_CUTOFF_20HZ | \
                                         DMIC0_FALLING_EDGE     | \
                                         DMIC1_RISING_EDGE)
#endif

#define DISTANCE                        0.01
#define DELAY_NS                        (1000000000 * DISTANCE / 343)
#define FREQ_MHZ                        2
#define US_TO_NS                        1000

#define DELAY_CFG                      ((int32_t)(DELAY_NS * FREQ_MHZ * 8) / \
                                        (64 * US_TO_NS))
#define FRAC_DELAY_CFG                 ((int32_t)((DELAY_NS - \
                                                   (DELAY_CFG * 64 * US_TO_NS / \
                                                    (FREQ_MHZ * 8))) * FREQ_MHZ / US_TO_NS))



// this is defined same as RX_DMA_OD in ble_android_asha_OD\app_audio.h
//
#define RX_DMA_OD                      (DMA_LITTLE_ENDIAN |        \
                                        DMA_ENABLE |               \
                                        DMA_DISABLE_INT_DISABLE |  \
                                        DMA_ERROR_INT_DISABLE |    \
                                        DMA_COMPLETE_INT_ENABLE | \
                                        DMA_COUNTER_INT_ENABLE |  \
                                        DMA_START_INT_DISABLE |    \
                                        DMA_DEST_WORD_SIZE_32 |    \
                                        DMA_SRC_WORD_SIZE_32 |     \
                                        DMA_SRC_ADDR_INC |         \
                                        DMA_TRANSFER_M_TO_P |      \
                                        DMA_DEST_ADDR_STATIC |     \
                                        DMA_DEST_OD |              \
                                        DMA_PRIORITY_0 |           \
                                        DMA_ADDR_CIRC)

// DEST_SELECT (14:12) ignored; for OD SRC_SELECT (11:9) seems to have been ignored as well (RSL10_hardware_reference.pdf page 366)
// DMA_SRC_WORD_SIZE_ and DMA_DEST_WORD_SIZE_ selections made based on corresponding values for OD
// SRC_ADDR_STEP_MODE and DEST_ADDR_STEP_MODE not specified following OD example
// SRC_ADDR_STEP_SIZE and DEST_ADDR_STEP_SIZE not specified following OD example
#define RX_DMA_DMIC                     (DMA_ENABLE               |\
                                         DMA_ADDR_CIRC            |\
                                         DMA_SRC_ADDR_STATIC      |\
                                         DMA_DEST_ADDR_INC        |\
                                         DMA_TRANSFER_P_TO_M      |\
                                         DMA_PRIORITY_0           |\
                                         DMA_SRC_DMIC             |\
                                         DMA_SRC_WORD_SIZE_32    |\
                                         DMA_DEST_WORD_SIZE_32    |\
                                         DMA_START_INT_DISABLE    |\
                                         DMA_COUNTER_INT_ENABLE  |\
                                         DMA_COMPLETE_INT_ENABLE  |\
                                         DMA_ERROR_INT_ENABLE    |\
                                         DMA_DISABLE_INT_DISABLE  |\
                                         DMA_LITTLE_ENDIAN)

#define INPUTGPIO_SETTING				DIO_MODE_GPIO_IN_0  | DIO_WEAK_PULL_UP |DIO_LPF_DISABLE

/* ----------------------------------------------------------------------------
 * Function prototype definitions
 * --------------------------------------------------------------------------*/

extern void DIO0_IRQHandler(void);
extern void Update_SmData_Buffer();
extern void Audio_Enhancement();
extern void Stop_Audiofunc();
extern int LED_Timer(ke_msg_id_t const msg_id,
              void const *param,
              ke_task_id_t const dest_id,
              ke_task_id_t const src_id);
extern int APP_Timer(ke_msg_id_t const msg_id,
              void const *param,
              ke_task_id_t const dest_id,
              ke_task_id_t const src_id);
void Normal_BUTTON_Handler();

extern uint8_t BUTTON_VOLUME1  ; //				11
extern uint8_t  BUTTON_VOLUME2  ;//			13
extern uint8_t  DMIC_CLK_DIO   ;//                 14
extern uint8_t  DMIC_DATA_DIO   ;//                10
extern uint8_t  OD_P_DIO       ;//                 1
extern uint8_t  OD_N_DIO       ;//                 3

extern unsigned char  app_resetcode ;

#define LEFT_BLE_BUTTON 7
#define RIGHT_BLE_BUTTON	11
#define  T3_BUTTON			5
#define  WIFI_AUDIO_BUTTON	4


/* ----------------------------------------------------------------------------
 * Global variables and types
 * --------------------------------------------------------------------------*/
#if  0
extern ARM_DRIVER_USART Driver_USART0;
extern DRIVER_GPIO_t Driver_GPIO;

extern ARM_DRIVER_USART *uart;
extern DRIVER_GPIO_t *gpio;
#endif
extern char rx_buffer[];
/* ---------------------------------------------------------------------------
* Function prototype definitions
* --------------------------------------------------------------------------*/
void Usart_EventCallBack(uint32_t event);
int main_bleclient();


void  ADC_BUTTON_Handler();

/* ----------------------------------------------------------------------------
 * Close the 'extern "C"' block
 * ------------------------------------------------------------------------- */
#ifdef __cplusplus
}
#endif    /* ifdef __cplusplus */

#endif    /* APP_H_ */


