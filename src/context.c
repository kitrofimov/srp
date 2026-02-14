// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

#include "message_callback_p.h"
#include "context.h"
#include "arena_p.h"

void srpNewContext(SRPContext* pContext)
{
	pContext->messageCallback = NULL;
	pContext->messageCallbackUserParameter = NULL;
	pContext->interpolationMode = SRP_INTERPOLATION_MODE_PERSPECTIVE;
	pContext->frontFace = SRP_FRONT_FACE_CCW;
	pContext->cullFace = SRP_CULL_FACE_NONE;
	pContext->arena = newArena(SRP_DEFAULT_ARENA_CAPACITY);
}

void srpContextSetMessageCallback(SRPMessageCallbackType callback)
{
	srpContext.messageCallback = callback;
}

SRPMessageCallbackType srpContextGetMessageCallback()
{
	return srpContext.messageCallback;
}

void srpContextSetP(SRPContextParameter contextParameter, void* data)
{
	switch (contextParameter)
	{
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
		return;
	case SRP_CONTEXT_FRONT_FACE:
		srpContext.frontFace = data;
		return;
	case SRP_CONTEXT_CULL_FACE:
		srpContext.cullFace = data;
		return;
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
	case SRP_CONTEXT_FRONT_FACE:
		return srpContext.frontFace;
	case SRP_CONTEXT_CULL_FACE:
		return srpContext.cullFace;
	default:
		srpMessageCallbackHelper(
			SRP_MESSAGE_ERROR, SRP_MESSAGE_SEVERITY_HIGH, __func__,
			"Unknown type (%i)", contextParameter
		);
		return 0;
	}
}

