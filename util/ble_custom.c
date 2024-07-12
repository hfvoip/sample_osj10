/* ----------------------------------------------------------------------------
 * Copyright (c) 2015-2017 Semiconductor Components Industries, LLC (d/b/a
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
 * ble_custom.c
 * - Bluetooth custom service functions
 * ----------------------------------------------------------------------------
 * $Revision: 1.33 $
 * $Date: 2019/12/20 22:05:14 $
 * ------------------------------------------------------------------------- */

#include "app.h"
#include <printf.h>

#include "sharedBuffers.h"


#ifdef BLE_FITTING
struct app_env_tag app_env;
/* Global variable definition */
struct cs_env_tag cs_env[NUM_MASTERS];
const uint32_t cs_atts_len[] = {
    [CS_IDX_TX_VALUE_VAL] = CS_TX_VALUE_MAX_LENGTH,
    [CS_IDX_RX_VALUE_VAL] = CS_RX_VALUE_MAX_LENGTH,
    [CS_IDX_TX_LONG_VALUE_VAL] = CS_TX_LONG_VALUE_MAX_LENGTH,
    [CS_IDX_RX_LONG_VALUE_VAL] = CS_RX_LONG_VALUE_MAX_LENGTH,
};
extern ShareMemoryData* SM_Ptr;
/* ----------------------------------------------------------------------------
 * Function      : void CustomService_Env_Initialize(void)
 * ----------------------------------------------------------------------------
 * Description   : Initialize custom service environment
 * Inputs        : None
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
void CustomService_Env_Initialize(void)
{
    for (unsigned int i = 0; i < NUM_MASTERS; i++)
    {
        /* Reset the application manager environment */
        memset(&cs_env[i], 0, sizeof(struct cs_env_tag));
        cs_env[i].sent_success = true;
        cs_env[i].tx_cccd_value = ATT_CCC_START_NTF;
        cs_env[i].rx_cccd_value = 0;
        cs_env[i].j11dump_flag = 0;

    }
}

