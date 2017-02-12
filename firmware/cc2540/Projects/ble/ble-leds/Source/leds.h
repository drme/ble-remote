#ifndef _BLE_LEDS_H_
#define _BLE_LEDS_H_

#ifdef __cplusplus
extern "C"
{
#endif

#define SBP_START_DEVICE_EVT       0x0001
#define SBP_PERIODIC_EVT           0x0002
#define SBP_ADV_IN_CONNECTION_EVT  0x0004
#define BATT_PERIODIC_EVT          0x0008
  
/**
 * Task Initialization for the BLE Application
 */
extern void LedsInit(uint8 taskId);

/**
 * Task Event Processor for the BLE Application
 */
extern uint16 LedsProcessEvent(uint8 taskId, uint16 events);

#ifdef __cplusplus
}
#endif

#endif
