// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

/** @file
 * 	@ingroup Math
 *  `mat4` definition */

#pragma once

#include "srp/vec.h"

/** @ingroup Math
 *  @{ */

/** Represents a 4x4 matrix of `float`s, stores data in row-major order */
typedef struct mat4 { float data[4][4]; } mat4;

/** Multiply mat4 by vec4
 *  @param a Pointer to a matrix A
 *  @param b Vector B
 *  @return The product A*B */
vec4 mat4MultiplyVec4(const mat4* restrict a, vec4 b);
/** Multiply two 4x4 matrices
 *  @param a Pointer to a matrix A
 *  @param b Pointer to a matrix B 
 *  @return The product A*B */
mat4 mat4MultiplyMat4(const mat4* restrict a, const mat4* restrict b);

/** Construct a 4x4 identity matrix
 *  @return 4x4 identity matrix */
mat4 mat4Identity();

/** Construct a 4x4 matrix that scales the X, Y and Z dimensions
 *  @param x,y,z Scaling coefficients for X, Y, Z dimensions respectively
 *  @return Scale matrix */
mat4 mat4Scale(float x, float y, float z);
/** Construct a 4x4 matrix that translates the points
 *  @param x,y,z Translating coefficient for X, Y, Z dimensions respectively
 *  @return Translation matrix */
mat4 mat4Translate(float x, float y, float z);
/** Construct a 4x4 matrix that rotates the points around an axis
 *  @param angle Angle (in radians) by which to rotate around the axis
 *  @param axis Rotation axis
 *  @see https://songho.ca/opengl/gl_rotate.html
 *  @return Rotation matrix */
mat4 mat4Rotate(float angle, vec3 axis);

/** Construct a 4x4 matrix that translates, rotates the points and scales the
 *  X, Y, and Z dimenstions
 *  @see mat4Scale() mat4Translate() mat4ConstructRotate()
 *  @return TRS matrix */
mat4 mat4ConstructTRS(
	float transX, float transY, float transZ,
	float rotataionX, float rotataionY, float rotataionZ,
	float scaleX, float scaleY, float scaleZ
);

/** Construct a 4x4 view matrix
 *  @param cameraX,cameraY,cameraZ Camera position
 *  @param rotationX,rotationY,rotationZ Camera rotation along each dimension
 *  @param scaleX,scaleY,scaleZ Camera "zoom" along each dimension
 *  @return View matrix */
mat4 mat4ConstructView(
	float cameraX, float cameraY, float cameraZ,
	float rotationX, float rotationY, float rotationZ,
	float scaleX, float scaleY, float scaleZ
);

/** Construct a 4x4 lookAt matrix
 *  @param eye The world-space camera position
 *  @param target The world-space point to look at
 *  @param up The up vector. May not be normalized
 *  @return The lookAt matrix */
mat4 mat4LookAt(vec3 eye, vec3 target, vec3 up);

/** Construct a 4x4 orthogonal projection matrix, converting the right-handed
 *  system to the left-handed system (OpenGL-style). The orthogonal projection
 *  matrix transforms arbitrary rectangular parallelepiped into an NDC cube.
 *  @see https://songho.ca/opengl/gl_projectionmatrix.html#ortho
 *  @param x_min,y_min,z_min Coordinates of the "min" point defining a rectangular parallelepiped
 *  @param x_max,y_max,z_max Coordinates of the "max" point defining a rectangular parallelepiped
 *  @return Orthogonal projection matrix */
mat4 mat4ConstructOrthogonalProjection(
	float x_min, float x_max,
	float y_min, float y_max,
	float z_min, float z_max
);

/** Construct a 4x4 perspective projection matrix, converting the right-handed
 *  system to the left handed system (OpenGL-style) The perspective projection
 *  matrix transforms the perspective frustum to an NDC cube.
 *  @see https://songho.ca/opengl/gl_projectionmatrix.html#perspective
 *  @param x_min_near,x_max_near Minimum and maximum X values on the near plane
 *  @param y_min_near,y_max_near Minimum and maximum Y on the near plane
 *  @param z_near,z_far Distance to the near and far planes
 *  @return Perspective projection matrix */
mat4 mat4ConstructPerspectiveProjection(
	float x_min_near, float x_max_near,
	float y_min_near, float y_max_near,
	float z_near, float z_far
);

/** Construct a 4x4 perspective projection matrix, converting the right-handed
 *  system to the left handed system (OpenGL-style) The perspective projection
 *  matrix transforms the perspective frustum to an NDC cube.
 *  @see https://songho.ca/opengl/gl_projectionmatrix.html#fov
 *  @param fovX The horizontal field of view angle, in degrees
 *  @param aspectRatio The width / height ratio of the render target
 *  @param z_near,z_far Distance to the near and far planes
 *  @return Perspective projection matrix */
mat4 mat4ConstructPerspectiveProjectionFOV(
	float fovX, float aspectRatio, float z_near, float z_far
);

/** @} */  // ingroup Math
