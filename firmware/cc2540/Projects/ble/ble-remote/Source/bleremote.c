#include "bcomdef.h"
#include "OSAL.h"
#include "OSAL_PwrMgr.h"
#include "gatt.h"
#include "hci.h"
#include "gapgattserver.h"
#include "gattservapp.h"
#include "devinfoservice.h"
#include "peripheral.h"
#include "gapbondmgr.h"
#include "bleremote.h"
#include "battservice.h"
#include "bleirprofile.h"
#include "osal_snv.h"
#include "OnBoard.h"
#include "dl_util.h"

#if defined FEATURE_OAD
	#include "oad.h"
	#include "oad_target.h"
#endif

#define REMOTE_PERIODIC_EVENT_PERIOD       50000                      /* How often to perform periodic event. */
#define DEFAULT_ADVERTISING_INTERVAL       1600 * 5                   /* What is the advertising interval when device is discoverable (units of 625us, 160=100ms). */
#define DEFAULT_DISCOVERABLE_MODE          GAP_ADTYPE_FLAGS_GENERAL   /* General discoverable mode advertises indefinitely. */
#define DEFAULT_DESIRED_MIN_CONN_INTERVAL  32                         /* Minimum connection interval (units of 1.25ms, 80=100ms) if automatic parameter update request is enabled. */
#define DEFAULT_DESIRED_MAX_CONN_INTERVAL  320                        /* Maximum connection interval (units of 1.25ms, 800=1000ms) if automatic parameter update request is enabled. */
#define DEFAULT_DESIRED_SLAVE_LATENCY      0                          /* Slave latency to use if automatic parameter update request is enabled. */
#define DEFAULT_DESIRED_CONN_TIMEOUT       1000                       /* Supervision timeout value (units of 10ms, 1000=10s) if automatic parameter update request is enabled. */
#define DEFAULT_ENABLE_UPDATE_REQUEST      TRUE                       /* Whether to enable automatic parameter update request when a connection is formed. */
#define DEFAULT_CONN_PAUSE_PERIPHERAL      6                          /* Connection Pause Peripheral time value (in seconds). */

static uint8 mainTaskId;
static gaprole_States_t gapProfileState = GAPROLE_INIT;

static uint8 scanResponseData[] =
{
	12,
	GAP_ADTYPE_LOCAL_NAME_COMPLETE,
	'B', 'L', 'E', '-', 'T', 'E', 'L', 'L', 'Y', '-', '2',
	0x05,
	GAP_ADTYPE_SLAVE_CONN_INTERVAL_RANGE,
	LO_UINT16(DEFAULT_DESIRED_MIN_CONN_INTERVAL),
	HI_UINT16(DEFAULT_DESIRED_MIN_CONN_INTERVAL),
	LO_UINT16(DEFAULT_DESIRED_MAX_CONN_INTERVAL),
	HI_UINT16(DEFAULT_DESIRED_MAX_CONN_INTERVAL),
	0x02,
	GAP_ADTYPE_POWER_LEVEL,
	0
};

static uint8 advertisementData[] =
{
	0x02,
	GAP_ADTYPE_FLAGS,
	DEFAULT_DISCOVERABLE_MODE | GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED,
	DL_UUID_SIZE + 1,
	GAP_ADTYPE_128BIT_MORE,
	DL_UUID(BLE_IR_SERVICE_UUID),
};

// GAP GATT Attributes
static uint8 attDeviceName[GAP_DEVICE_NAME_LEN] = "BLE-TELLY-2";