/* ----------------------------------------------------------------------------
 * Function      : void CustomService_ServiceAdd(void)
 * ----------------------------------------------------------------------------
 * Description   : Send request to add custom profile into the attribute
 *                 database.Defines the different access functions
 *                 (setter/getter commands to access the different
 *                 characteristic attributes).
 * Inputs        : None
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
void CustomService_ServiceAdd(void)
{
    struct gattm_add_svc_req *req =
        KE_MSG_ALLOC_DYN(GATTM_ADD_SVC_REQ,
                         TASK_GATTM, TASK_APP,
                         gattm_add_svc_req,
                         CS_IDX_NB * sizeof(struct gattm_att_desc));

    const uint8_t svc_uuid[ATT_UUID_128_LEN] = CS_SVC_UUID;

    const struct gattm_att_desc att[CS_IDX_NB] =
    {
        /* Attribute Index  = Attribute properties: UUID,
         *                                          Permissions,
         *                                          Max size,
         *                                          Extra permissions */

        /* TX Characteristic */
        [CS_IDX_TX_VALUE_CHAR]     = ATT_DECL_CHAR(),

        [CS_IDX_TX_VALUE_VAL]      = ATT_DECL_CHAR_UUID_128(CS_CHARACTERISTIC_TX_UUID,
                                                            PERM(RD, ENABLE) | PERM(NTF, ENABLE),
                                                            CS_TX_VALUE_MAX_LENGTH),

        [CS_IDX_TX_VALUE_CCC]      = ATT_DECL_CHAR_CCC(),

        [CS_IDX_TX_VALUE_USR_DSCP] = ATT_DECL_CHAR_USER_DESC(CS_USER_DESCRIPTION_MAX_LENGTH),

        /* RX Characteristic */
        [CS_IDX_RX_VALUE_CHAR]     = ATT_DECL_CHAR(),

        [CS_IDX_RX_VALUE_VAL]      = ATT_DECL_CHAR_UUID_128(CS_CHARACTERISTIC_RX_UUID,
                                                            PERM(RD, ENABLE) | PERM(WRITE_REQ, ENABLE)
                                                            | PERM(WRITE_COMMAND, ENABLE),
                                                            CS_RX_VALUE_MAX_LENGTH),

        [CS_IDX_RX_VALUE_CCC]      = ATT_DECL_CHAR_CCC(),
        [CS_IDX_RX_VALUE_USR_DSCP] = ATT_DECL_CHAR_USER_DESC(CS_USER_DESCRIPTION_MAX_LENGTH),

        /* TX long Characteristic */
        [CS_IDX_TX_LONG_VALUE_CHAR] = ATT_DECL_CHAR(),

        [CS_IDX_TX_LONG_VALUE_VAL]  = ATT_DECL_CHAR_UUID_128(CS_CHARACTERISTIC_TX_LONG_UUID,
                                                             PERM(RD, ENABLE), CS_TX_LONG_VALUE_MAX_LENGTH),

        [CS_IDX_TX_LONG_VALUE_USR_DSCP] = ATT_DECL_CHAR_USER_DESC(CS_USER_DESCRIPTION_MAX_LENGTH),

        /* RX long Characteristic */
        [CS_IDX_RX_LONG_VALUE_CHAR]     = ATT_DECL_CHAR(),

        [CS_IDX_RX_LONG_VALUE_VAL]      = ATT_DECL_CHAR_UUID_128(CS_CHARACTERISTIC_RX_LONG_UUID,
                                                                 PERM(RD, ENABLE) | PERM(WRITE_REQ, ENABLE)
                                                                 | PERM(WRITE_COMMAND, ENABLE),
                                                                 CS_RX_LONG_VALUE_MAX_LENGTH),

        [CS_IDX_RX_LONG_VALUE_USR_DSCP] = ATT_DECL_CHAR_USER_DESC(CS_USER_DESCRIPTION_MAX_LENGTH),
    };

    /* Fill the add custom service message */
    req->svc_desc.start_hdl = 0;
    req->svc_desc.task_id = TASK_APP;
    req->svc_desc.perm = PERM(SVC_UUID_LEN, UUID_128);
    req->svc_desc.nb_att = CS_IDX_NB;

    memcpy(&req->svc_desc.uuid[0], &svc_uuid[0], ATT_UUID_128_LEN);

    for (unsigned int i = 0; i < CS_IDX_NB; i++)
    {
        memcpy(&req->svc_desc.atts[i], &att[i],
               sizeof(struct gattm_att_desc));
    }

    /* Send the message */
    ke_msg_send(req);
}

/* ----------------------------------------------------------------------------
 * Function      : int GATTM_AddSvcRsp(ke_msg_id_t const msg_id,
 *                                     struct gattm_add_svc_rsp
 *                                     const *param,
 *                                     ke_task_id_t const dest_id,
 *                                     ke_task_id_t const src_id)
 * ----------------------------------------------------------------------------
 * Description   : Handle the response from adding a service in the attribute
 *                 database from the GATT manager
 * Inputs        : - msg_id     - Kernel message ID number
 *                 - param      - Message parameters in format of
 *                                struct gattm_add_svc_rsp
 *                 - dest_id    - Destination task ID number
 *                 - src_id     - Source task ID number
 * Outputs       : return value - Indicate if the message was consumed;
 *                                compare with KE_MSG_CONSUMED
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
int GATTM_AddSvcRsp(ke_msg_id_t const msg_id,
                    struct gattm_add_svc_rsp const *param,
                    ke_task_id_t const dest_id,
                    ke_task_id_t const src_id)
{
    for (unsigned int i = 0; i < NUM_MASTERS; i++)
    {
        cs_env[i].start_hdl = param->start_hdl;
    }

 //   PRINTF("__CUSTOM SERVICE ADDED\n");
    /* Add the next requested service  */
    if (!Service_Add())
    {
        /* All services have been added, go to the ready state
         * and start advertising */
        for (unsigned int i = 0; i < NUM_MASTERS; i++)
        {
            ble_env[i].state = APPM_READY;
        }
        Advertising_Start();
    }

    return (KE_MSG_CONSUMED);
}

