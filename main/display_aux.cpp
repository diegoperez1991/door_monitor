#ifndef DISPLAY_AUX_c
#define DISPLAY_AUX_c

static const char* TAG = "tDisplay";
//#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include "esp_log.h"
#include "global_variables_aux.c"

// sntp
#include <time.h>
#include <sys/time.h>
#include "lwip/err.h"
#include "apps/sntp/sntp.h"

//#include "meteogalicia_aux.c"


/*
 * display_aux.c
 *
 *  Created on: 4 ago. 2018
 *      Author: user
 */

//https://github.com/espressif/arduino-esp32/blob/master/docs/esp-idf_component.md
//file: main.cpp
#include "Arduino.h"

// Define OLED properties
// https://github.com/ThingPulse/esp8266-oled-ssd1306
// https://es.aliexpress.com/item/ESP32-OLED-for-Arduino-ESP32-OLED-WiFi-Modules-Bluetooth-Dual-ESP-32-ESP-32S-ESP8266-OLED/32807531243.html
#include "SSD1306Wire.h" //#include "SSD1306Wire.h"
#include "OLEDDisplayUi.h"
// Initialize the OLED display using Wire library
SSD1306Wire  display(0x3c, 5, 4);
OLEDDisplayUi   ui( &display );

enum State_t {TEMP_VIGO = 1,
	SENS_VIGO = 2,
	TEMP_ARBO = 3,
	SENS_ARBO = 4,
	HORA = 5,
	FECHA = 6,
};
State_t xDisplay = TEMP_VIGO;
char pcDisplayedMessage[50];



void drawConectando() {

	display.resetDisplay();
	display.clear();
	display.setFont(ArialMT_Plain_16);
	display.setTextAlignment(TEXT_ALIGN_CENTER);
	char *pcMensageConectando1 = "Conectando a...\n";
	strcpy(pcDisplayedMessage,pcMensageConectando1);
	strcat(pcDisplayedMessage,pcBSSID);
	display.drawString(64, 10, pcDisplayedMessage);
	display.display();
	ESP_LOGI(TAG, "drawConectando(): %s",pcDisplayedMessage);


}

void drawConectate() {

	display.resetDisplay();
	display.clear();
	display.setFont(ArialMT_Plain_16);
	display.setTextAlignment(TEXT_ALIGN_CENTER);
	char *pcMensageConectando1 = "Conectate a\n \"MiWiFi\"\n y...";
	display.drawString(64, 0, pcMensageConectando1);
	display.display();
	ESP_LOGI(TAG, "drawConectate(): %s",pcMensageConectando1);


}

void drawAccede() {

	display.resetDisplay();
	display.clear();
	display.setFont(ArialMT_Plain_16);
	display.setTextAlignment(TEXT_ALIGN_CENTER);
	char *pcMensageConectando1 = "... accede a\n 192.168.4.1/wifi";
	display.drawString(64, 10, pcMensageConectando1);
	display.display();
	ESP_LOGI(TAG, "drawAccede(): %s",pcMensageConectando1);

}


void drawFrame(char * sLabel1, char * sLabel2) {

	strcpy(pcDisplayedMessage,(const char*)sLabel1);
	strcat(pcDisplayedMessage,(const char*)"\n");
	strcat(pcDisplayedMessage,(const char*)sLabel2);
	display.resetDisplay();
	display.clear();
	display.drawString(64, 0, pcDisplayedMessage);
	display.display();
	ESP_LOGI(TAG, "drawFrame(): %s",pcDisplayedMessage);

}




void vTaskDisplay( void *pvParameters ){
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount ();

	// arduino thing
	initArduino();
	ESP_LOGI(TAG, "Arduino initialized");

	// ssd1306
	bool bInitOk = display.init();
	if ( bInitOk )
		ESP_LOGI(TAG, "Display init completed");
	else
		ESP_LOGE(TAG, "Display init NOT completed");

	display.flipScreenVertically();
	// Set display contrast
	// really low brightness & contrast: contrast = 10, precharge = 5, comdetect = 0
	// normal brightness & contrast:  contrast = 100
	// void setContrast(uint8_t contrast, uint8_t precharge = 241, uint8_t comdetect = 64);
	display.resetDisplay();
	//display.flipScreenVertically();
	display.setBrightness(1);
	display.clear();

	//display.setBrightness(1);

	// clear the display

	int iValue = 0;
	char str[10];
	char sValue[10];

	//sntp
	char ppcDiasSemana[7][20] = { "Domingo","Lunes", "Martes", "Miercoles", "Jueves","Viernes","Sabado"};
	char * pcFormatDia = "%w"; // day of the week 0 to 6
	char * pcFormatFecha = "%d/%m/%y";
	char * pcFormatHora = "%H:%M:%S";
	char outstr[200];
	char outstr2[20];
	int iAux;
	time_t t;
	struct tm *tmp;

	do{



		// Conecting to an AP
		while( iDisplayState == STATE_CONECTING_TO_AP){
			drawConectando();
			vTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS( 3000 ) );
		}

		// Configured as an AP
		while( iDisplayState == STATE_AP_STARTED){
			drawConectate();
			vTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS( 3000 ) );
			drawAccede();
			vTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS( 3000 ) );

		}



		display.setFont(ArialMT_Plain_24);
		display.setTextAlignment(TEXT_ALIGN_CENTER);

		// Conected to an AP
		while (iDisplayState == STATE_CONECTED_TO_AP){

			//ESP_LOGI(TAG, "current display state == %i",xDisplay);

			switch( xDisplay ) {

			case TEMP_VIGO  :
				iValue = -5;
				sprintf(str, "%d", iValue);
				//char *strcat(char *dest, const char *src);
				strcpy(sValue,pcTempVigo);
				strcat(sValue,(const char*)"°C");
				drawFrame("Vigo",sValue);
				xDisplay = SENS_VIGO;
				break; /* optional */

			case SENS_VIGO  :
				iValue = 24;
				sprintf(str, "%d", iValue);
				//char *strcat(char *dest, const char *src);
				strcpy(sValue,pcSensVigo);
				strcat(sValue,(const char*)"°C");
				drawFrame("Sensacion",sValue);
				xDisplay = HORA;
				break; /* optional */

			case HORA  :
				t = time(NULL);
				tmp = localtime(&t);
				strftime(pcHora, sizeof(pcHora), pcFormatHora , tmp);
				drawFrame("Hora",pcHora);
				xDisplay = FECHA;
				break; /* optional */

			case FECHA  :
				t = time(NULL);
				tmp = localtime(&t);
				strftime(pcFecha, sizeof(pcFecha), pcFormatFecha , tmp);
				strftime(outstr2, sizeof(outstr), pcFormatDia , tmp);
				iAux = ((int)*outstr2) - 48;
				//ppcDiasSemana[iAux]
								drawFrame(ppcDiasSemana[iAux],pcFecha);
								xDisplay = TEMP_VIGO;
								break; /* optional */

			default : /* Optional */
				xDisplay = SENS_VIGO;
				drawFrame("ST Vigo","DEFAULT");

			}

			vTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS( 2000 ) );
		}

	} while(true);
}

#endif
