/*
 * FreeRTOS Kernel V10.2.0
 * Copyright (C) 2019 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

/* 
	NOTE : Tasks run in system mode and the scheduler runs in Supervisor mode.
	The processor MUST be in supervisor mode when vTaskStartScheduler is 
	called.  The demo applications included in the FreeRTOS.org download switch
	to supervisor mode prior to main being called.  If you are not using one of
	these demo application projects then ensure Supervisor mode is used.
*/


/*
 * Creates all the demo application tasks, then starts the scheduler.  The WEB
 * documentation provides more details of the demo application tasks.
 * 
 * Main.c also creates a task called "Check".  This only executes every three 
 * seconds but has the highest priority so is guaranteed to get processor time.  
 * Its main function is to check that all the other tasks are still operational.
 * Each task (other than the "flash" tasks) maintains a unique count that is 
 * incremented each time the task successfully completes its function.  Should 
 * any error occur within such a task the count is permanently halted.  The 
 * check task inspects the count of each task to ensure it has changed since
 * the last time the check task executed.  If all the count variables have 
 * changed all the tasks are still executing error free, and the check task
 * toggles the onboard LED.  Should any task contain an error at any time 
 * the LED toggle rate will change from 3 seconds to 500ms.
 *
 */

/* Standard includes. */
#include <stdlib.h>
#include <stdio.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "lpc21xx.h"

/* Peripheral includes. */
#include "serial.h"
#include "GPIO.h"
#include "queue.h"


/*-----------------------------------------------------------*/

/* Constants to setup I/O and processor. */
#define mainBUS_CLK_FULL	( ( unsigned char ) 0x01 )

/* Constants for the ComTest demo application tasks. */
#define mainCOM_TEST_BAUD_RATE	( ( unsigned long ) 115200 )


/*
 * Configure the processor for use with the Keil demo board.  This is very
 * minimal as most of the setup is managed by the settings in the project
 * file.
 */
static void prvSetupHardware( void );
/*-----------------------------------------------------------*/
QueueHandle_t xQueue1;
char* Global_Message;
unsigned short Global_Length;
int Button_1_Monitor_InTime,  Button_1_Monitor_OutTime, Button_1_Monitor_TotalTime;
int Button_2_Monitor_InTime,  Button_2_Monitor_OutTime,  Button_2_Monitor_TotalTime;
int Periodic_Transmitter_InTime, Periodic_Transmitter_OutTime, Periodic_Transmitter_TotalTime;
int Uart_Receiver_InTime,    Uart_Receiver_OutTime,     Uart_Receiver_TotalTime;
int Load_1_Simulation_InTime,    Load_1_Simulation_OutTime,   Load_1_Simulation_TotalTime;
int Load_2_Simulation_InTime,    Load_2_Simulation_OutTime,    Load_2_Simulation_TotalTime;
int system_Time;
int cpu_Load;

void Button_1_Monitor();
void Button_2_Monitor();
void Periodic_Transmitter();
void Uart_Receiver();
void Load_1_Simulation();
void Load_2_Simulation();

void vApplicationTickHook( void )
{
	GPIO_write (PORT_0, PIN0, PIN_IS_HIGH);
	GPIO_write (PORT_0, PIN0, PIN_IS_LOW);
}

/*void vApplicationIdleHook( void )
{
	GPIO_write (PORT_0, PIN9, PIN_IS_HIGH);
}
*/
/*
 * Application entry point:
 * Starts all the other tasks, then starts the scheduler. 
 */
int main( void )
{
	/* Setup the hardware for use with the Keil demo board. */
		prvSetupHardware();
		xQueue1 = xQueueCreate( 5, sizeof( char* ) );
	
    /* Create Tasks here */
		xTaskPeriodicCreate(Button_1_Monitor,"Button_1_Monitor",100,( void * ) 0,1,NULL,50);     													
		xTaskPeriodicCreate(Button_2_Monitor,"Button_2_Monitor",100,( void * ) 0,1,NULL,50);     													
		xTaskPeriodicCreate(Periodic_Transmitter,"Periodic_Transmitter",100,( void * ) 0,1,NULL,100);     													
		xTaskPeriodicCreate(Uart_Receiver,"Uart_Receiver",100,( void * ) 0,1,NULL,20);     													
		xTaskPeriodicCreate(Load_1_Simulation,"Load_1_Simulation",100,( void * ) 0,1,NULL,10);     													
		xTaskPeriodicCreate(Load_2_Simulation,"Load_2_Simulation",100,( void * ) 0,1,NULL,100);     													

	/* Now all the tasks have been started - start the scheduler.

	NOTE : Tasks run in system mode and the scheduler runs in Supervisor mode.
	The processor MUST be in supervisor mode when vTaskStartScheduler is 
	called.  The demo applications included in the FreeRTOS.org download switch
	to supervisor mode prior to main being called.  If you are not using one of
	these demo application projects then ensure Supervisor mode is used here. */
	vTaskStartScheduler();

	/* Should never reach here!  If you do then there was not enough heap
	available for the idle task to be created. */
	for( ;; );
}
/*-----------------------------------------------------------*/

/* Function to reset timer 1 */
void timer1Reset(void)
{
	T1TCR |= 0x2;
	T1TCR &= ~0x2;
}

