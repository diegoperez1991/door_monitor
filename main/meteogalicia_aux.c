#ifndef METEOGALICIA_AUX_c
#define METEOGALICIA_AUX_c

#include "esp_http_client.h"
#include "global_variables_aux.c"


#include "esp_log.h"
#include "string.h"
#include "freertos/task.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

#include <mbedtls/platform.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/debug.h>
#include <mbedtls/entropy.h>
#include <mbedtls/error.h>
#include <mbedtls/net.h>
#include <mbedtls/ssl.h>
#include <esp_log.h>

// servizos.meteogalicia.gal/rss/observacion/observacionConcellos.action
const char * pcURLMeteogalicia = "http://servizos.meteogalicia.gal/rss/observacion/observacionConcellos.action";// "http://httpbin.org/redirect/2";

/*
 * meteogalicia_aux.c
 *
 *  Created on: 6 ago. 2018
 *      Author: user
 */

char *pcStrTok;
char *pcStrstr;

esp_err_t _http_event_handle(esp_http_client_event_t *evt)
	{
		static const char* TAG = "tMeteo";

	    switch(evt->event_id) {
	        case HTTP_EVENT_ERROR:
	            ESP_LOGI(TAG, "HTTP_EVENT_ERROR");
	            break;
	        case HTTP_EVENT_ON_CONNECTED:
	            ESP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED");
	            break;
	        case HTTP_EVENT_HEADER_SENT:
	            ESP_LOGI(TAG, "HTTP_EVENT_HEADER_SENT");
	            break;
	        case HTTP_EVENT_ON_HEADER:
	            ESP_LOGI(TAG, "HTTP_EVENT_ON_HEADER");
	            printf("%.*s", evt->data_len, (char*)evt->data);
	            break;
	        case HTTP_EVENT_ON_DATA:
	            ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
	            if (!esp_http_client_is_chunked_response(evt->client)) {

	            	pcStrstr = strstr((char*)evt->data,(char*)"Arbo");
	            	if (pcStrstr != NULL ){
	            		pcStrTok = strtok(pcStrstr, ":,}");
	            		pcStrTok = strtok(NULL, ":,}");
	            		pcStrTok = strtok(NULL, ":,}");
	            		if (pcStrTok != NULL ){
	            			printf("Sensacion termica Arbo = %s", pcStrTok);
	            			strcpy(pcSensArbo,pcStrTok);
	            		}
	            		else{
	            			printf("Sensacion termica  Arbo = NULL");
	            		}
	            		pcStrTok = strtok(NULL, ":,}");
	            		pcStrTok = strtok(NULL, ":,}");
	            		if (pcStrTok != NULL ){
	            			printf("Temperatura Arbo = %s", pcStrTok);
	            			strcpy(pcTempArbo,pcStrTok);
	            		}
	            		else{
	            			printf("Temperatura Arbo = NULL");
	            		}
	            	}
	            	pcStrstr = strstr((char*)evt->data,(char*)"Vigo");
	            	if (pcStrstr != NULL ){
	            		pcStrTok = strtok(pcStrstr, ":,}");
	            		pcStrTok = strtok(NULL, ":,}");
	            		pcStrTok = strtok(NULL, ":,}");
	            		if (pcStrTok != NULL ){
	            			printf("Sensacion termica Vigo = %s", pcStrTok);
	            			strcpy(pcSensVigo,pcStrTok);
	            		}
	            		else{
	            			printf("Sensacion termica  Vigo = NULL");
	            		}
	            		pcStrTok = strtok(NULL, ":,}");
	            		pcStrTok = strtok(NULL, ":,}");
	            		if (pcStrTok != NULL ){
	            			printf("Temperatura Vigo = %s", pcStrTok);
	            			strcpy(pcTempVigo,pcStrTok);
	            		}
	            		else{
	            			printf("Temperatura Vigo = NULL");
	            		}
	            	}
	                //printf("%.*s", evt->data_len, (char*)evt->data);
	            }

	            break;
	        case HTTP_EVENT_ON_FINISH:
	            ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH");
	            break;
	        case HTTP_EVENT_DISCONNECTED:
	            ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
	            break;
	    }
	    return ESP_OK;
	}



static void my_debug(void *ctx, int level, const char *file, int line, const char *str) {
	((void) level);
	((void) ctx);
	printf("%s:%04d: %s", file, line, str);
}


void vTaskMeteogalicia( void *pvParameters ){

	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount ();

	static const char* TAG = "tMeteo";

	//mbedtls_esp_enable_debug_log();

	// Cliente Meteogalicia
	esp_http_client_config_t configMeteo = {	};
    //strcpy((char*)config.url, pcURLMeteogalicia);
    configMeteo.url = pcURLMeteogalicia;
    configMeteo.event_handler = _http_event_handle;
	esp_http_client_handle_t clientMeteo = esp_http_client_init(&configMeteo);


	while( 1 ) {

		xSemaphoreTake(xMyMutexHttp,portMAX_DELAY);
		esp_err_t err = esp_http_client_perform(clientMeteo);
		xSemaphoreGive(xMyMutexHttp);

		if (err == ESP_OK) {
			ESP_LOGI(TAG, "Status = %d, content_length = %d",
					esp_http_client_get_status_code(clientMeteo),
					esp_http_client_get_content_length(clientMeteo));
		}

		vTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS( 60000 ) );
	}
	//esp_http_client_cleanup(clientMeteo);
}



#endif
