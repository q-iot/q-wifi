#ifndef QSYS_OS_WRAP_H
#define QSYS_OS_WRAP_H


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/croutine.h"
#include "freertos/semphr.h"

#ifndef NULL
#define NULL ((void *)0)
#endif

#ifndef NOT_NULL
#define NOT_NULL ((void *)1)
#endif

#if 1
#define OS_SCHEDULE_PERIOD_FACTOR 10
#define OS_TASK_T xTaskHandle 
#define OS_QUEUE_T xQueueHandle
#define OS_SEM_T xSemaphoreHandle
#define OS_BASE_T portBASE_TYPE

#define Ms2Tick(Ms) ((Ms)/OS_SCHEDULE_PERIOD_FACTOR)
#define Tick2Ms(Period) ((Period)*OS_SCHEDULE_PERIOD_FACTOR)

#define OS_MAX_DELAY portMAX_DELAY
#define OS_GetNowMs() (system_get_time()/1000)

#define OS_DeclareCritical()	
#define OS_EnterCritical 	vPortEnterCritical
#define OS_ExitCritical 	vPortExitCritical

#define OS_StartScheduler vTaskStartScheduler
#define OS_EndSchedule vTaskEndScheduler 
#define OS_TaskYield taskYIELD

#define OS_StackGetMarkSpace uxTaskGetStackHighWaterMark
#define OS_TaskGetRunTime vTaskGetRunTimeStats
#define OS_TaskList vTaskList
#define OS_TaskGetTotal uxTaskGetNumberOfTasks
#define OS_TaskGetName pcTaskGetTaskName

#define OS_TaskCreate xTaskCreate
#define OS_TaskDelete vTaskDelete
#define OS_TaskDelayTick vTaskDelay 
#define OS_TaskDelayMs(x) vTaskDelay((x)/OS_SCHEDULE_PERIOD_FACTOR)//软延时，线程调用，会引起调度//nMs最好为节拍毫秒的整数倍
#define OS_TaskDelaySec(x) vTaskDelay((x)*(1000/OS_SCHEDULE_PERIOD_FACTOR))//软延时，线程调用，会引起调度//nMs最好为节拍毫秒的整数倍
#define OS_TaskSuspendAll vTaskSuspendAll 
#define OS_TaskResumeAll xTaskResumeAll
#define OS_TaskPrioGet uxTaskPriorityGet
#define OS_TaskPrioSet vTaskPrioritySet
#define OS_TaskSuspend vTaskSuspend 
#define OS_TaskResume vTaskResume
#define OS_TaskResume_ISR xTaskResumeFromISR

#define OS_QueueCreate xQueueCreate
#define OS_QueueDelete vQueueDelete
#define OS_QueueSend xQueueSend
#define OS_QueueSendToBack xQueueSendToBack
#define OS_QueueSendToFront xQueueSendToFront
#define OS_QueueReceive xQueueReceive
#define OS_QueuePeek xQueuePeek
#define OS_QueueSend_ISR xQueueSendFromISR
#define OS_QueueSendToBack_ISR xQueueSendToBackFromISR
#define OS_QueueSendToFront_ISR xQueueSendToFrontFromISR
#define OS_QueueReceive_ISR xQueueReceiveFromISR
#define OS_QueuePeek_ISR xQueuePeekFromISR
#define OS_QueueItemWaiting uxQueueMessagesWaiting
#define OS_QueueItemWaiting_ISR uxQueueMessagesWaitingFromISR

#define OS_SemCreateBinary vSemaphoreCreateBinary
#define OS_SemCreateCounting xSemaphoreCreateCounting
#define OS_SemCreateMutex xSemaphoreCreateMutex
#define OS_SemDelete vSemaphoreDelete
#define OS_SemTake xSemaphoreTake
#define OS_SemGive xSemaphoreGive
#define OS_SemTake_ISR xSemaphoreTakeFromISR
#define OS_SemGive_ISR xSemaphoreGiveFromISR


void OS_GetAllTaskInfo(void);

#define OS_TIMER_T os_timer_t
#define OS_TIMER_FUNC_T os_timer_func_t
#define OS_TimerInit os_timer_arm
#define OS_TimerDeinit os_timer_disarm
#define OS_TimerSetCallback os_timer_setfn

#define Q_HeapFreeSize system_get_free_heap_size
#define Q_Malloc zalloc
#define Q_Free free

#endif			


#endif

