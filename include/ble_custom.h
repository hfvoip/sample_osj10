/*----------------------------------------------------------------------------
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
 * ble_custom.h
 * - Bluetooth custom service header
 * ----------------------------------------------------------------------------
 * $Revision: 1.25 $
 * $Date: 2018/02/27 15:42:16 $
 * ------------------------------------------------------------------------- */

#ifndef BLE_CUSTOM_H
#define BLE_CUSTOM_H

/* ----------------------------------------------------------------------------
 * If building with a C++ compiler, make all of the definitions in this header
 * have a C binding.
 * ------------------------------------------------------------------------- */
#ifdef __cplusplus
extern "C"
{
#endif    /* ifdef __cplusplus */

/* ----------------------------------------------------------------------------
 * Include files
 * --------------------------------------------------------------------------*/

/* ----------------------------------------------------------------------------
 * Defines
 * --------------------------------------------------------------------------*/

/* Custom service UUIDs */
#define CS_SVC_UUID                     { 0x24, 0xdc, 0x0e, 0x6e, 0x01, 0x40, \
                                          0xca, 0x9e, 0xe5, 0xa9, 0xa3, 0x00, \
                                          0xb5, 0xf3, 0x93, 0xe0 }
#define CS_CHARACTERISTIC_TX_UUID       { 0x24, 0xdc, 0x0e, 0x6e, 0x02, 0x40, \
                                          0xca, 0x9e, 0xe5, 0xa9, 0xa3, 0x00, \
                                          0xb5, 0xf3, 0x93, 0xe0 }
#define CS_CHARACTERISTIC_RX_UUID       { 0x24, 0xdc, 0x0e, 0x6e, 0x03, 0x40, \
                                          0xca, 0x9e, 0xe5, 0xa9, 0xa3, 0x00, \
                                          0xb5, 0xf3, 0x93, 0xe0 }
#define CS_CHARACTERISTIC_TX_LONG_UUID  { 0x24, 0xdc, 0x0e, 0x6e, 0x04, 0x40, \
                                          0xca, 0x9e, 0xe5, 0xa9, 0xa3, 0x00, \
                                          0xb5, 0xf3, 0x93, 0xe0 }
#define CS_CHARACTERISTIC_RX_LONG_UUID  { 0x24, 0xdc, 0x0e, 0x6e, 0x05, 0x40, \
                                          0xca, 0x9e, 0xe5, 0xa9, 0xa3, 0x00, \
                                          0xb5, 0xf3, 0x93, 0xe0 }

#define ATT_DECL_CHAR() \
    { ATT_DECL_CHARACTERISTIC_128, PERM(RD, ENABLE), 0, 0 }
#define ATT_DECL_CHAR_UUID_16(uuid, perm, max_length) \
    { uuid, perm, max_length, PERM(RI, ENABLE) | PERM(UUID_LEN, UUID_16) }
#define ATT_DECL_CHAR_UUID_32(uuid, perm, max_length) \
    { uuid, perm, max_length, PERM(RI, ENABLE) | PERM(UUID_LEN, UUID_32) }
#define ATT_DECL_CHAR_UUID_128(uuid, perm, max_length) \
    { uuid, perm, max_length, PERM(RI, ENABLE) | PERM(UUID_LEN, UUID_128) }
#define ATT_DECL_CHAR_CCC()                                                     \
    { ATT_DESC_CLIENT_CHAR_CFG_128, PERM(RD, ENABLE) | PERM(WRITE_REQ, ENABLE), \
      0, PERM(RI, ENABLE) }
#define ATT_DECL_CHAR_USER_DESC(max_length)                      \
    { ATT_DESC_CHAR_USER_DESC_128, PERM(RD, ENABLE), max_length, \
      PERM(RI, ENABLE) }

enum cs_idx_att
{
    /* TX Characteristic */
    CS_IDX_TX_VALUE_CHAR,
    CS_IDX_TX_VALUE_VAL,
    CS_IDX_TX_VALUE_CCC,
    CS_IDX_TX_VALUE_USR_DSCP,

    /* RX Characteristic */
    CS_IDX_RX_VALUE_CHAR,
    CS_IDX_RX_VALUE_VAL,
    CS_IDX_RX_VALUE_CCC,
    CS_IDX_RX_VALUE_USR_DSCP,

    /* TX Long Characteristic */
    CS_IDX_TX_LONG_VALUE_CHAR,
    CS_IDX_TX_LONG_VALUE_VAL,
    CS_IDX_TX_LONG_VALUE_USR_DSCP,

    /* RX Long Characteristic */
    CS_IDX_RX_LONG_VALUE_CHAR,
    CS_IDX_RX_LONG_VALUE_VAL,
    CS_IDX_RX_LONG_VALUE_USR_DSCP,

    /* Max number of characteristics */
    CS_IDX_NB,
};