void RemotePeripheralInit(uint8 taskId)
{
	if (P2_1 == 1)
	{
		if (SUCCESS == osal_snv_read(DEVICE_PASSWORD_FLASH_ADDRESS, sizeof(uint32), &devicePassword))
		{
		}
		else
		{
			devicePassword = 0x00000000;
		}
	}
	else
	{
		devicePassword = 0x00000000;
		
		if (SUCCESS == osal_snv_write(DEVICE_PASSWORD_FLASH_ADDRESS, sizeof(uint32), &devicePassword))
		{
		}
	}

	mainTaskId = taskId;

	GAP_SetParamValue(TGAP_CONN_PAUSE_PERIPHERAL, DEFAULT_CONN_PAUSE_PERIPHERAL);
	
	// Setup the GAP Peripheral Role Profile
	{
		uint8 initial_advertising_enable = TRUE;

		// By setting this to zero, the device will go into the waiting state after
		// being discoverable for 30.72 second, and will not being advertising again
		// until the enabler is set back to TRUE
		uint16 gapRole_AdvertOffTime = 0;

		uint8 enable_update_request = DEFAULT_ENABLE_UPDATE_REQUEST;
		uint16 desired_min_interval = DEFAULT_DESIRED_MIN_CONN_INTERVAL;
		uint16 desired_max_interval = DEFAULT_DESIRED_MAX_CONN_INTERVAL;
		uint16 desired_slave_latency = DEFAULT_DESIRED_SLAVE_LATENCY;
		uint16 desired_conn_timeout = DEFAULT_DESIRED_CONN_TIMEOUT;

		// Set the GAP Role Parameters
		GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED,		sizeof(uint8),				&initial_advertising_enable);
		GAPRole_SetParameter(GAPROLE_ADVERT_OFF_TIME,		sizeof(uint16),				&gapRole_AdvertOffTime);
		GAPRole_SetParameter(GAPROLE_SCAN_RSP_DATA,			sizeof(scanResponseData),	scanResponseData);
		GAPRole_SetParameter(GAPROLE_ADVERT_DATA,			sizeof(advertisementData),	advertisementData);
		GAPRole_SetParameter(GAPROLE_PARAM_UPDATE_ENABLE,	sizeof(uint8),				&enable_update_request);
		GAPRole_SetParameter(GAPROLE_MIN_CONN_INTERVAL,		sizeof(uint16),				&desired_min_interval);
		GAPRole_SetParameter(GAPROLE_MAX_CONN_INTERVAL,		sizeof(uint16),				&desired_max_interval);
		GAPRole_SetParameter(GAPROLE_SLAVE_LATENCY,			sizeof(uint16),				&desired_slave_latency);
		GAPRole_SetParameter(GAPROLE_TIMEOUT_MULTIPLIER,	sizeof(uint16),				&desired_conn_timeout);
	}

	// Set the GAP Characteristics
	GGS_SetParameter(GGS_DEVICE_NAME_ATT, GAP_DEVICE_NAME_LEN, attDeviceName);

	// Set advertising interval
	{
    	uint16 advInt = DEFAULT_ADVERTISING_INTERVAL;

		GAP_SetParamValue(TGAP_LIM_DISC_ADV_INT_MIN, advInt);
		GAP_SetParamValue(TGAP_LIM_DISC_ADV_INT_MAX, advInt);
		GAP_SetParamValue(TGAP_GEN_DISC_ADV_INT_MIN, advInt);
		GAP_SetParamValue(TGAP_GEN_DISC_ADV_INT_MAX, advInt);
	}

	// Setup the GAP Bond Manager
	{
		uint32 passkey = devicePassword;
		uint8 pairMode = GAPBOND_PAIRING_MODE_WAIT_FOR_REQ;
		uint8 mitm = TRUE;
		uint8 ioCap = GAPBOND_IO_CAP_DISPLAY_ONLY;
		uint8 bonding = TRUE;

		GAPBondMgr_SetParameter(GAPBOND_DEFAULT_PASSCODE,	sizeof(uint32),	&passkey);
		GAPBondMgr_SetParameter(GAPBOND_PAIRING_MODE,		sizeof(uint8),	&pairMode);
		GAPBondMgr_SetParameter(GAPBOND_MITM_PROTECTION,	sizeof(uint8),	&mitm);
		GAPBondMgr_SetParameter(GAPBOND_IO_CAPABILITIES,	sizeof(uint8),	&ioCap);
		GAPBondMgr_SetParameter(GAPBOND_BONDING_ENABLED,	sizeof(uint8),	&bonding);
	}

	// Initialize GATT attributes
	GGS_AddService(GATT_ALL_SERVICES);			// GAP
	GATTServApp_AddService(GATT_ALL_SERVICES);	// GATT attributes
	DevInfo_AddService();						// Device Information Service

	IrProfileAddService(GATT_ALL_SERVICES);
	Batt_AddService();

	#if defined FEATURE_OAD
		VOID OADTarget_AddService();			// OAD Profile
	#endif

	IrInit(mainTaskId);

	// Enable clock divide on halt. This reduces active current while radio is active and CC254x MCU is halted.
	HCI_EXT_ClkDivOnHaltCmd(HCI_EXT_ENABLE_CLK_DIVIDE_ON_HALT);
	
	// Setup a delayed profile startup
	osal_set_event(mainTaskId, REMOTE_START_DEVICE_EVENT);
};

