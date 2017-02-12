#include "bcomdef.h"
#include "gatt.h"
#include "dl_util.h"

bStatus_t ExtractUuid16(gattAttribute_t* attr, uint16* uuid)
{
	bStatus_t status = SUCCESS;

	if (attr->type.len == ATT_BT_UUID_SIZE)
	{
		*uuid = BUILD_UINT16(attr->type.uuid[0], attr->type.uuid[1]);
	}
	else if (attr->type.len == ATT_UUID_SIZE)
	{
		*uuid = BUILD_UINT16(attr->type.uuid[12], attr->type.uuid[13]);
	}
	else
	{
		*uuid = 0xFFFF;
		status = FAILURE;
	}

	return status;
};