#define CS_TX_VALUE_MAX_LENGTH          20
#define CS_RX_VALUE_MAX_LENGTH          100
#define CS_USER_DESCRIPTION_MAX_LENGTH  16
#define CS_TX_LONG_VALUE_MAX_LENGTH     40
#define CS_RX_LONG_VALUE_MAX_LENGTH     40

#define CS_TX_CHARACTERISTIC_NAME       "TX_VALUE"
#define CS_RX_CHARACTERISTIC_NAME       "J11 RX_VALUE"
#define CS_TX_LONG_CHARACTERISTIC_NAME  "TX_LONG_VALUE"
#define CS_RX_LONG_CHARACTERISTIC_NAME  "RX_LONG_VALUE"

/* List of message handlers that are used by the custom service
 * application manager */
#define CS_MESSAGE_HANDLER_LIST                                     \
    DEFINE_MESSAGE_HANDLER(GATTC_READ_REQ_IND, GATTC_ReadReqInd),   \
    DEFINE_MESSAGE_HANDLER(GATTC_WRITE_REQ_IND, GATTC_WriteReqInd), \
    DEFINE_MESSAGE_HANDLER(GATTM_ADD_SVC_RSP, GATTM_AddSvcRsp),     \
    DEFINE_MESSAGE_HANDLER(GATTC_CMP_EVT, GATTC_CmpEvt),            \
    DEFINE_MESSAGE_HANDLER(GATTC_ATT_INFO_REQ_IND, GATTC_AttInfoReqInd)

/* Define the available custom service states */
enum cs_state
{
    CS_INIT,
    CS_SERVICE_DISCOVERD,
    CS_ALL_ATTS_DISCOVERED,
    CS_STATE_MAX
};

/* ----------------------------------------------------------------------------
 * Global variables and types
 * --------------------------------------------------------------------------*/

struct cs_env_tag
{
    /* The value of service handle in the database of attributes in the stack */
    uint16_t start_hdl;

    /* The value of TX characteristic value */
    uint8_t tx_value[CS_TX_VALUE_MAX_LENGTH];

    /* CCCD value of TX characteristic */
    uint16_t tx_cccd_value;

    /* A flag that indicates that TX value has been changed */
    bool tx_value_changed;

    /* A flag that indicates that PDU has been sent over the air */
    bool sent_success;
    int8_t  j11dump_flag ;

    /* The value of RX characteristic value */
    uint8_t rx_value[CS_RX_VALUE_MAX_LENGTH];
    uint8_t arr_params[CS_RX_VALUE_MAX_LENGTH];
    uint8_t arr_flash_params[800];


    /* CCCD value of RX characteristic */
    uint16_t rx_cccd_value;

    uint8_t tx_long_value[CS_TX_LONG_VALUE_MAX_LENGTH];

    uint8_t rx_long_value[CS_RX_LONG_VALUE_MAX_LENGTH];

    /* A flag that indicates that RX value has been changed, to be used by
     * application */
    bool rx_value_changed;

    bool rx_long_value_changed;
    /* The state machine for service discovery
     * (it is not used for server role) */
    uint8_t state;
    uint8_t  wdrc_mask;

    /* Custom service */
    uint16_t cnt_notifc;
    uint8_t val_notif;
};

extern struct cs_env_tag cs_env[];

/* ----------------------------------------------------------------------------
 * Function prototype definitions
 * --------------------------------------------------------------------------*/
extern void CustomService_Env_Initialize(void);

extern void CustomService_ServiceAdd(void);

extern int GATTM_AddSvcRsp(ke_msg_id_t const msgid,
                           struct gattm_add_svc_rsp const *param,
                           ke_task_id_t const dest_id,
                           ke_task_id_t const src_id);

extern int GATTC_ReadReqInd(ke_msg_id_t const msg_id,
                            struct gattc_read_req_ind const *param,
                            ke_task_id_t const dest_id,
                            ke_task_id_t const src_id);

extern int GATTC_WriteReqInd(ke_msg_id_t const msg_id,
                             struct gattc_write_req_ind const *param,
                             ke_task_id_t const dest_id,
                             ke_task_id_t const src_id);

extern void CustomService_SendNotification(uint8_t conidx, uint8_t attidx,
                                           uint8_t *value, uint8_t length);

extern int GATTC_CmpEvt(ke_msg_id_t const msg_id,
                        struct gattc_cmp_evt const *param,
                        ke_task_id_t const dest_id,
                        ke_task_id_t const src_id);

extern int GATTC_AttInfoReqInd(ke_msg_id_t const msg_id,
                               struct gattc_read_req_ind const *param,
                               ke_task_id_t const dest_id,
                               ke_task_id_t const src_id);

extern void  Update_SMData_RX(uint8_t* cmd, uint16_t lenData) ;
extern void  Update_ShortSMData_RX(uint8_t* cmd, uint16_t lenData) ;


/* ----------------------------------------------------------------------------
 * Close the 'extern "C"' block
 * ------------------------------------------------------------------------- */
#ifdef __cplusplus
}
#endif    /* ifdef __cplusplus */

#endif    /* BLE_CUSTOM_H */