static void PerformPeriodicTask(void)
{
	if (gapProfileState == GAPROLE_CONNECTED)
	{
		Batt_MeasLevel();
	}
};

static void ProcessOSALMessage(osal_event_hdr_t* message)
{
	switch (message->event)
	{
		default:
			break;
	}
};

static void StateNotificationCallBack(gaprole_States_t newState)
{
	switch (newState)
	{
		case GAPROLE_STARTED:
			{
				uint8 ownAddress[B_ADDR_LEN];
				uint8 systemId[DEVINFO_SYSTEM_ID_LEN];

				GAPRole_GetParameter(GAPROLE_BD_ADDR, ownAddress);

				// use 6 bytes of device address for 8 bytes of system ID value
				systemId[0] = ownAddress[0];
				systemId[1] = ownAddress[1];
				systemId[2] = ownAddress[2];

				// set middle bytes to zero
				systemId[4] = 0x00;
				systemId[3] = 0x00;

				// shift three bytes up
				systemId[7] = ownAddress[5];
				systemId[6] = ownAddress[4];
				systemId[5] = ownAddress[3];

				DevInfo_SetParameter(DEVINFO_SYSTEM_ID, DEVINFO_SYSTEM_ID_LEN, systemId);
			}
			break;
		case GAPROLE_ADVERTISING:
			break;
		case GAPROLE_CONNECTED:
			osal_start_timerEx(mainTaskId, REMOTE_PERIODIC_EVENT, REMOTE_PERIODIC_EVENT_PERIOD);
			break;
		case GAPROLE_WAITING:
			break;
		case GAPROLE_WAITING_AFTER_TIMEOUT:
			break;
		case GAPROLE_ERROR:
			break;
		default:
			break;
	}

	gapProfileState = newState;
};

static gapRolesCBs_t peripheralCallBacks =
{
	StateNotificationCallBack,
	NULL
};

static gapBondCBs_t bondManagerCallBacks =
{
	NULL,
	NULL
};

uint16 RemotePeripheralProcessEvent(uint8 taskId, uint16 events)
{
	if (events & SYS_EVENT_MSG)
	{
		uint8* message;

		if ((message = osal_msg_receive(mainTaskId)) != NULL)
		{
			ProcessOSALMessage((osal_event_hdr_t*)message);

			osal_msg_deallocate(message);
		}

		return (events ^ SYS_EVENT_MSG);
	}

	if (events & REMOTE_START_DEVICE_EVENT)
	{
		GAPRole_StartDevice(&peripheralCallBacks);
		GAPBondMgr_Register(&bondManagerCallBacks);

		osal_start_timerEx(mainTaskId, REMOTE_PERIODIC_EVENT, REMOTE_PERIODIC_EVENT_PERIOD);

		return (events ^ REMOTE_START_DEVICE_EVENT);
	}

	if (events & REMOTE_PERIODIC_EVENT)
	{
		if (gapProfileState == GAPROLE_CONNECTED)	  
		{  
			if (REMOTE_PERIODIC_EVENT_PERIOD)
			{
				osal_start_timerEx(mainTaskId, REMOTE_PERIODIC_EVENT, REMOTE_PERIODIC_EVENT_PERIOD);
			}

			PerformPeriodicTask();
		}
		
		return (events ^ REMOTE_PERIODIC_EVENT);
	}

	if (events & CHANGE_TYPE_EVENT)
	{
		IrChangeType();
		
		return (events ^ CHANGE_TYPE_EVENT);
	}

	if (events & REPEAT_COMMAND1_EVENT)
	{
		IrRepeatCommand();
		
		return (events ^ REPEAT_COMMAND1_EVENT);
	}

	if (events & REPEAT_COMMAND2_EVENT)
	{
		IrRepeatCommand();
		
		return (events ^ REPEAT_COMMAND2_EVENT);
	}
	
	return 0;
};