/* ----------------------------------------------------------------------------
 * Function      : int GATTC_ReadReqInd(ke_msg_id_t const msg_id,
 *                                      struct gattc_read_req_ind
 *                                      const *param,
 *                                      ke_task_id_t const dest_id,
 *                                      ke_task_id_t const src_id)
 * ----------------------------------------------------------------------------
 * Description   : Handle received read request indication
 *                 from a GATT Controller
 * Inputs        : - msg_id     - Kernel message ID number
 *                 - param      - Message parameters in format of
 *                                struct gattc_read_req_ind
 *                 - dest_id    - Destination task ID number
 *                 - src_id     - Source task ID number
 * Outputs       : return value - Indicate if the message was consumed;
 *                                compare with KE_MSG_CONSUMED
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
int GATTC_ReadReqInd(ke_msg_id_t const msg_id,
                     struct gattc_read_req_ind const *param,
                     ke_task_id_t const dest_id,
                     ke_task_id_t const src_id)
{
    uint8_t length = 0;
    uint8_t status = GAP_ERR_NO_ERROR;
    uint16_t attnum;
    uint8_t *valptr = NULL;

    //我们用的是 RX_VALUE (attid=5),其他 CHAR UUID 没有用
    /* Retrieve the index of environment structure representing peer device */
    signed int device_indx = Find_Connected_Device_Index(KE_IDX_GET(src_id));

    if (device_indx == INVALID_DEV_IDX)
    {
        return (KE_MSG_CONSUMED);
    }

    struct gattc_read_cfm *cfm;

    /* Set the attribute handle using the attribute index
     * in the custom service */
    if (param->handle > cs_env[device_indx].start_hdl)
    {
        attnum = (param->handle - cs_env[device_indx].start_hdl - 1);
    }
    else
    {
        status = ATT_ERR_INVALID_HANDLE;
    }

    /* If there is no error, send back the requested attribute value */
    if (status == GAP_ERR_NO_ERROR)
    {
        switch (attnum)
        {
            case CS_IDX_RX_VALUE_VAL:
            {
                length = CS_RX_VALUE_MAX_LENGTH;
                Readfrom_SmData_Buffer(cs_env[device_indx].arr_params);
                valptr = (uint8_t *)&cs_env[device_indx].arr_params;  //从arr_params 中读取
            }
            break;

            case CS_IDX_RX_VALUE_CCC:
            {
                length = 2;
                valptr = (uint8_t *)&cs_env[device_indx].rx_cccd_value;
            }
            break;

            case CS_IDX_RX_VALUE_USR_DSCP:
            {
                length = strlen(CS_RX_CHARACTERISTIC_NAME);
                valptr = (uint8_t *)CS_RX_CHARACTERISTIC_NAME;
            }
            break;

            case CS_IDX_TX_VALUE_VAL:
            {
                length = CS_TX_VALUE_MAX_LENGTH;
                valptr = (uint8_t *)&cs_env[device_indx].tx_value;
            }
            break;

            case CS_IDX_TX_VALUE_CCC:
            {
                length = 2;
                valptr = (uint8_t *)&cs_env[device_indx].tx_cccd_value;
            }
            break;

            case CS_IDX_TX_VALUE_USR_DSCP:
            {
                length = strlen(CS_TX_CHARACTERISTIC_NAME);
                valptr = (uint8_t *)CS_TX_CHARACTERISTIC_NAME;
            }
            break;

            /* RX long characteristic*/
            case CS_IDX_RX_LONG_VALUE_VAL:
            {
                length = CS_RX_LONG_VALUE_MAX_LENGTH;
                valptr = (uint8_t *)&cs_env[device_indx].rx_long_value;
            }
            break;

            case CS_IDX_RX_LONG_VALUE_USR_DSCP:
            {
                length = strlen(CS_RX_LONG_CHARACTERISTIC_NAME);
                valptr = (uint8_t *)CS_RX_LONG_CHARACTERISTIC_NAME;
            }
            break;

            /* TX long characteristic */
            case CS_IDX_TX_LONG_VALUE_VAL:
            {
                length = CS_TX_LONG_VALUE_MAX_LENGTH;
                valptr = (uint8_t *)&cs_env[device_indx].tx_long_value;
            }
            break;

            case CS_IDX_TX_LONG_VALUE_USR_DSCP:
            {
                length = strlen(CS_TX_LONG_CHARACTERISTIC_NAME);
                valptr = (uint8_t *)CS_TX_LONG_CHARACTERISTIC_NAME;
            }
            break;

            default:
            {
                status = ATT_ERR_READ_NOT_PERMITTED;
            }
            break;
        }
    }

    /* Allocate and build message */
    cfm = KE_MSG_ALLOC_DYN(GATTC_READ_CFM,
                           KE_BUILD_ID(TASK_GATTC, ble_env[device_indx].conidx),
                           TASK_APP,
                           gattc_read_cfm, length);

    if (valptr != NULL)
    {
        memcpy(cfm->value, valptr, length);
    }

    cfm->handle = param->handle;
    cfm->length = length;
    cfm->status = status;

    /* Send the message */
    ke_msg_send(cfm);

    return (KE_MSG_CONSUMED);
}

