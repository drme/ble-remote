#include "bcomdef.h"
#include "OSAL.h"
#include "OSAL_PwrMgr.h"
#include "OnBoard.h"
#include "gatt.h"
#include "hci.h"
#include "gapgattserver.h"
#include "gattservapp.h"
#include "devinfoservice.h"
#include "peripheral.h"
#include "gapbondmgr.h"
#include "fan.h"
#include "battservice.h"
#include "fan_profile.h"
#include "fan_control.h"
#include "osal_snv.h"
#include "dl_util.h"

#if defined FEATURE_OAD
	#include "oad.h"
	#include "oad_target.h"
#endif

#define DEFAULT_ADVERTISING_INTERVAL        1600                  /* What is the advertising interval when device is discoverable (units of 625us, 160=100ms) */
#define DEFAULT_ENABLE_UPDATE_REQUEST       TRUE                  /* Whether to enable automatic parameter update request when a connection is formed */
#define DEFAULT_DESIRED_MIN_CONN_INTERVAL   6                     /* Minimum connection interval (units of 1.25ms, 80=100ms) if automatic parameter update request is enabled */
#define DEFAULT_DESIRED_MAX_CONN_INTERVAL   24                    /* Maximum connection interval (units of 1.25ms, 800=1000ms) if automatic parameter update request is enabled */
#define DEFAULT_DESIRED_SLAVE_LATENCY       0                     /* Slave latency to use if automatic parameter update request is enabled */
#define DEFAULT_DESIRED_CONN_TIMEOUT        3                     /* Supervision timeout value (units of 10ms, 1000=10s) if automatic parameter update request is enabled */
#define	DEFAULT_FAN_NAME                    'P', 'i', 'n', 'k', '-', 'F', 'a', 'n', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0

static uint8 fanTaskId;   // Task ID for internal task/event processing

/**
 * GAP - SCAN RSP data (max size = 31 bytes)
 */
static uint8 scanResponseData[] =
{
	0x16,
	GAP_ADTYPE_LOCAL_NAME_COMPLETE,
	DEFAULT_FAN_NAME,
	0x05,
	GAP_ADTYPE_SLAVE_CONN_INTERVAL_RANGE,
	LO_UINT16(DEFAULT_DESIRED_MIN_CONN_INTERVAL),
	HI_UINT16(DEFAULT_DESIRED_MIN_CONN_INTERVAL),
	LO_UINT16(DEFAULT_DESIRED_MAX_CONN_INTERVAL),
	HI_UINT16(DEFAULT_DESIRED_MAX_CONN_INTERVAL)
};

/**
 * GAP - Advertisement data (max size = 31 bytes, though this is
 * best kept short to conserve power while advertisting)
 */ 
static uint8 advertisementData[] =
{
	0x02,
	GAP_ADTYPE_FLAGS,
	GAP_ADTYPE_FLAGS_GENERAL | GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED,
	DL_UUID_SIZE + 1,
	GAP_ADTYPE_128BIT_MORE,
	DL_UUID(BLE_FAN_SERVICE_UUID)
}; 

static void FanProcessOSALMessage(osal_event_hdr_t* message);
static void FanPeripheralStateNotificationCallBack(gaprole_States_t newState);

/**
 * GAP Role Callbacks.
 */
static gapRolesCBs_t fanPeripheralCallBacks =
{
	FanPeripheralStateNotificationCallBack,
	NULL
};

/**
 * GAP Bond Manager Callbacks.
 */
static gapBondCBs_t fanBondManagerCallBacks =
{
	NULL,								// Passcode callback (not used by application)
	NULL								// Pairing / Bonding state Callback (not used by application)
};

