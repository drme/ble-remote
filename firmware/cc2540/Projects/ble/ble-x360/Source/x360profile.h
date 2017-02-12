#ifndef _BUTTONS_PROFILE_H_
#define _BUTTONS_PROFILE_H_

#define BLE_X360_SERVICE_UUID 0x5FF0
#define BLE_X360_KEYS_UUID    0x5FF1 


#define BLE_BUTTONS_A_CHAR_UUID			0xAFF1
#define BLE_BUTTONS_B_CHAR_UUID			0xAFF2
#define BLE_BUTTONS_X_CHAR_UUID			0xAFF3
#define BLE_BUTTONS_Y_CHAR_UUID			0xAFF4

#define BLE_BUTTONS_UP_CHAR_UUID		0xAFF5
#define BLE_BUTTONS_DN_CHAR_UUID		0xAFF6
#define BLE_BUTTONS_L_CHAR_UUID			0xAFF7
#define BLE_BUTTONS_R_CHAR_UUID			0xAFF8

#define BLE_BUTTONS_LB_CHAR_UUID			0xAFF9
#define BLE_BUTTONS_RB_CHAR_UUID			0xAFFA

#define BLE_BUTTONS_START_CHAR_UUID			0xAFFB
#define BLE_BUTTONS_BACK_CHAR_UUID			0xAFFC
#define BLE_BUTTONS_GUIDE_CHAR_UUID			0xAFFD

#define BLE_BUTTONS_LS_CHAR_UUID			0xAFFE
#define BLE_BUTTONS_RS_CHAR_UUID			0xAFFF

#define BLE_BUTTONS_LX_CHAR_UUID			0xBFF1
#define BLE_BUTTONS_LY_CHAR_UUID			0xBFF2
#define BLE_BUTTONS_RX_CHAR_UUID			0xCFF1
#define BLE_BUTTONS_RY_CHAR_UUID			0xCFF2
#define BLE_BUTTONS_LT_CHAR_UUID			0xDFF1
#define BLE_BUTTONS_RT_CHAR_UUID			0xDFF2

	
	
// Simple Keys Profile Services bit fields
#define BLE_X360_SERVICE				0x00000008

/*

// Callback when a characteristic value has changed
typedef NULL_OK void(*buttonsProfileChange_t)(uint8 paramID);

typedef struct
{
	buttonsProfileChange_t	pfnButtonsProfileChange;	// Called when characteristic value changes
} buttonsProfileCBs_t;

*/

extern bStatus_t X360AddService(uint32 services);

/**
 * Registers the application callback function. Only call this function once.
 *
 * @param appCallbacks - pointer to application callbacks.
 */
//extern bStatus_t buttonsProfile_RegisterAppCBs(buttonsProfileCBs_t* appCallbacks);

extern void X360Init();

#endif
