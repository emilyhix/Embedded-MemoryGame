/* ###################################################################
**     Filename    : main.c
**     Project     : MemoryGame
**     Processor   : MK64FN1M0VLL12
**     Version     : Driver 01.01
**     Compiler    : GNU C Compiler
**     Date/Time   : 2025-06-04, 16:46, # CodeGen: 0
**     Abstract    :
**         Main module.
**         This module contains user's application code.
**     Settings    :
**     Contents    :
**         No public methods
**
** ###################################################################*/
/*!
** @file main.c
** @version 01.01
** @brief
**         Main module.
**         This module contains user's application code.
*/         
/*!
**  @addtogroup main_module main module documentation
**  @{
*/         
/* MODULE main */


/* Including needed modules to compile this module/procedure */
#include "Cpu.h"
#include "Events.h"
#include "Pins1.h"
#include "SM1.h"
/* Including shared modules, which are used for whole project */
#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"
#include "PDD_Includes.h"
#include "Init_Config.h"
/* User includes (#include below this line is not maintained by Processor Expert) */
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

// Global Variables
static int lights_size = 5; // Initialize array for memory game
static int* light_order;
static unsigned int timer_val = 0;
static unsigned int count = 0;
static unsigned long delay = 0x17A120; // Delay value
static unsigned long Input = 0;
static unsigned long red_button = 0;
static unsigned long green_button = 0;
static unsigned long yellow_button = 0;
static unsigned long blue_button = 0;
static int pressed_button = -1;
static unsigned int button_count = 1;
static int score = 0;
unsigned char start1[30];
unsigned char start2[30];
unsigned char write[512];
LDD_TDeviceData *SM1_DeviceData;

enum MemoryGameState {
	STATE_START_MSG,
	STATE_START_GAME,
	STATE_DELAY,
	STATE_RANDOM_LIGHT,
	STATE_DISPLAY_LIGHT,
	STATE_USER_INPUT,
	STATE_TURN_MESSAGE,
	STATE_CHECK_INPUT,
	STATE_UPDATE_SIZE,
	STATE_GAME_OVER,
	STATE_START_BUTTON,
	STATE_END
};
/*lint -save  -e970 Disable MISRA rule (6.3) checking. */

void software_delay(unsigned long delay) {
	while (delay > 0)
		delay--;
}

// selects the lights and adds them to the array
void random_light(int lights[], int index) {
	unsigned int rnd = rand();

	unsigned int light;
	light = rnd % 4;
	lights[index] = light;
}

// displays the lights in the array
void display_lights(int lights[], int index) {
	if (lights[index] == 0) {
		GPIOC_PDOR = 0x01;
	} else if (lights[index] == 1) {
		GPIOC_PDOR = 0x02;
	} else if (lights[index] == 2) {
		GPIOC_PDOR = 0x04;
	} else if (lights[index] == 3) {
		GPIOC_PDOR = 0x08;
	}
}

enum MemoryGameState execute_state(enum MemoryGameState currState) {
	static int len;

