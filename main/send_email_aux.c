#ifndef SEND_EMAIL_AUX_c
#define SEND_EMAIL_AUX_c

#include "esp_http_client.h"
#include "global_variables_aux.c"
#include "esp_log.h"
#include "string.h"
#include "freertos/task.h"

#define MAX_TIME_DOOR_OPEN 15000 // ms
#define GPIO_DEBOUNCE_TIME 1000 // ms

	const char * pcURLEmail = "https://api.smtp2go.com/v3/email/send";// "http://httpbin.org/redirect/2";
	int iSMTPport = 2525;

	char pcJSONEmail[500];
	char * pcJSONEmailPart1 = "{\r\n"
			"\"api_key\": \"api-F4BAB7D6A0D711E8A823F23C91BBF4A0\",\r\n"
			"\"to\": [\"Test Person <diegonetmail@gmail.com>\"],\r\n"
			"\"sender\": \"ESP32 <diegonetmail@gmail.com>\",\r\n"
			"\"subject\": \"";
	char * pcJSONEmailPart2 = "\",\r\n"
			"\"text_body\": \"";
	char * pcJSONEmailPart3 ="\"\r\n" // last line does not have a comma
			"}\r\n";

/*
 * meteogalicia_aux.c
 *
 *  Created on: 6 ago. 2018
 *      Author: user
 */

	esp_err_t _http_event_handle_email(esp_http_client_event_t *evt)
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

		            	printf("%.*s", evt->data_len, (char*)evt->data);
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

void vTaskSendEmail( void *pvParameters ){

	TickType_t xLastWakeTime;
	TickType_t xLastTimeDoorOpened = 0;
	xLastWakeTime = xTaskGetTickCount ();
	int iAvisoEnviado = 0;
	static const char* TAG = "tMeteo";
	uint32_t  buttonState = gpio_get_level(GPIO_INPUT_DOOR_SW); // the current reading from the input pin

	    while( 1 ) {

	    	uint32_t  reading = gpio_get_level(GPIO_INPUT_DOOR_SW);

	    	char pcEmailMsg[200];

	    	if (reading != buttonState) {
	    		buttonState = reading;

	    		// Cliente Email
	    		// 'Content-Type': "application/json"
	    		esp_http_client_config_t configEmail = {		};
	    		configEmail.url = pcURLEmail;
	    		configEmail.event_handler = _http_event_handle_email;
	    		configEmail.port = iSMTPport;
	    		configEmail.method = HTTP_METHOD_POST;
	    		configEmail.auth_type = HTTP_AUTH_TYPE_BASIC;
	    		configEmail.password = "bTJld205d2wxdTAw";
	    		configEmail.username = "miesp32";
	    		esp_http_client_handle_t clientEmail = esp_http_client_init(&configEmail);
	    		// POST
	    		esp_http_client_set_url(clientEmail, "https://api.smtp2go.com/v3/email/send");
	    		esp_http_client_set_method(clientEmail, HTTP_METHOD_POST);
	    		esp_http_client_set_header(clientEmail, "Content-Type", "application/json");

	    		if ( buttonState == 1 ){
	    			strcpy(pcEmailMsg,"Puerta ABIERTA, hora ");
	    			xLastTimeDoorOpened = xTaskGetTickCount ();
	    		} else {
	    			strcpy(pcEmailMsg,"Puerta CERRADA, hora ");
	    			iAvisoEnviado = 0;
	    		}
	    		strcat(pcEmailMsg,pcHora);
	    		strcat(pcEmailMsg,", fecha ");
	    		strcat(pcEmailMsg,pcFecha);

	    		strcpy(pcJSONEmail,pcJSONEmailPart1);
	    		strcat(pcJSONEmail,pcEmailMsg);
	    		strcat(pcJSONEmail,pcJSONEmailPart2);
	    		strcat(pcJSONEmail,pcEmailMsg);
	    		strcat(pcJSONEmail,pcJSONEmailPart3);

	    		ESP_LOGI(TAG, "Email = \r\n%s",pcJSONEmail);

	    		esp_http_client_set_post_field(clientEmail, pcJSONEmail, strlen(pcJSONEmail));

	    		xSemaphoreTake(xMyMutexHttp,portMAX_DELAY);
	    		esp_err_t err = esp_http_client_perform(clientEmail);
	    		xSemaphoreGive(xMyMutexHttp);
	    		if (err == ESP_OK) {
	    			ESP_LOGI(TAG, "Status = %d, content_length = %d",
	    					esp_http_client_get_status_code(clientEmail),
							esp_http_client_get_content_length(clientEmail));
	    		}
	    		else {
	    			ESP_LOGE(TAG, "HTTP POST request failed: %s", esp_err_to_name(err));
	    		}

	    		esp_http_client_cleanup(clientEmail);

	    	} else if ( (reading == 1) && ((xLastWakeTime - xLastTimeDoorOpened)>MAX_TIME_DOOR_OPEN) && (!iAvisoEnviado) ) {

	    		// Cliente Email
	    		// 'Content-Type': "application/json"
	    		esp_http_client_config_t configEmail = {		};
	    		configEmail.url = pcURLEmail;
	    		configEmail.event_handler = _http_event_handle_email;
	    		configEmail.port = iSMTPport;
	    		configEmail.method = HTTP_METHOD_POST;
	    		configEmail.auth_type = HTTP_AUTH_TYPE_BASIC;
	    		configEmail.password = "bTJld205d2wxdTAw";
	    		configEmail.username = "miesp32";
	    		esp_http_client_handle_t clientEmail = esp_http_client_init(&configEmail);
	    		// POST
	    		esp_http_client_set_url(clientEmail, "https://api.smtp2go.com/v3/email/send");
	    		esp_http_client_set_method(clientEmail, HTTP_METHOD_POST);
	    		esp_http_client_set_header(clientEmail, "Content-Type", "application/json");

	    		iAvisoEnviado = 1;
    			strcpy(pcEmailMsg,"Puerta ABIERTA durante mas de 1 MINUTO, hora ");
	    		strcat(pcEmailMsg,pcHora);
	    		strcat(pcEmailMsg,", fecha ");
	    		strcat(pcEmailMsg,pcFecha);

	    		strcpy(pcJSONEmail,pcJSONEmailPart1);
	    		strcat(pcJSONEmail,pcEmailMsg);
	    		strcat(pcJSONEmail,pcJSONEmailPart2);
	    		strcat(pcJSONEmail,pcEmailMsg);
	    		strcat(pcJSONEmail,pcJSONEmailPart3);

	    		ESP_LOGI(TAG, "Email = \r\n%s",pcJSONEmail);

	    		esp_http_client_set_post_field(clientEmail, pcJSONEmail, strlen(pcJSONEmail));

	    		xSemaphoreTake(xMyMutexHttp,portMAX_DELAY);
	    		esp_err_t err = esp_http_client_perform(clientEmail);
	    		xSemaphoreGive(xMyMutexHttp);
	    		if (err == ESP_OK) {
	    			ESP_LOGI(TAG, "Status = %d, content_length = %d",
	    					esp_http_client_get_status_code(clientEmail),
							esp_http_client_get_content_length(clientEmail));
	    		}
	    		else {
	    			ESP_LOGE(TAG, "HTTP POST request failed: %s", esp_err_to_name(err));
	    		}

	    		esp_http_client_cleanup(clientEmail);
	    	}



			vTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS( GPIO_DEBOUNCE_TIME ));


	


}
}



#endif
