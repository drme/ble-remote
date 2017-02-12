#ifndef _BLE_LEDS_PROFILE_H_
#define _BLE_LEDS_PROFILE_H_

#ifdef __cplusplus

extern "C"
{
#endif

#define BLE_LEDS_SERVICE_UUID    0xAFF0
#define BLE_LEDS_PIXEL_CHAR_UUID 0xAFF1
#define BLE_LEDS_PROFILE_SERVICE 0x00000002

// Callback when a characteristic value has changed
typedef NULL_OK void(*ledsProfileChange_t)(uint8 paramID);

typedef struct
{
	ledsProfileChange_t	pfnLedsProfileChange;	// Called when characteristic value changes
} ledsProfileCBs_t;

/**
 * Initializes the GATT Profile service by registering GATT attributes with the GATT server.
 *
 * @param services - services to add. This is a bit map and can contain more than one service.
 */
extern bStatus_t LedsProfileAddService(uint32 services);

/**
 * Registers the application callback function. Only call this function once.
 *
 * @param appCallbacks - pointer to application callbacks.
 */
extern bStatus_t LedsProfileRegisterAppCBs(ledsProfileCBs_t* appCallbacks);

extern void LedsProfileInit();
extern void LedsStart();
extern void LedsStop();
extern void LedsUpdateStatus();
extern void LedsSetPixel(uint8 pixelId, uint8 r, uint8 g, uint8 b);

#ifdef __cplusplus
}
#endif

#endif