	switch (currState) {

	case STATE_START_MSG:
		len = sprintf(start1, "Press the red\n");
		SM1_SendBlock(SM1_DeviceData, &start1, len);
		software_delay(0x10000);
		len = 0;
		len = sprintf(start2, "button to start!\n");
		SM1_SendBlock(SM1_DeviceData, &start2, len);
		len = 0;
		return STATE_START_BUTTON;

	case STATE_START_BUTTON:
		Input = GPIOB_PDIR;
		if ((Input >> 2) & 0x01) {
			return STATE_START_GAME;
		} else {
			return STATE_START_BUTTON;
		}

	case STATE_START_GAME:
		timer_val = FTM3_CNT;
		srand(timer_val);
		len = sprintf(write, "Game START!\n");
		SM1_SendBlock(SM1_DeviceData, &write, len);
		software_delay(delay);
		return STATE_DELAY;

	case STATE_DELAY:
		software_delay(delay);
		return STATE_RANDOM_LIGHT;

	case STATE_RANDOM_LIGHT:
		random_light(light_order, count);
		count++;
		return (count % lights_size == 0) ?
				STATE_UPDATE_SIZE : STATE_DISPLAY_LIGHT;

	case STATE_DISPLAY_LIGHT:
		len = sprintf(write, "Watch closely!\n");
		SM1_SendBlock(SM1_DeviceData, &write, len);
		for (int i = 0; i < count; i++) {
			display_lights(light_order, i);
			software_delay(delay);
			GPIOC_PDOR = 0x00000000;
			software_delay(delay);
		}
		return STATE_TURN_MESSAGE;

	case STATE_TURN_MESSAGE:
		software_delay(0x10000);
		len = sprintf(write, "Your Turn!\n");
		SM1_SendBlock(SM1_DeviceData, &write, len);
		software_delay(0x10000);
		len = sprintf(write, "Score: %i\n", score);
		SM1_SendBlock(SM1_DeviceData, &write, len);
		return STATE_USER_INPUT;

	case STATE_USER_INPUT:
		Input = GPIOB_PDIR;
		red_button = (Input >> 2) & 0x01;
		green_button = (Input >> 3) & 0x01;
		yellow_button = (Input >> 10) & 0x01;
		blue_button = (Input >> 11) & 0x01;

		if (button_count <= count) {
			if (!red_button && !green_button && !yellow_button
					&& !blue_button) {
				return STATE_USER_INPUT; // stay here
			} else {
				software_delay(0xFFFF);

				Input = GPIOB_PDIR;
				red_button = (Input >> 2) & 0x01;
				green_button = (Input >> 3) & 0x01;
				yellow_button = (Input >> 10) & 0x01;
				blue_button = (Input >> 11) & 0x01;

				if (red_button)
					pressed_button = 0;
				else if (green_button)
					pressed_button = 1;
				else if (yellow_button)
					pressed_button = 2;
				else if (blue_button)
					pressed_button = 3;
				else
					return STATE_USER_INPUT;

				while (GPIOB_PDIR
						& ((1 << 2) | (1 << 3) | (1 << 10) | (1 << 11))) {
					software_delay(0x10000);
				}

				return STATE_CHECK_INPUT;
			}
		} else {
			button_count = 1;
			score++;
			return STATE_DELAY;
		}

	case STATE_CHECK_INPUT:
		if (light_order[button_count - 1] != pressed_button) {
			return STATE_GAME_OVER;
		} else {
			button_count++;
			pressed_button = -1;
			return STATE_USER_INPUT;
		}

	case STATE_UPDATE_SIZE:
		lights_size *= 2;
		int* temp = realloc(light_order, lights_size * sizeof(int));
		if (temp == NULL) {
			return STATE_GAME_OVER;
		}
		light_order = temp;
		return STATE_DISPLAY_LIGHT;

	case STATE_GAME_OVER:
		len = sprintf(write, "Game Over!\n");
		SM1_SendBlock(SM1_DeviceData, &write, len);
		software_delay(0x10000);
		len = sprintf(write, "Your Score: %i!!\n", score);
		SM1_SendBlock(SM1_DeviceData, &write, len);
		GPIOC_PDOR = 0x0F;
		count = 0;
		return STATE_END;

	case STATE_END:
		return STATE_END;

	default:
		return STATE_GAME_OVER;
	}
}

int main(void) {
	/*lint -restore Enable MISRA rule (6.3) checking. */

	/*** Processor Expert internal initialization. DON'T REMOVE THIS CODE!!! ***/
	PE_low_level_init();
	/*** End of Processor Expert internal initialization.                    ***/

	// Clock Gating
	SIM_SCGC5 |= SIM_SCGC5_PORTB_MASK; /*Enable Port B Clock Gate Control*/
	SIM_SCGC5 |= SIM_SCGC5_PORTC_MASK; /*Enable Port C Clock Gate Control*/
	SIM_SCGC3 |= SIM_SCGC3_FTM3_MASK;

	FTM3_MODE = 0x5;
	FTM3_MOD = 0xFFFF;
	FTM3_SC = 0x0F;

	// GPIO
	PORTB_GPCLR = 0x0FFF0100; // PORTB
	PORTC_GPCLR = 0x000F0100; // PORTC

	// I/O
	GPIOB_PDDR = 0x00000000; // PORTB - Buttons - input
	GPIOC_PDDR = 0x0000000F; // PORTC - LEDs - output

	// Turn off all lights
	GPIOC_PDOR = 0x00000000;

	// Initialize array
	light_order = (int*) malloc(lights_size * sizeof(int));
	for (int i = 0; i < lights_size; i++) {
		light_order[i] = -1;
	}

	// Initialize first state
	enum MemoryGameState currState = STATE_START_MSG;
	SM1_DeviceData = SM1_Init(NULL);

	for (;;) {
		currState = execute_state(currState);
	}

	/* For example: for(;;) { } */

	/*** Don't write any code pass this line, or it will be deleted during code generation. ***/
  /*** RTOS startup code. Macro PEX_RTOS_START is defined by the RTOS component. DON'T MODIFY THIS CODE!!! ***/
  #ifdef PEX_RTOS_START
    PEX_RTOS_START();                  /* Startup of the selected RTOS. Macro is defined by the RTOS component. */
  #endif
  /*** End of RTOS startup code.  ***/
  /*** Processor Expert end of main routine. DON'T MODIFY THIS CODE!!! ***/
  for(;;){}
  /*** Processor Expert end of main routine. DON'T WRITE CODE BELOW!!! ***/
} /*** End of main routine. DO NOT MODIFY THIS TEXT!!! ***/

/* END main */
/*!
 ** @}
 */
/*
 ** ###################################################################
 **
 **     This file was created by Processor Expert 10.4 [05.11]
 **     for the Freescale Kinetis series of microcontrollers.
 **
 ** ###################################################################
 */
