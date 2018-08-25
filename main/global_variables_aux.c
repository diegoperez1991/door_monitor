#ifndef GLOBAL_VARIABLES_AUX_c
#define GLOBAL_VARIABLES_AUX_c

/*
 * global_variables_aux.c
 *
 *  Created on: 7 ago. 2018
 *      Author: user
 */
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

char pcTempVigo[10];
char pcSensVigo[10];
char pcTempArbo[10];
char pcSensArbo[10];

char pcHora[20];
char pcFecha[20];

QueueHandle_t xMyQueueHandle;

char pcBSSID[128] = "";
char pcPassword[128] = "";


#define STATE_CONECTING_TO_AP 1
#define STATE_CONECTED_TO_AP 2
#define STATE_AP_STARTED 3

int iDisplayState = STATE_CONECTING_TO_AP;

#define GPIO_INPUT_DOOR_SW     GPIO_NUM_15
#define GPIO_OUTPUT_LED     GPIO_NUM_16
#define GPIO_INPUT_PIN_SEL  (1ULL << GPIO_INPUT_DOOR_SW)
#define GPIO_OUTPUT_PIN_SEL  (1ULL << GPIO_OUTPUT_LED)
#define ESP_INTR_FLAG_DEFAULT 0

// http client methods are not reentrant;
// therefore, a mutex avoids concurrent calls
static SemaphoreHandle_t xMyMutexHttp;

#endif