/* Function to initialize and start timer 1 */
static void configTimer1(void)
{
	T1PR = 1000;
	T1TCR |= 0x1;
}

static void prvSetupHardware( void )
{
	/* Perform the hardware setup required.  This is minimal as most of the
	setup is managed by the settings in the project file. */

	/* Configure UART */
	xSerialPortInitMinimal(mainCOM_TEST_BAUD_RATE);

	/* Configure GPIO */
	GPIO_init();
	
	/* Config trace timer 1 and read T1TC to get current tick */
	configTimer1();

	/* Setup the peripheral bus to be the same as the PLL output. */
	VPBDIV = mainBUS_CLK_FULL;
}
/*-----------------------------------------------------------*/




void Button_1_Monitor(){
	char*High_Message;
	char*Low_Message;
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();
	vTaskSetApplicationTaskTag( NULL, ( void * )1 );

		while(1){
	if(GPIO_read(PORT_0,PIN8)==PIN_IS_LOW){
			Global_Message="Button_1_Monitor is Falling\n";
			Global_Length=28;
			Low_Message="Button_1_Monitor is Falling\n";
		  if( xQueue1 != 0 )
			{
				xQueueSend( xQueue1, ( void * ) &Low_Message, ( TickType_t ) 0 );
			}
	}
	if(GPIO_read(PORT_0,PIN8)==PIN_IS_HIGH){
			Global_Message="Button_1_Monitor is Rising\n";
			Global_Length=27;
			High_Message="Button_1_Monitor is Rising\n";
			if( xQueue1 != 0 )
			{
				xQueueSend( xQueue1, ( void * ) &High_Message, ( TickType_t ) 0 );
			}

	}
			//GPIO_write (PORT_0, PIN3, PIN_IS_LOW);
			vTaskDelayUntil( &xLastWakeTime, 50 );
			//GPIO_write (PORT_0, PIN3, PIN_IS_HIGH);
}
		

}
void Button_2_Monitor()
	{
		char*High_Message;
		char*Low_Message;
		TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();
	vTaskSetApplicationTaskTag( NULL, ( void * )2 );
			while(1){

	if(GPIO_read(PORT_0,PIN9)==PIN_IS_LOW){
			Global_Message="Button_2_Monitor is Falling\n";
			Global_Length=28;
			Low_Message="Button_2_Monitor is Falling\n";
		  if( xQueue1 != 0 )
			{
				xQueueSend( xQueue1, ( void * ) &Low_Message, ( TickType_t ) 0 );
			}
	}
	if(GPIO_read(PORT_0,PIN9)==PIN_IS_HIGH){
			Global_Message="Button_2_Monitor is Rising\n";
			Global_Length=27;
			High_Message="Button_1_Monitor is Rising\n";
			if( xQueue1 != 0 )
			{
				xQueueSend( xQueue1, ( void * ) &High_Message, ( TickType_t ) 0 );
			}
	}
			//	GPIO_write (PORT_0, PIN3, PIN_IS_LOW);
			vTaskDelayUntil( &xLastWakeTime, 50 );
			//GPIO_write (PORT_0, PIN3, PIN_IS_HIGH);
}
	}
void Periodic_Transmitter(){
	char*TX_Message;
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();
	vTaskSetApplicationTaskTag( NULL, ( void * )3 );
			while(1){

		Global_Message="TX\n";
		Global_Length=2;
		TX_Message="TX";
			if( xQueue1 != 0 )
			{
				xQueueSend( xQueue1, ( void * ) &TX_Message, ( TickType_t ) 0 );
			}
	//GPIO_write (PORT_0, PIN3, PIN_IS_LOW);
	vTaskDelayUntil( &xLastWakeTime, 100 );
//	GPIO_write (PORT_0, PIN3, PIN_IS_HIGH);
}
}
void Uart_Receiver(){
	char* rx;
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();
	vTaskSetApplicationTaskTag( NULL, ( void * )4 );
			while(1){
				if( xQueueReceive( xQueue1,&(rx ),( TickType_t ) 0 ) == pdPASS )
		{
			vSerialPutString((const signed char *)(rx), 30);
		}
				//vSerialPutString(Global_Message,Global_Length);
				//GPIO_write (PORT_0, PIN3, PIN_IS_LOW);
				vTaskDelayUntil( &xLastWakeTime, 20 );
				//GPIO_write (PORT_0, PIN3, PIN_IS_HIGH);
}
			}
void Load_1_Simulation(){
	int i;
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();
	vTaskSetApplicationTaskTag( NULL, ( void * )5 );
			while(1){
					for(i=0;i<35000;i++){}
					//GPIO_write (PORT_0, PIN3, PIN_IS_LOW);
					vTaskDelayUntil( &xLastWakeTime, 10 );
					//GPIO_write (PORT_0, PIN3, PIN_IS_HIGH);
			}
}
void Load_2_Simulation(){
		int i;
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();
	vTaskSetApplicationTaskTag( NULL, ( void * )6 );
			while(1){
				
			for(i=0;i<90000;i++){}
					//GPIO_write (PORT_0, PIN3, PIN_IS_LOW);
					vTaskDelayUntil( &xLastWakeTime, 100 );
					//GPIO_write (PORT_0, PIN3, PIN_IS_HIGH);
			}
}