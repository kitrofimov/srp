// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

#include <stdio.h>
#include "utils/message_callback_p.h"
#include "type_p.h"

size_t srpSizeofType(SRPType type)
{
	switch (type)
	{
		case TYPE_UINT8:
			return sizeof(uint8_t);
		case TYPE_UINT16:
			return sizeof(uint16_t);
		case TYPE_UINT32:
			return sizeof(uint32_t);
		case TYPE_UINT64:
			return sizeof(uint64_t);
		case TYPE_FLOAT:
			return sizeof(float);
		case TYPE_DOUBLE:
			return sizeof(double);
		default:
		{
			srpMessageCallbackHelper(
				SRP_MESSAGE_ERROR, SRP_MESSAGE_SEVERITY_HIGH, __func__,
				"Unknown type (%i)", type
			);
			return 0;
		}
	}
}

