#include "hal_types.h"
#include "OSAL.h"
#include "OSAL_Tasks.h"
#include "hal_drivers.h"
#include "ll.h"
#include "hci_tl.h"

#if defined(OSAL_CBTIMER_NUM_TASKS)
	#include "osal_cbTimer.h"
#endif

#include "l2cap.h"
#include "gap.h"
#include "gapgattserver.h"
#include "gapbondmgr.h"
#include "gatt.h"
#include "gattservapp.h"
#include "peripheral.h"
#include "bleremote.h"

const pTaskEventHandlerFn tasksArr[] =
{
	LL_ProcessEvent,
	Hal_ProcessEvent,
	HCI_ProcessEvent,

#if defined(OSAL_CBTIMER_NUM_TASKS)
	OSAL_CBTIMER_PROCESS_EVENT(osal_CbTimerProcessEvent),
#endif

L2CAP_ProcessEvent,
	GAP_ProcessEvent,
	GATT_ProcessEvent,
	SM_ProcessEvent,
	GAPRole_ProcessEvent,
	GAPBondMgr_ProcessEvent,
	GATTServApp_ProcessEvent,
	RemotePeripheralProcessEvent
};

const uint8 tasksCnt = sizeof(tasksArr) / sizeof(tasksArr[0]);
uint16* tasksEvents;

void osalInitTasks(void)
{
	uint8 taskID = 0;

	tasksEvents = (uint16*)osal_mem_alloc(sizeof(uint16) * tasksCnt);

	osal_memset(tasksEvents, 0, (sizeof(uint16) * tasksCnt));

	LL_Init(taskID++);
	Hal_Init(taskID++);
	HCI_Init(taskID++);

#if defined (OSAL_CBTIMER_NUM_TASKS)
	osal_CbTimerInit(taskID);
	taskID += OSAL_CBTIMER_NUM_TASKS;
#endif

	L2CAP_Init( taskID++ );
	GAP_Init(taskID++);
	GATT_Init(taskID++);
	SM_Init(taskID++);
	GAPRole_Init(taskID++);
	GAPBondMgr_Init(taskID++);
	GATTServApp_Init(taskID++);
	RemotePeripheralInit(taskID);
};
