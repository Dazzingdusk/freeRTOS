#include "stm32f4xx.h"
#include "usart.h"
#include "delay.h"
#include "FreeRTOS.h"
#include "task.h"
#include "led.h"
#include "key.h"
#include "queue.h"

TickType_t MyCountTemp=0;
#define  DEBUG    0

#define START_TASK_PRIO  1
#define START_STK_SIZE    128
TaskHandle_t StartTask_Handler;
void start_task(void *PvParameters);

#define LED_TASK_PRIO  2
#define LED_STK_SIZE    128
TaskHandle_t LedTask_Handler;
void led_task(void *PvParameters);

#define KEY_TASK_PRIO  3
#define KEY_STK_SIZE    128
TaskHandle_t KeyTask_Handler; 
void Key_task(void *PvParameters);


QueueHandle_t  Q_Handle_Key;
QueueHandle_t  Q_Handle_Uart;

u8 string[]="RECEIVER Queue from key task!";

int main(void)
{

	uart_init(115200);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	LED_Init();
	KEY_Init();
	

	xTaskCreate((TaskFunction_t	)led_task,
							(const char*   	)"LED_TASK",
							(uint16_t      	)LED_STK_SIZE,
							(void*					)NULL,
							(UBaseType_t	 	)LED_TASK_PRIO,
							(TaskHandle_t *	)&LedTask_Handler);
							
	xTaskCreate((TaskFunction_t	)start_task,
							(const char*   	)"Start_TASK",
							(uint16_t      	)START_STK_SIZE,
							(void*					)NULL,
							(UBaseType_t	 	)START_TASK_PRIO,
							(TaskHandle_t *	)&StartTask_Handler);
							
	xTaskCreate((TaskFunction_t	)Key_task,
							(const char*   	)"Key_TASK",
							(uint16_t      	)KEY_STK_SIZE,
							(void*					)NULL,
							(UBaseType_t	 	)KEY_TASK_PRIO,
							(TaskHandle_t *	)&KeyTask_Handler);
							
	Q_Handle_Key = xQueueCreate(2,sizeof(u8));
	Q_Handle_Uart= xQueueCreate(2,sizeof(string));						
	vTaskStartScheduler();
while(1);							
}

void led_task(void *PvParameters)
{
	u8 key_vel;
	BaseType_t err;
	
//	portTickType xLastWakeTime = xTaskGetTickCount();
	while(1)
	{
		
			if(uxQueueMessagesWaiting(Q_Handle_Key))
			{
					err=xQueueReceive(Q_Handle_Key,&key_vel,0);
					if(err == pdPASS)
					{
							switch (key_vel)
							{
								case KEY0_PRES:LED0=!LED0;break;
								case KEY1_PRES:LED1=!LED1;break;
								case WKUP_PRES:err = xQueueSend(Q_Handle_Uart,&string,0);break;
								default:break;	
							}
							#if DEBUG
							if(key_vel == WKUP_PRES)
							{
								if(err == pdPASS)
									printf("A Send Queue to Uart is OK\r\n");
								else if(err == errQUEUE_FULL)
									printf("A Send Queue to Uart is FULL\r\n");
								else 
									printf("A Send Queue to Uart is FALSE\r\n");
							}
							#endif
					}
					#if DEBUG
					else
					{
						printf("A LedTask receive data  FALSE\r\n");
					}
					#endif
			}
		vTaskDelay(1);
	}
}
void start_task(void *PvParameters)
{
	u8 recv[sizeof(&string[0])+1];
	BaseType_t err;
//	UBaseType_t QueueNum;
	//portTickType xLastWakeTime = xTaskGetTickCount();
	while(1)
	{
		if(uxQueueMessagesWaiting(Q_Handle_Uart))
		{
			err=xQueueReceive(Q_Handle_Uart,&recv[0],0);
			if(err == pdPASS)
				printf("B %s\r\n",recv);
			#if DEBUG
			else
				printf("B Receive Queue from Uart is FALSE\r\n");
			#endif
		}
		vTaskDelay(1);
			//vTaskDelayUntil(&xLastWakeTime,50);
	}
}
void Key_task(void *PvParameters)
{
	u8 key_vel;
	BaseType_t err;
	while(1)
	{
			key_vel =  KEY_Scan(0);	
			if(key_vel)
			{
				err = xQueueSend(Q_Handle_Key,&key_vel,portMAX_DELAY);
				#if DEBUG
				if(err ==pdPASS )
					printf("C Send Queue is OK!\r\n");
				else if(err == errQUEUE_FULL)
					printf("C Queue is FULL\r\n");
				else
					printf("C KeyTask Send Queue is ERROR!\r\n");
				#endif
			}
			vTaskDelay(10);
	}
}
