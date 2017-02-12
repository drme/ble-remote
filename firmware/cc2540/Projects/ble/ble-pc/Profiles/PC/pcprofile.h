#ifndef _PC_PROFILE_H_
#define _PC_PROFILE_H_

#ifdef __cplusplus
extern "C"
{
#endif

#define BLE_BUTTONS_SERVICE_UUID			0x5FF0
#define BLE_BUTTONS_SWITCH1_CHAR_UUID		0xFFF1
#define BLE_BUTTONS_SWITCH2_CHAR_UUID		0xFFF2
#define BLE_BUTTONS_LED1_CHAR_UUID			0xFFE1
#define BLE_BUTTONS_LED2_CHAR_UUID			0xFFE2

// Simple Keys Profile Services bit fields
#define BLE_BUTTONS_PROFILE_SERVICE			0x00000008

// Callback when a characteristic value has changed
typedef NULL_OK void(*buttonsProfileChange_t)(uint8 paramID);

typedef struct
{
	buttonsProfileChange_t	pfnButtonsProfileChange;	// Called when characteristic value changes
} buttonsProfileCBs_t;

extern uint8 ledCharSwitch1;

/**
 * Initializes the Simple GATT Profile service by registering GATT attributes with the GATT server.
 *
 * @param services - services to add. This is a bit map and can contain more than one service.
 */
extern bStatus_t ButtonsProfileAddService(uint32 services);

/**
 * Registers the application callback function. Only call this function once.
 *
 * @param appCallbacks - pointer to application callbacks.
 */
extern bStatus_t ButtonsProfileRegisterAppCBs(buttonsProfileCBs_t* appCallbacks);

extern void ButtonsInit();

extern void ButtonsUpdate();

#ifdef __cplusplus
}
#endif

#endif