/* ----------------------------------------------------------------------------
 * Function      : int GATTC_WriteReqInd(ke_msg_id_t const msg_id,
 *                                       struct gattc_write_req_ind
 *                                       const *param,
 *                                       ke_task_id_t const dest_id,
 *                                       ke_task_id_t const src_id)
 * ----------------------------------------------------------------------------
 * Description   : Handle received write request indication
 *                 from a GATT Controller
 * Inputs        : - msg_id     - Kernel message ID number
 *                 - param      - Message parameters in format of
 *                                struct gattc_write_req_ind
 *                 - dest_id    - Destination task ID number
 *                 - src_id     - Source task ID number
 * Outputs       : return value - Indicate if the message was consumed;
 *                                compare with KE_MSG_CONSUMED
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
int GATTC_WriteReqInd(ke_msg_id_t const msg_id,
                      struct gattc_write_req_ind const *param,
                      ke_task_id_t const dest_id,
                      ke_task_id_t const src_id)
{
    /* Retrieve the index of environment structure representing peer device */
    signed int device_indx = Find_Connected_Device_Index(KE_IDX_GET(src_id));

    if (device_indx == INVALID_DEV_IDX)
    {
        return (KE_MSG_CONSUMED);
    }

    struct gattc_write_cfm *cfm =
        KE_MSG_ALLOC(GATTC_WRITE_CFM,
                     KE_BUILD_ID(TASK_GATTC, ble_env[device_indx].conidx),
                     TASK_APP, gattc_write_cfm);

    uint8_t status = GAP_ERR_NO_ERROR;
    uint16_t attnum;
    uint8_t *valptr = NULL;

    /* Check that offset is not zero */
    if (param->offset)
    {
        status = ATT_ERR_INVALID_OFFSET;
    }
  //  PRINTF("\r\n WriteReq:handle: %d %d %d %d %d\r\n",param->handle,param->value[0],param->value[1],param->value[2],param->value[3]);
    /* Set the attribute handle using the attribute index
     * in the custom service */
    if (param->handle > cs_env[device_indx].start_hdl)
    {
        attnum = (param->handle - cs_env[device_indx].start_hdl - 1);
    }
    else
    {
        status = ATT_ERR_INVALID_HANDLE;
    }

    /* If there is no error, save the requested attribute value */
    if (status == GAP_ERR_NO_ERROR)
    {
        switch (attnum)
        {
            case CS_IDX_RX_VALUE_VAL:
            {
                valptr = (uint8_t *)&cs_env[device_indx].rx_value;

                cs_env[device_indx].rx_value_changed = 1;


            }
            break;

            case CS_IDX_RX_VALUE_CCC:
            {
                valptr = (uint8_t *)&cs_env[device_indx].rx_cccd_value;
            }
            break;

            case CS_IDX_TX_VALUE_CCC:
            {
                valptr = (uint8_t *)&cs_env[device_indx].tx_cccd_value;
            }
            break;

            case CS_IDX_RX_LONG_VALUE_VAL:
            {
                valptr = (uint8_t *)&cs_env[device_indx].rx_long_value;
                cs_env[device_indx].rx_long_value_changed = 1;
            }
            break;

            default:
            {
                status = ATT_ERR_WRITE_NOT_PERMITTED;
            }
            break;
        }


    }

    if (valptr != NULL)
    {
        memcpy(valptr, param->value, param->length);

    }

    cfm->handle = param->handle;
    cfm->status = status;

    /* Send the message */
    ke_msg_send(cfm);

    if (cs_env[device_indx].rx_value_changed ) {

    	Update_SMData_RX(valptr,param->length);

    	if (cs_env[0].rx_value[0]!=0xAA)
    		memcpy(cs_env[0].arr_params,cs_env[0].rx_value,100);
    }

    return (KE_MSG_CONSUMED);
}

