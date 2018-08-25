#ifndef WIFI_AUX_c
#define WIFI_AUX_c

#define AP_SSID "MiWiFi" // CONFIG_WIFI_SSID
#define WIFI_LOGGING_INFO_RECEIVED 111

#include "string.h"
#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "driver/gpio.h"

#include "esp_log.h"

#define STORAGE_NAMESPACE "storage"

#include "global_variables_aux.c"
#include "esp_event.h"
#include "tcpip_adapter.h"
#include <esp_wifi.h>
#include <esp_event_loop.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>

#include <http_server.h>

static const char *TAG2="APP";



// NVS helper functions
// ---------------------------------------------------------------

/* Save new run time value in NVS
   by first reading a table of previously saved values
   and then adding the new value at the end of the table.
   Return an error if anything goes wrong
   during this process.
 */
esp_err_t save_wifi_login_info(void)
{

    nvs_handle my_handle;
    esp_err_t err;

    // Open
    err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) return err;


    // Write BSSID
    size_t required_size = strlen(pcBSSID)*sizeof(char);
    err = nvs_set_str(my_handle, "BSSID", pcBSSID);
    if (err != ESP_OK) return err;

    // Write Password
    required_size = strlen(pcPassword)*sizeof(char);
    err = nvs_set_str(my_handle, "PASSWORD", pcPassword);
    if (err != ESP_OK) return err;

    // Commit
    err = nvs_commit(my_handle);
    if (err != ESP_OK) return err;

    // Close
    nvs_close(my_handle);
    return ESP_OK;
}

/* Read from NVS and print restart counter
   and the table with run times.
   Return an error if anything goes wrong
   during this process.
 */
esp_err_t read_wifi_login_info(void)
{
    nvs_handle my_handle;
    esp_err_t err;
    char *pcAuxStr;

    // Open
    err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) return err;

    // Read BSSID
    size_t iBSSID_length = 0; // value will default to 0, if not set yet in NVS
    // Read the size of memory space required for blob
    err = nvs_get_str(my_handle, "BSSID", NULL, &iBSSID_length);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) return err;
    // esp_err_t = nvs_get_str(nvs_handlehandle, const char *key, char *out_value, size_t *length)
    pcAuxStr = (char *)malloc(iBSSID_length);
    if (iBSSID_length > 0) {
    	err = nvs_get_str(my_handle, "BSSID", pcAuxStr, &iBSSID_length);
    	if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) return err;
    }
    if (iBSSID_length == 0) {
        printf("Nothing saved yet!\n");
        strcpy(pcBSSID,"");
    } else {
        printf("BSSID = %s\n", pcAuxStr);
        strcpy(pcBSSID,pcAuxStr); //pcAuxStr
    }

    // Read password
    size_t iPassword_length;
    // Read the size of memory space required for blob
    err = nvs_get_str(my_handle, "PASSWORD", NULL, &iPassword_length);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) return err;
    // esp_err_t = nvs_get_str(nvs_handlehandle, const char *key, char *out_value, size_t *length)
    pcAuxStr = (char *)malloc(iPassword_length);
    if (iPassword_length > 0) {
    	err = nvs_get_str(my_handle, "PASSWORD", pcAuxStr, &iPassword_length);
    	if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) return err;
    }
    if (iPassword_length == 0) {
    	printf("Nothing saved yet!\n");
        strcpy(pcPassword,"");
    } else {
    	printf("Password = %s\n", pcAuxStr);
        strcpy(pcPassword,pcAuxStr);
    }


    // Close
    nvs_close(my_handle);
    return ESP_OK;
}


// SERVER helper functions
// ----------------------------------------------------------------