void FanInit(uint8 taskId)
{
	fanTaskId = taskId;

	FanControlInit(taskId, scanResponseData + 2);
	
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

		{
			GAPRole_SetParameter(GAPROLE_ADVERT_DATA,			sizeof(advertisementData),	advertisementData);
		}
		
		GAPRole_SetParameter(GAPROLE_PARAM_UPDATE_ENABLE,	sizeof(uint8),				&enable_update_request);
		GAPRole_SetParameter(GAPROLE_MIN_CONN_INTERVAL,		sizeof(uint16),				&desired_min_interval);
		GAPRole_SetParameter(GAPROLE_MAX_CONN_INTERVAL,		sizeof(uint16),				&desired_max_interval);
		GAPRole_SetParameter(GAPROLE_SLAVE_LATENCY,			sizeof(uint16),				&desired_slave_latency);
		GAPRole_SetParameter(GAPROLE_TIMEOUT_MULTIPLIER,	sizeof(uint16),				&desired_conn_timeout);
	}

	GGS_SetParameter(GGS_DEVICE_NAME_ATT, GAP_DEVICE_NAME_LEN, scanResponseData + 2);

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
		uint32 passkey = 0x00; // passkey "000000"
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
	GGS_AddService(GATT_ALL_SERVICES);            // GAP
	GATTServApp_AddService(GATT_ALL_SERVICES);    // GATT attributes
	DevInfo_AddService();                         // Device Information Service

	FanProfileAddService(GATT_ALL_SERVICES);
	
	#if defined FEATURE_OAD
		OADTarget_AddService();              // OAD Profile
	#endif

	Batt_AddService();

	// Enable clock divide on halt.
	// This reduces active current while radio is active and CC254x MCU is halted.
	HCI_EXT_ClkDivOnHaltCmd(HCI_EXT_ENABLE_CLK_DIVIDE_ON_HALT);
	
	#if defined(DC_DC_P0_7)
		// Enable stack to toggle bypass control on TPS62730 (DC/DC converter)
		HCI_EXT_MapPmIoPortCmd( HCI_EXT_PM_IO_PORT_P0, HCI_EXT_PM_IO_PORT_PIN7 );
	#endif

	// Setup a delayed profile startup
	osal_set_event(fanTaskId, FAN_START_DEVICE_EVT);
};

uint16 FanProcessEvent(uint8 taskId, uint16 events)
{
	if (events & SYS_EVENT_MSG)
	{
		uint8* message = NULL;

		if ((message = osal_msg_receive(fanTaskId)) != NULL)
		{
			FanProcessOSALMessage((osal_event_hdr_t*)message);
			osal_msg_deallocate(message);
		}

		return (events ^ SYS_EVENT_MSG);
	}

	if (events & FAN_START_DEVICE_EVT)
	{
		GAPRole_StartDevice(&fanPeripheralCallBacks);
		GAPBondMgr_Register(&fanBondManagerCallBacks);
	
		return (events ^ FAN_START_DEVICE_EVT);
	}

	if (events & FAN_UPDATE_STATUS_EVT)
	{
		FanUpdateStatus();
	
		return (events ^ FAN_UPDATE_STATUS_EVT);
	}

	return 0;
};

static void FanProcessOSALMessage(osal_event_hdr_t* message)
{
	switch (message->event)
	{
		default:
			break;
	}
};

static void FanPeripheralStateNotificationCallBack(gaprole_States_t newState)
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
			FanConnected(fanTaskId);
			break;
		case GAPROLE_WAITING:
			{
				FanDisConnected();
				uint8 advertEnabled = FALSE;
				GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(uint8), &advertEnabled); 
				GAPRole_SetParameter(GAPROLE_SCAN_RSP_DATA, sizeof(scanResponseData), scanResponseData);
				GGS_SetParameter(GGS_DEVICE_NAME_ATT, GAP_DEVICE_NAME_LEN, scanResponseData + 2);
				advertEnabled = TRUE;
				GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(uint8), &advertEnabled);			
			}
			break;
		case GAPROLE_WAITING_AFTER_TIMEOUT:
			break;
		case GAPROLE_ERROR:
			break;
		default:
			break;
	}
};
