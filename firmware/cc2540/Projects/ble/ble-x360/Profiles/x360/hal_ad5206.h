#ifndef _HAL_AD5206_H_
#define _HAL_AD5206_H_

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Initialize AD5206
 */
extern void HalAD5206Init(void);
/**
 * Sets cahannmel value.
 */
extern void HalAD5206SetValue(uint8 channel, uint8 value);

#ifdef __cplusplus
}
#endif

#endif
