/*
 * ESPRSSIF MIT License
 *
 * Copyright (c) 2015 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on ESPRESSIF SYSTEMS ESP8266 only, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "SysDefines.h"


/******************************************************************************
 * FunctionName : user_rf_cal_sector_set
 * Description  : SDK just reversed 4 sectors, used for rf init data and paramters.
 *                We add this function to force users to set rf cal sector, since
 *                we don't know which sector is free in user's application.
 *                sector map for last several sectors : ABCCC
 *                A : rf cal
 *                B : rf init data
 *                C : sdk parameters
 * Parameters   : none
 * Returns      : rf cal sector
*******************************************************************************/
uint32 user_rf_cal_sector_set(void)
{
    flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;

    switch (size_map) {
        case FLASH_SIZE_4M_MAP_256_256:
            rf_cal_sec = 128 - 5;
            break;

        case FLASH_SIZE_8M_MAP_512_512:
            rf_cal_sec = 256 - 5;
            break;

        case FLASH_SIZE_16M_MAP_512_512:
        case FLASH_SIZE_16M_MAP_1024_1024:
            rf_cal_sec = 512 - 5;
            break;

        case FLASH_SIZE_32M_MAP_512_512:
        case FLASH_SIZE_32M_MAP_1024_1024:
            rf_cal_sec = 1024 - 5;
            break;

        default:
            rf_cal_sec = 0;
            break;
    }

    return rf_cal_sec;
} 

extern void StartSys_Task(void *pvParameters);
void user_init(void)
{
	uart_init_new();
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0TXD_U,FUNC_GPIO1);//此处废弃tx0，可以避免更多系统打印输出，当作普通gpio
	UART_SetPrintPort(UART1);//将系统默认的打印信息转移到com1

	Debug("\033[2J\n\r\n\r\n\r");
	Debug("System Reset Info:%d\n\r",system_get_rst_info()->reason);
	Debug("QWIFI START\n\rSDK version:%s\n\r", system_get_sdk_version());   
	Debug("ESP8266 CHIP ID:%u(0x%x), Random:0x%x\n\r",GetHwID(),GetHwID(),os_random()); 
	Debug("FLASH:0x%x, SizeMap:0x%x\n\r",spi_flash_get_id(),system_get_flash_size_map()); 

	system_deep_sleep_set_option(1);//唤醒后重新上电会进行RF_CAL
	wifi_station_ap_number_set(ESP_AP_RECORD_NUM);
	//wifi_station_set_reconnect_policy(TRUE);//设置是否掉线自动重连，默认就是true
	//wifi_station_set_auto_connect(TRUE);//设置是否开机自动连接，默认就是true
    wifi_set_opmode(STATIONAP_MODE);
	OS_TaskCreate(StartSys_Task, "StartSys Task",TASK_STK_SIZE_NORMAL,NULL,TASK_PRIO_MAX,NULL); 
}

