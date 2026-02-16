// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

#pragma once

/** @file
 *  The only header the end user needs to include */

#include "srp/context.h"
#include "srp/buffer.h"
#include "srp/texture.h"
#include "srp/color.h"
#include "srp/framebuffer.h"
#include "srp/vertex.h"
#include "srp/shaders.h"

#ifdef SRP_INCLUDE_VEC
	#include "srp/vec.h"
#endif
#ifdef SRP_INCLUDE_MAT
	#include "srp/mat.h"
#endif

