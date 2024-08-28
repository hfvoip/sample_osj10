#include <app.h>
#include <math.h>
#include "sharedBuffers.h"
#include "playtone.h"
#include <printf.h>

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

void Dec_Volume() {
	float volume_step =3;
	float max_vol = MCU_VOLUME.Volume;
	float min_vol =  MCU_VOLUME.Volume;

	//这里也改为8档
	bool bcontinue = (min_vol>=(0- 6*  volume_step));
	if (!bcontinue) {
		Start_Playtone(1000,2,6,1);
		Fill_SmData_Buffer();

		SM_Ptr->Control |= MASK16(UPDATE_CFG);
		return;
	}else {
		 MCU_VOLUME.Volume -=   volume_step;

	}
		Start_Playtone(400,1,6,1);
		Fill_SmData_Buffer();


		SM_Ptr->Control |= MASK16(UPDATE_CFG);
}


void Inc_Volume() {

	float max_vol =  MCU_VOLUME.Volume;
	float volume_step =3;

	bool bcontinue = (max_vol<= (0-volume_step));
	//音量到最大，长度一声
	if (!bcontinue) {

			Start_Playtone(1000,2,6,1);
			Fill_SmData_Buffer();

			SM_Ptr->Control |= MASK16(UPDATE_CFG);

			return;




	}else {
		 MCU_VOLUME.Volume += volume_step;
		//在这里看是否到了最大音量，如果到了，就播最大音量

		Start_Playtone(400,1,6,1);
	}

	Fill_SmData_Buffer();

	SM_Ptr->Control |= MASK16(UPDATE_CFG);


}
//mode: 0..3
#define TOTAL_MEM_NUMS  4
#define BYTESIZE_PERMEM  128
void Change_Mode(uint8_t mem_idx) {



		if (mem_idx >= TOTAL_MEM_NUMS) mem_idx = 0;


		if ((mem_idx >=0 ) && (mem_idx < TOTAL_MEM_NUMS)) {
			 for (int i=0;i<100;i++)
					cs_env[0].arr_params[i] = cs_env[0].arr_flash_params[BYTESIZE_PERMEM*mem_idx+i];

			 //确保第一个字节不是0xAA,否则会陷入循环
			 cs_env[0].arr_params[0] = mem_idx;
			 Update_SMData_RX(cs_env[0].arr_params, BYTESIZE_PERMEM);
			Start_Playtone(500,mem_idx+1,-9,1);
			 Fill_SmData_Buffer();
			 if (cs_env[0].wdrc_mask  & 0x10)  SM_Ptr->Control |= MASK16(WDRC);
			 else
				 SM_Ptr->Control &= ~MASK16(WDRC);

			 if (cs_env[0].wdrc_mask  & 0x8)  SM_Ptr->Control |= MASK16(EQ);
			 else
				 SM_Ptr->Control &= ~MASK16(EQ);

			 SM_Ptr->Control |= MASK16(UPDATE_CFG);



		}


}

void Normal_BUTTON_Handler() {
	//检查DIO5 (T3), DIO11(RIGHT),DIO7(LEFT)

	uint8_t btn_flag_7 = DIO_DATA->ALIAS[LEFT_BLE_BUTTON];
	uint8_t btn_flag_11 = DIO_DATA->ALIAS[RIGHT_BLE_BUTTON];
	uint8_t btn_flag_5 = DIO_DATA->ALIAS[T3_BUTTON];
	uint8_t btn_flag_4 = DIO_DATA->ALIAS[WIFI_AUDIO_BUTTON];
	PRINTF("\r\n 5:%d 7:%d 11:%d", btn_flag_5, btn_flag_7,btn_flag_11);
	 Sys_Delay_ProgramROM(0.5 * SystemCoreClock);
	if (btn_flag_7 ==0) {
	//	PRINTF("DIO7 PRESSED\r\n");
	    Sys_Delay_ProgramROM(0.5 * SystemCoreClock);

	}

	if (btn_flag_11 ==0) {
	//	PRINTF("DIO11 PRESSED\r\n");
	    Sys_Delay_ProgramROM(0.5 * SystemCoreClock);

	}
	if (btn_flag_5 ==1) {
		//	PRINTF("DIO5 PRESSED\r\n");
		    Sys_Delay_ProgramROM(0.5 * SystemCoreClock);

		}

}