/* ----------------------------------------------------------------------------
 * Function      : void CustomService_SendNotification(uint8_t conidx,
 *                               uint8_t attidx, uint8_t *value, uint8_t length)
 * ----------------------------------------------------------------------------
 * Description   : Send a notification to the client device
 * Inputs        : - conidx       - connection index
 *                 - attidx       - index to attributes in the service
 *                 - value        - pointer to value
 *                 - length       - length of value
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
void CustomService_SendNotification(uint8_t conidx, uint8_t attidx,
                                    uint8_t *value, uint8_t length)
{
    struct gattc_send_evt_cmd *cmd;

    /* Retrieve the index of environment structure representing peer device */
    signed int device_indx = Find_Connected_Device_Index(conidx);

    if (device_indx == INVALID_DEV_IDX)
	{
		return;
	}

    uint16_t handle = (attidx + cs_env[device_indx].start_hdl + 1);



    /* Prepare a notification message for the specified attribute */
    cmd = KE_MSG_ALLOC_DYN(GATTC_SEND_EVT_CMD,
                           KE_BUILD_ID(TASK_GATTC, conidx), TASK_APP,
                           gattc_send_evt_cmd,
                           length * sizeof(uint8_t));
    cmd->handle = handle;
    cmd->length = length;
    cmd->operation = GATTC_NOTIFY;
    cmd->seq_num = 0;
    memcpy(cmd->value, value, length);

    cs_env[device_indx].sent_success = false;

    /* Send the message */
    ke_msg_send(cmd);
}

/* ----------------------------------------------------------------------------
 * Function      : int GATTC_CmpEvt(ke_msg_id_t const msg_id,
 *                                  struct gattc_cmp_evt
 *                                  const *param,
 *                                  ke_task_id_t const dest_id,
 *                                  ke_task_id_t const src_id)
 * ----------------------------------------------------------------------------
 * Description   : Handle received GATT controller complete event
 * Inputs        : - msg_id     - Kernel message ID number
 *                 - param      - Message parameters in format of
 *                                struct gattc_cmp_evt
 *                 - dest_id    - Destination task ID number
 *                 - src_id     - Source task ID number
 * Outputs       : return value - Indicate if the message was consumed;
 *                                compare with KE_MSG_CONSUMED
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
int GATTC_CmpEvt(ke_msg_id_t const msg_id,
                 struct gattc_cmp_evt const *param,
                 ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    /* Retrieve the index of environment structure representing peer device */
    signed int device_indx = Find_Connected_Device_Index(KE_IDX_GET(src_id));

    if (device_indx == INVALID_DEV_IDX)
    {
        return (KE_MSG_CONSUMED);
    }

    if (param->operation == GATTC_NOTIFY)
    {
        if (param->status == GAP_ERR_NO_ERROR ||
            param->status == GAP_ERR_DISCONNECTED)
        {
            cs_env[device_indx].sent_success = true;
        }
    }

    return (KE_MSG_CONSUMED);
}

