/* ----------------------------------------------------------------------------
 * Copyright (c) 2015-2017 Semiconductor Components Industries, LLC (d/b/a
 * ON Semiconductor), All Rights Reserved
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 * This module is derived in part from example code provided by RivieraWaves
 * and as such the underlying code is the property of RivieraWaves [a member
 * of the CEVA, Inc. group of companies], together with additional code which
 * is the property of ON Semiconductor. The code (in whole or any part) may not
 * be redistributed in any form without prior written permission from
 * ON Semiconductor.
 *
 * The terms of use and warranty for this code are covered by contractual
 * agreements between ON Semiconductor and the licensee.
 *
 * This is Reusable Code.
 *
 * ----------------------------------------------------------------------------
 * app_uart.c
 * - Application task handler definition and support processes
 * ----------------------------------------------------------------------------
 * $Revision: 1.20 $
 * $Date: 2018/02/27 15:42:17 $
 * ------------------------------------------------------------------------- */

#include "app.h"
#include "printf.h"
#if 0
/* Global variables */
ARM_DRIVER_USART *uart;
DRIVER_GPIO_t *gpio;

/* Global variables */
ARM_DRIVER_USART *uart;
DRIVER_GPIO_t *gpio;
char tx_buffer[] __attribute__ ((aligned(4))) = "67";
char rx_buffer[sizeof(tx_buffer)] __attribute__ ((aligned(4)));



/* ----------------------------------------------------------------------------
 * Function      : void Button_EventCallback(void)
 * ----------------------------------------------------------------------------
 * Description   : This function is a callback registered by the function
 *                 Initialize. Based on event argument different actions are
 *                 executed.
 * Inputs        : None
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
void Usart_EventCallBack(uint32_t event)
{
    /* Check if receive complete event occured */
    if (event & ARM_USART_EVENT_RECEIVE_COMPLETE)
    {
        /* Check if received data matches sent tx */
        if (!strcmp(tx_buffer, rx_buffer))
        {
            /* Toggle led */
        	ToggleLed(2, 500);
            PRINTF("LED BLINKED: CORRECT_DATA_RECEIVED\n");
            uart->Send("ABCDE",6); /* start transmission */

        }

        /* Receive next data */
        uart->Receive(rx_buffer, sizeof(tx_buffer));
    }
}

void ToggleLed(uint32_t n, uint32_t delay_ms)
{
    for (; n > 0; n--)
    {
        /* Refresh the watchdog */
        Sys_Watchdog_Refresh();

        /* Toggle led diode */

        Sys_GPIO_Toggle(LED_DIO);


        /* Delay */
        Sys_Delay_ProgramROM((delay_ms / 1000.0) * SystemCoreClock);
    }
}
#endif
