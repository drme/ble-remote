#ifndef _DL_UTIL_H_
#define _DL_UTIL_H_

#define DL_BASE_UUID_128(uuid)	0x48, 0xe0, 0x0c, 0x9e, 0x6d, 0xd6, 0x4a, 0x99, 0x92, 0x16, 0xcd, 0xf6, LO_UINT16(uuid), HI_UINT16(uuid), 0x39, 0xa7
#define DL_UUID_SIZE			ATT_UUID_SIZE
#define DL_UUID(uuid)			DL_BASE_UUID_128(uuid)

bStatus_t ExtractUuid16(gattAttribute_t* attr, uint16* value);

#endif
