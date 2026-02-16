// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

/** @file
 *  Message callback declaration and helper functions */

#pragma once

#include "srp/message_callback.h"

/** A helper function used by the library implementation to make callbacks easier
 *  @param[in] type Message type
 *  @param[in] severity Message severity
 *  @param[in] sourceFunction Function that is the source of this message
 *  @param[in] format `printf`-like format string
 *  @param[in] ... Values passed to the format string
 *  @see `SRPMessageCallbackType` */
void srpMessageCallbackHelper(
	SRPMessageType type, SRPMessageSeverity severity,
	const char* sourceFunction, const char* format, ...
);

