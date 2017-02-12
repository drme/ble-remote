#ifndef _BLE_PC_H_
#define _BLE_PC_H_

#ifdef __cplusplus
extern "C"
{
#endif

// BLE Peripheral Task Events
#define SBP_START_DEVICE_EVT			0x0001
#define SBP_PERIODIC_EVT				0x0002
#define SBP_ADV_IN_CONNECTION_EVT		0x0004
#define BATT_PERIODIC_EVT				0x0008

/**
 * Task Initialization for the BLE Application
 */
extern void BLESensorPeripheral_Init(uint8 task_id);

/**
 * Task Event Processor for the BLE Application
 */
extern uint16 BLESensorPeripheral_ProcessEvent(uint8 task_id, uint16 events);

#ifdef __cplusplus
}
#endif

#endif
