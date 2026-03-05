// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

/** @file
 *  @ingroup Context_internal
 *  Message callback declaration */

#pragma once

#include "srp/message_callback.h"

/** @ingroup Context_internal
 *  @{ */

/** A helper function used by the library implementation to make callbacks easier
 *  @param[in] type Message type
 *  @param[in] severity Message severity
 *  @param[in] sourceFunction Function that is the source of this message
 *  @param[in] format `printf`-like format string
 *  @param[in] ... Values passed to the format string
 *  @see `SRPMessageCallbackFunc` */
void srpMessageCallbackHelper(
	SRPMessageType type, SRPMessageSeverity severity,
	const char* sourceFunction, const char* format, ...
);

/** @} */  // ingroup Context_internal
