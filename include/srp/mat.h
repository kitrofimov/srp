// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

/** @file
 *  `mat4` and mat4d */

#pragma once

#include "srp/vec.h"

/** @ingroup Matrix
 *  @{ */

/** Represents a 4x4 matrix of `float`s, stores data in row-major order */
typedef struct mat4 { float data[4][4]; } mat4;
/** Represents a 4x4 matrix of `double`s, stores data in row-major order */
typedef struct mat4d { double data[4][4]; } mat4d;

/** Get the `index`th column of the matrix A
 *  @param a Pointer to a matrix A
 *  @param index Index of a column to get
 *  @return indexth column of the matrix A */
vec4d mat4dGetColumn(const mat4d* a, uint8_t index);
/** Set the `index`th column of the *a matrix to column
 *  @param a Pointer to a matrix
 *  @param column A new column
 *  @param index Index of a column to modify */
void mat4dSetColumn(mat4d* a, vec4d column, uint8_t index);

/** Multiply mat4d by vec4d
 *  @param a Pointer to a matrix A
 *  @param b Vector B
 *  @return The product A*B */
vec4d mat4dMultiplyVec4d(const mat4d* a, vec4d b);
/** Multiply two 4x4 matrices
 *  @param a Pointer to a matrix A
 *  @param b Pointer to a matrix B 
 *  @return The product A*B */
mat4d mat4dMultiplyMat4d(const mat4d* a, const mat4d* b);

/** Construct a 4x4 identity matrix
 *  @return 4x4 identity matrix */
mat4d mat4dConstructIdentity();

/** Construct a 4x4 matrix that scales the X, Y and Z dimensions
 *  @param x,y,z Scaling coefficients for X, Y, Z dimensions respectively
 *  @return Scale matrix */
mat4d mat4dConstructScale(double x, double y, double z);
/** Construct a 4x4 matrix that translates the points
 *  @param x,y,z Translating coefficient for X, Y, Z dimensions respectively
 *  @return Translation matrix */
mat4d mat4dConstructTranslate(double x, double y, double z);
/** Construct a 4x4 matrix that rotates the points
 *  @param x,y,z Angle (in radians) by which the points are rotated
 *  around X, Y, Z axis respectively
 *  @return Rotation matrix */
mat4d mat4dConstructRotate(double x, double y, double z);

/** Construct a 4x4 matrix that translates, rotates the points and scales the
 *  X, Y, and Z dimenstions
 *  @see mat4dConstructScale() mat4dConstructTranslate() mat4dConstructRotate()
 *  @return TRS matrix */
mat4d mat4dConstructTRS(
	double transX, double transY, double transZ,
	double rotataionX, double rotataionY, double rotataionZ,
	double scaleX, double scaleY, double scaleZ
);
/** Construct a 4x4 view matrix
 *  @param cameraX,cameraY,cameraZ Camera position
 *  @param rotationX,rotationY,rotationZ Camera rotation along each dimension
 *  @param scaleX,scaleY,scaleZ Camera "zoom" along each dimension
 *  @return View matrix */
mat4d mat4dConstructView(
	double cameraX, double cameraY, double cameraZ,
	double rotataionX, double rotataionY, double rotataionZ,
	double scaleX, double scaleY, double scaleZ
);

/** Construct a 4x4 orthogonal projection matrix.
 *  The orthogonal projection matrix transforms arbitrary rectangular parallelepiped
 *  (defined by 2 points: "min" and "max" ones) into an NDC cube, so it can be drawn
 *  in screen space
 *  @param x_min,y_min,z_min Coordinates of the "min" point defining a rectangular parallelepiped
 *  @param x_max,y_max,z_max Coordinates of the "max" point defining a rectangular parallelepiped
 *  @return Orthogonal projection matrix */
mat4d mat4dConstructOrthogonalProjection(
	double x_min, double x_max,
	double y_min, double y_max,
	double z_min, double z_max
);
/** Construct a 4x4 perspective projection matrix.
 *  The perspective projection matrix transforms the perspective frustum (defined
 *  by the distance to near and far planes and by "min" and "max" points on the
 *  near plane) to a unit cube, so it can be drawn in screen space
 *  @param x_min_near,x_max_near Minimum and maximum X values on the near plane
 *  @param y_min_near,y_max_near Minimum and maximum Y on the near plane
 *  @param z_near,z_far Distance to the near and far planes
 *  @return Perspective projection matrix */
mat4d mat4dConstructPerspectiveProjection(
	double x_min_near, double x_max_near,
	double y_min_near, double y_max_near,
	double z_near, double z_far
);

/** @} */  // defgroup Matrix