// wifi.html
char * pcHtml1Part1 = "<!DOCTYPE html>\r\n"
		"<html>\r\n"
		"<body>\r\n"
		"\r\n"
		"<h2>Configuraci&oacute;n del acceso a la Wi-Fi</h2>\r\n"
		"\r\n"
		"<form action=\"/guardado\">\r\n"
		"  Nombre de la red (SSID):<br>\r\n"
		"  <input type=\"text\" name=\"ssid\" value=\"\">\r\n"
		"  <br>\r\n"
		"  Contrase&ntilde;a:<br>\r\n"
		"  <input type=\"text\" name=\"password\" value=\"\">\r\n"
		"  <br><br>\r\n"
		"  <input type=\"submit\" value=\"Enviar\">\r\n"
		"</form> \r\n"
		"\r\n"
		"<p>Haz click en \"Enviar\" para guardar la informaci&oacute;n.</p>\r\n"
		"\r\n"
		"<p>Los datos de acceso actuales son:</p>\r\n"
		"<ul>\r\n"
			"<li>SSID: " ;
char * pcHtml1Part2 = "</li>\r\n"
			"<li>Contrase&ntilde;a: ";
char * pcHtml1Part3 ="</li>\r\n"
		"</ul>\r\n"
		"</body>\r\n"
		"\r\n"
		"</html>\r\n";

// guardado.html
char * pcHtml2Part1 = "<!DOCTYPE html>\r\n"
		"<html>\r\n"
		"<body>\r\n"
		"\r\n"
		"<h2>Configuraci&oacute;n del acceso a la Wi-Fi guardada</h2>\r\n"
		"\r\n"
		"<p>Los datos de acceso guardados son:</p>\r\n"
		"<ul>\r\n"
			"<li>SSID: " ;
char * pcHtml2Part2 = "</li>\r\n"
			"<li>Contrase&ntilde;a: ";
char * pcHtml2Part3 ="</li>\r\n"
		"</ul>\r\n"
		"\r\n"
		"<p>El punto de acceso \"MiWiFi\" ha sido desactivado.</p>\r\n"
		"</body>\r\n"
		"\r\n"
		"</html>\r\n";

char pcHtmlPage[1000];

/* An HTTP GET handler */
esp_err_t wifi_get_handler(httpd_req_t *req)
{
    char*  buf;
    size_t buf_len;

    /* Set some custom headers */
    // httpd_resp_set_hdr(req, "Custom-Header-1", "Custom-Value-1");
    httpd_resp_set_type(req,HTTPD_TYPE_TEXT);

    /* Send response with custom headers and body set as the
     * string passed in user context*/
    strcpy(pcHtmlPage,pcHtml1Part1);
    strcat(pcHtmlPage,pcBSSID);
    strcat(pcHtmlPage,pcHtml1Part2);
    strcat(pcHtmlPage,pcPassword);
    strcat(pcHtmlPage,pcHtml1Part3);
    const char* resp_str = (const char*) pcHtmlPage; //req->user_ctx;
    httpd_resp_send(req, resp_str, strlen(resp_str));

    /* After sending the HTTP response the old HTTP request
     * headers are lost. Check if HTTP request headers can be read now. */
    if (httpd_req_get_hdr_value_len(req, "Host") == 0) {
        ESP_LOGI(TAG2, "Request headers lost");
    }
    return ESP_OK;
}

httpd_uri_t wifi = {
    .uri       = "/wifi", // "/wifi"
    .method    = HTTP_GET,
    .handler   = wifi_get_handler,
    .user_ctx  = (void*) "Hello World!"
};


