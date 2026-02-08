// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

#include "message_callback_p.h"
#include "context.h"

void srpNewContext(SRPContext* pContext)
{
	pContext->messageCallback = NULL;
	pContext->messageCallbackUserParameter = NULL;
	pContext->interpolationMode = SRP_INTERPOLATION_MODE_PERSPECTIVE;
}

void srpContextSetP(SRPContextParameter contextParameter, void* data)
{
	switch (contextParameter)
	{
	case SRP_CONTEXT_MESSAGE_CALLBACK:
		srpContext.messageCallback = data;
		return;
	case SRP_CONTEXT_MESSAGE_CALLBACK_USER_PARAMETER:
		srpContext.messageCallbackUserParameter = data;
		return;
	default:
		srpMessageCallbackHelper(
			SRP_MESSAGE_ERROR, SRP_MESSAGE_SEVERITY_HIGH, __func__,
			"Unknown type (%i)", contextParameter
		);
		return;
	}
}

void srpContextSetI(SRPContextParameter contextParameter, int data)
{
	switch (contextParameter)
	{
	case SRP_CONTEXT_INTERPOLATION_MODE:
		srpContext.interpolationMode = data;
	default:
		srpMessageCallbackHelper(
			SRP_MESSAGE_ERROR, SRP_MESSAGE_SEVERITY_HIGH, __func__,
			"Unknown type (%i)", contextParameter
		);
		return;
	}
}

void* srpContextGetP(SRPContextParameter contextParameter)
{
	switch (contextParameter)
	{
	case SRP_CONTEXT_MESSAGE_CALLBACK:
		return srpContext.messageCallback;
	case SRP_CONTEXT_MESSAGE_CALLBACK_USER_PARAMETER:
		return srpContext.messageCallbackUserParameter;
	default:
		srpMessageCallbackHelper(
			SRP_MESSAGE_ERROR, SRP_MESSAGE_SEVERITY_HIGH, __func__,
			"Unknown type (%i)", contextParameter
		);
		return NULL;
	}
}

int srpContextGetI(SRPContextParameter contextParameter)
{
	switch (contextParameter)
	{
	case SRP_CONTEXT_INTERPOLATION_MODE:
		return srpContext.interpolationMode;
	default:
		srpMessageCallbackHelper(
			SRP_MESSAGE_ERROR, SRP_MESSAGE_SEVERITY_HIGH, __func__,
			"Unknown type (%i)", contextParameter
		);
		return 0;
	}
}