/* ----------------------------------------------------------------------------
 * Function      : int GATTC_AttInfoReqInd(
 *                                  ke_msg_id_t const msg_id,
 *                                  struct gattc_read_req_ind const *param,
 *                                  ke_task_id_t const dest_id,
 *                                  ke_task_id_t const src_id)
 * ----------------------------------------------------------------------------
 * Description   : Request Attribute info to upper layer
 *                 could be trigger during prepare write to check if attribute
 *                 modification is authorized by profile/application or not and
 *                 to get current attribute length
 * Inputs        : - msg_id     - Kernel message ID number
 *                 - param      - Message parameters in format of
 *                                struct gattc_read_req_ind
 *                 - dest_id    - Destination task ID number
 *                 - src_id     - Source task ID number
 * Outputs       : return value - Indicate if the message was consumed;
 *                                compare with KE_MSG_CONSUMED
 * Assumptions   : None
 * ------------------------------------------------------------------------- */

int GATTC_AttInfoReqInd(ke_msg_id_t const msg_id,
                        struct gattc_read_req_ind const *param,
                        ke_task_id_t const dest_id,
                        ke_task_id_t const src_id)
{
    signed int device_indx = Find_Connected_Device_Index(KE_IDX_GET(src_id));
    if (device_indx == INVALID_DEV_IDX)
    {
        return (KE_MSG_CONSUMED);
    }

    uint16_t attnum;
    attnum = (param->handle - cs_env[device_indx].start_hdl - 1);

    /* Attribute Information confirmation message to inform if peer
     * device is authorized to modify attribute value, and to get current
     * attribute length.
     */
    struct gattc_att_info_cfm *cfm =
        KE_MSG_ALLOC(GATTC_ATT_INFO_CFM,
                     KE_BUILD_ID(TASK_GATTC, ble_env[device_indx].conidx),
                     TASK_APP, gattc_att_info_cfm);

    if (attnum == CS_IDX_RX_VALUE_VAL ||
        attnum == CS_IDX_RX_LONG_VALUE_VAL)
    {
        cfm->handle = param->handle;
        cfm->length = cs_atts_len[attnum];
        cfm->status = GAP_ERR_NO_ERROR;

        /* Send the message */
        ke_msg_send(cfm);
    }

    return (KE_MSG_CONSUMED);
}



void App_Env_Initialize(void)
{
    /* Reset the application manager environment */
    memset(&app_env, 0, sizeof(app_env));

    app_env.batt_lvl_low1 = 100;

    /* Create the application task handler */
    ke_task_create(TASK_APP, &TASK_DESC_APP);

    /* Initialize the custom service environment */
    CustomService_Env_Initialize();


}

int Minmax(int val ,int min,int max) {
	if (val >max) val = max;
	if (val < min) val = min;
	return val;

}

void  Update_SMData_RX(uint8_t* valptr, uint16_t lenData) {
	if (valptr[0] == 0xAA) {
		Update_ShortSMData_RX(valptr,lenData);
		return ;
	}


	 uint8_t  result_xor = valptr[1]  ^ valptr[2];
	 result_xor = result_xor ^valptr[3];
	 result_xor = result_xor ^ 0xDE;


	 uint8_t mem_idx = valptr[0];

	 MCU_VOLUME.Volume = 0 - valptr[1];


	 uint8_t wdrc_mask =valptr[3];
	 cs_env[0].wdrc_mask = wdrc_mask;



	 int base_offset = 20;
	//WDRC  这里6*8=48

	for (uint8_t band_idx=0;band_idx <8;band_idx++)
		MCU_WDRC.exp_cr[band_idx] = 0.1f * valptr[base_offset++];

	for (uint8_t band_idx=0;band_idx <8;band_idx++)
		MCU_WDRC.exp_end_knee[band_idx] = 1.0f * valptr[base_offset++];

	for (uint8_t band_idx=0;band_idx <8;band_idx++)
		MCU_WDRC.tkgain[band_idx] = 1.0f * valptr[base_offset++];

	for (uint8_t band_idx=0;band_idx <8;band_idx++)
		MCU_WDRC.tk[band_idx] = 1.0f * valptr[base_offset++];

	for (uint8_t band_idx=0;band_idx <8;band_idx++)
		MCU_WDRC.cr[band_idx] = 0.1f * valptr[base_offset++];

	for (uint8_t band_idx=0;band_idx <8;band_idx++)
		MCU_WDRC.bolt[band_idx] = 1.0f * valptr[base_offset++];



	//EQ 以后再说
	base_offset = 80;
	for (uint8_t band_idx=0;band_idx <8;band_idx++)
			MCU_EQ.dB_Gain_float[band_idx] = 0.0f - valptr[base_offset++] ;

	//不要再这里进行浮点运算，我们放到while()



}