/* An HTTP GET handler */
esp_err_t guardado_get_handler(httpd_req_t *req)
{
    char*  buf;
    size_t buf_len;


    /* Read URL query string length and allocate memory for length + 1,
     * extra byte for null termination */
    buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1) {
        buf = (char *)malloc(buf_len);
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
            ESP_LOGI(TAG2, "Found URL query => %s", buf);
            char param[128];
            /* Get value of expected key from query string */
            if (httpd_query_key_value(buf, "ssid", param, sizeof(param)) == ESP_OK) {
                ESP_LOGI(TAG2, "Found URL query parameter => ssid=%s", param);
                strcpy(pcBSSID,param);
            }
            if (httpd_query_key_value(buf, "password", param, sizeof(param)) == ESP_OK) {
                ESP_LOGI(TAG2, "Found URL query parameter => password=%s", param);
                strcpy(pcPassword,param);
            }

            esp_err_t err;
            err = save_wifi_login_info();
            if (err != ESP_OK) printf("Error (%s) saving restart counter to NVS!\n", esp_err_to_name(err));

        }
        free(buf);
    }

    /* Set some custom headers */
    // httpd_resp_set_hdr(req, "Custom-Header-1", "Custom-Value-1");
    httpd_resp_set_type(req,HTTPD_TYPE_TEXT);

    /* Send response with custom headers and body set as the
     * string passed in user context*/
    strcpy(pcHtmlPage,pcHtml2Part1);
    strcat(pcHtmlPage,pcBSSID);
    strcat(pcHtmlPage,pcHtml2Part2);
    strcat(pcHtmlPage,pcPassword);
    strcat(pcHtmlPage,pcHtml2Part3);
    const char* resp_str = (const char*) pcHtmlPage; //req->user_ctx;
    httpd_resp_send(req, resp_str, strlen(resp_str));

    /* After sending the HTTP response the old HTTP request
     * headers are lost. Check if HTTP request headers can be read now. */
    if (httpd_req_get_hdr_value_len(req, "Host") == 0) {
        ESP_LOGI(TAG2, "Request headers lost");
    }

    int iMessage = WIFI_LOGGING_INFO_RECEIVED;
    BaseType_t xStatus = xQueueSendToBack(xMyQueueHandle,&(iMessage),0);
    if ( xStatus != pdPASS )         ESP_LOGE("wifiaux event", "Could not send to the queue");

    return ESP_OK;
}

httpd_uri_t guardado = {
    .uri       = "/guardado",
    .method    = HTTP_GET,
    .handler   = guardado_get_handler,
    .user_ctx  = (void*) "Hello World!"
};

httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Start the httpd server
    ESP_LOGI(TAG2, "Starting server on port: %d", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers
        ESP_LOGI(TAG2, "Registering URI handlers");
        httpd_register_uri_handler(server, &wifi);
        httpd_register_uri_handler(server, &guardado);
        return server;
    }

    ESP_LOGI(TAG2, "Error starting server!");
    return NULL;
}

void stop_webserver(httpd_handle_t server)
{
    // Stop the httpd server
    httpd_stop(server);
}


// WIFI sutff


// EVENT handler
// ---------------------------------------------------------


esp_err_t event_handler(void *ctx, system_event_t *event)
{
	BaseType_t xStatus;
	int iMessage;

	httpd_handle_t *server = (httpd_handle_t *) ctx;

	switch(event->event_id){
	case SYSTEM_EVENT_STA_GOT_IP:
		iMessage = SYSTEM_EVENT_STA_GOT_IP;
		xStatus = xQueueSendToBack(xMyQueueHandle,&(iMessage),0);
		if ( xStatus != pdPASS )         ESP_LOGE("wifiaux event", "Could not send to the queue");
        ESP_LOGE("wifiaux", "event_handler == %d", event->event_id);
    	// stops web server
    	if (*server){
    		stop_webserver(*server);
    		*server = NULL;
            ESP_LOGE("wifiaux", "stop_webserver()");
    	}
        return ESP_OK;
	case SYSTEM_EVENT_STA_DISCONNECTED:
		iMessage = SYSTEM_EVENT_STA_DISCONNECTED;
		xStatus = xQueueSendToBack(xMyQueueHandle,&(iMessage),0);
		if ( xStatus != pdPASS )         ESP_LOGE("wifiaux event", "Could not send to the queue");
        ESP_LOGE("wifiaux", "event_handler == %d", event->event_id);
        return ESP_OK;
	case SYSTEM_EVENT_AP_STAIPASSIGNED:
		// starts web server
		if (*server  == NULL){
			*server = start_webserver();
	        ESP_LOGE("wifiaux", "start_webserver()");
		}
        return ESP_OK;
	case SYSTEM_EVENT_AP_STOP: //SYSTEM_EVENT_AP_STADISCONNECTED:
        ESP_LOGE("wifiaux", "event_handler == %d", event->event_id);
        return ESP_OK;
	default:
        ESP_LOGE("wifiaux", "event_handler == %d", event->event_id);
        return ESP_OK;
	}
}

