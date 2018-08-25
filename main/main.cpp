// cd C:/Users/user/eclipse-workspace/myapp


// sntp
// https://github.com/espressif/esp-idf/blob/master/examples/protocols/sntp/main/sntp_example_main.c
#include "wifi_aux.c"
#include "display_aux.cpp"
#include "meteogalicia_aux.c"
#include "send_email_aux.c"
#include "global_variables_aux.c"

#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "driver/gpio.h"

#include "string.h"
#include "driver/uart.h"

//debugging stuff
#include "freertos/task.h"
char ptrTaskList[1000];
char pcTable[1000];
char pcTableRowSep[] = "*************************************\r\n";
char pcTableHeader[] = "Task     State   Prio    Stack    Num\r\n";
char pcTableHeader2[] = "Task    Time    Percent \r\n";

#include "freertos/timers.h"

#include <time.h>
#include <sys/time.h>
#include "lwip/err.h"
#include "apps/sntp/sntp.h"
#include "lwip/netdb.h"

#include "esp_log.h"

extern "C" {
void app_main();
}

void app_main(void)
{
	static const char* TAG = "tMain";
	//#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
	int level = 0;


	xMyMutexHttp = xSemaphoreCreateMutex();


	 xTaskCreate( vTaskDisplay,
         		"tDisplay",
     			10000,
     			NULL,
     			24,
     			NULL);

	//ESP_LOGI(TAG,  "Display task created, error = %i",  xError);

         nvs_flash_init();

         vTaskDelay(2000 / portTICK_PERIOD_MS);


         init_wifi();

   ip_addr_t addr;
   sntp_setoperatingmode(SNTP_OPMODE_POLL);
   inet_pton(AF_INET,"129.6.15.28",&addr);
   sntp_setserver(0,&addr);
   sntp_init();
   // NTP TZ setting    CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00
   setenv("TZ", "CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00", 1);

	ESP_LOGI(TAG,  "Waiting to create http client task" );
    vTaskDelay(5000 / portTICK_PERIOD_MS);

    gpio_config_t io_conf;
    //interrupt of rising edge
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //bit mask of the pins, use GPIO4/5 here
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    //set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    //enable pull-up mode
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&io_conf);

    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    //disable pull-down mode
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    //disable pull-up mode
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    //configure GPIO with the given settings
    gpio_config(&io_conf);


    BaseType_t xError = xTaskCreate( vTaskMeteogalicia,
    		"tMeteo",
			8192,
			NULL,
			10,
			NULL);

	ESP_LOGE(TAG,  "Meteogalicia task created, error: %s", esp_err_to_name(xError));


    xError = xTaskCreate( vTaskSendEmail,
    		"tEmail",
			8192,
			NULL,
			10,
			NULL);

	ESP_LOGE(TAG,  "SendEmail task created, error: %s", esp_err_to_name(xError));

    //gpio_set_direction(GPIO_NUM_16, GPIO_MODE_OUTPUT);




    while (true) {

        level = !level;
        gpio_set_level(GPIO_OUTPUT_LED, level);
        vTaskDelay(1000 / portTICK_PERIOD_MS);


		vTaskList(ptrTaskList);
		ESP_LOGI(TAG,  "%s", ptrTaskList );

//		// comp config -> freertos -> enable freertos 2 collect run time stats
//		vTaskGetRunTimeStats(ptrTaskList);
//		ESP_LOGI(TAG,  "%s", ptrTaskList );



    }
}