void  Update_ShortSMData_RX(uint8_t* valptr, uint16_t lenData) {
	if (valptr[0] != 0xAA) {
		return ;
	}
	uint8_t  val1 = valptr[1];
	uint8_t  val2 = valptr[2];
	uint8_t  val3 = valptr[3];
	if (val1 == 0) {

		uint8_t mem_idx = val3;
		if ((mem_idx >=0 ) && (mem_idx <4)) {
			 for (int i=0;i<100;i++)
					cs_env[0].arr_params[i] = cs_env[0].arr_flash_params[128*mem_idx+i];

			 //确保第一个字节不是0xAA,否则会陷入循环
			 cs_env[0].arr_params[0] = mem_idx;
			 Update_SMData_RX(cs_env[0].arr_params, 100);

		}


	}
	if (val1 >0 && val1 <100){

		uint8_t ilen = val2;
		uint8_t start_offset=val1;
		if (ilen ==0)  ilen =1;
		if (ilen <=10) {
			 memcpy(&cs_env[0].arr_params[start_offset],&valptr[3],ilen);
			 Update_SMData_RX(cs_env[0].arr_params, 100);

		}



	}

	//0XAA AB 03 00 [3 个byte] 表示纯音测听
	if (val1 ==0xAB && val2 ==3) {
		uint8_t val4 = valptr[4];
		uint8_t val5 = valptr[5];
		uint8_t val6 = valptr[6];
		int freq = val4*256 +val5;
		uint8_t gain = val6;
		app_env.playtone_freq = freq;
		app_env.playtone_gain = 0- gain;




	}



}

void Readfrom_SmData_Buffer(uint8_t*  valptr) {

	valptr[0] = cs_env[0].arr_params[0];

	valptr[1] = (uint8_t) (0 -MCU_VOLUME.Volume);

	valptr[2] = 0x0; ;

	valptr[3] = 0x0;
	if(SM_Ptr->Control&MASK16(WDRC)) valptr[3] |= 0x10;
	if(SM_Ptr->Control&MASK16(EQ)) valptr[3] |= 0x8;
	if(SM_Ptr->Control&MASK16(AFC)) valptr[3] |= 0x4;


	//valptr[3..19]  空下来
	int base_offset = 20;
	//WDRC  这里6*8=48
	for (uint8_t band_idx=0;band_idx <8;band_idx++)
		valptr[base_offset++] = (uint8_t)(MCU_WDRC.exp_cr[band_idx]*10);
	for (uint8_t band_idx=0;band_idx <8;band_idx++)
		valptr[base_offset++] = (uint8_t)(MCU_WDRC.exp_end_knee[band_idx]*1);
	for (uint8_t band_idx=0;band_idx <8;band_idx++)
		valptr[base_offset++] = (uint8_t)(MCU_WDRC.tkgain[band_idx]*1);
	for (uint8_t band_idx=0;band_idx <8;band_idx++)
		valptr[base_offset++] = (uint8_t)(MCU_WDRC.tk[band_idx]*1);
	for (uint8_t band_idx=0;band_idx <8;band_idx++)
		valptr[base_offset++] = (uint8_t)(MCU_WDRC.cr[band_idx]*10);
	for (uint8_t band_idx=0;band_idx <8;band_idx++)
		valptr[base_offset++] = (uint8_t)(MCU_WDRC.bolt[band_idx]*1);

	//valptr[68..79] 空下来

	base_offset = 80;
	for (uint8_t band_idx=0;band_idx <8;band_idx++)
			valptr[base_offset++] = (uint8_t)(0-MCU_EQ.dB_Gain_float[band_idx]);



}
#endif