void init_wifi(){

	static httpd_handle_t server = NULL;

	esp_err_t err;
	esp_err_t err_sta;
	BaseType_t xMessage;

	xMyQueueHandle = xQueueCreate(10,sizeof(int));

    err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK( err );

    err = read_wifi_login_info();
    if (err != ESP_OK) printf("Error (%s) reading data from NVS!\n", esp_err_to_name(err));

    tcpip_adapter_init();
    ESP_ERROR_CHECK( esp_event_loop_init(event_handler, &server) );
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    //Allocate storage for the struct
    wifi_config_t sta_config = {};
    wifi_config_t ap_config = {};

    // config AP
    strcpy((char*)ap_config.ap.ssid, AP_SSID);
    ap_config.ap.ssid_len = strlen(AP_SSID);
    strcpy((char*)ap_config.ap.password, "");
    ap_config.ap.max_connection = 2;
    ap_config.ap.authmode = WIFI_AUTH_OPEN;

    ESP_LOGI("wifiaux", "enters do-while");

    do {
    	iDisplayState = STATE_CONECTING_TO_AP;

        // config STA
        //Assign ssid & password strings
        strcpy((char*)sta_config.sta.ssid, pcBSSID);
        strcpy((char*)sta_config.sta.password, pcPassword); // WIFI_PASS
        sta_config.sta.bssid_set = false;

    	ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    	ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &sta_config) );
    	ESP_ERROR_CHECK( esp_wifi_start() );
    	//ESP_ERROR_CHECK( esp_wifi_connect() );
    	err_sta = esp_wifi_connect();

    	do {
    		xQueueReceive( xMyQueueHandle, &xMessage, portMAX_DELAY );
    	}while(xMessage != SYSTEM_EVENT_STA_GOT_IP && xMessage != SYSTEM_EVENT_STA_DISCONNECTED);

    	if ( xMessage == SYSTEM_EVENT_STA_DISCONNECTED ){

    		iDisplayState = STATE_AP_STARTED;

    		ESP_LOGE("wifiaux", "error esp_wifi_connect() == %d", err_sta);

    		//esp_wifi_disconnect();
    		esp_wifi_stop();
    		//esp_wifi_deinit();

    		ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_AP) );
    		ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_AP, &ap_config) );
    		ESP_ERROR_CHECK( esp_wifi_start() );

    		tcpip_adapter_ip_info_t ipInfo;
    		tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_AP,&ipInfo);
    		//  const char * inet_ntop (int af, const void *cp, char *buf, size_t len)
    		ESP_LOGW("wifiaux", "connect to ap SSID: %s, password %s, ip = %d.%d.%d.%d", AP_SSID,"",IP2STR(&(ipInfo.ip)));

    		do {
    			err = xQueueReceive( xMyQueueHandle, &xMessage, portMAX_DELAY );
        		if(err == pdPASS) ESP_LOGE("wifiaux", "xQueueReceive() == %d", xMessage);

    		}while(xMessage != WIFI_LOGGING_INFO_RECEIVED);

        	iDisplayState = STATE_CONECTING_TO_AP;

        	err = save_wifi_login_info();
    	    if (err != ESP_OK) {
    	    	ESP_LOGE("wifiaux", "save_wifi_login_info() == %d", err);
    	    }

    	    vTaskDelay(4000 / portTICK_PERIOD_MS);

    		err = esp_wifi_stop();
    	    if (err != ESP_OK) {
    	    	ESP_LOGE("wifiaux", "esp_wifi_stop() == %d", err);
    	    }

    	    vTaskDelay(4000 / portTICK_PERIOD_MS);

    	}
    } while ( xMessage != SYSTEM_EVENT_STA_GOT_IP  ); // espera mientras no reciba una IP



    iDisplayState = STATE_CONECTED_TO_AP;
}

#endif
