/*
 * =======================================================================
 *
 * Gyro Space to Play - A plug-and-play Gyro Space Transformer code
 *
 * Provides functionality for transforming gyro inputs into Local Space,
 * Player Space, and World Space, while handling sensitivity adjustments
 * and gravity alignment. Compatible with both C and C++ environments.
 *
 * Based on the work by Jibb Smart (JoyShockMapper, GamepadMotionHelpers,
 * Fortnite v.19.30's Gyro Aim/Flick Stick implementation)
 *
 * =======================================================================
 */

#ifndef GYROSPACE_HPP
#define GYROSPACE_HPP

#ifdef __cplusplus
#include <cmath>
#include <cstdio>
#include <cstdbool>
#include <cstdint>
extern "C" {
#else
#include "math.h"
#include "stdbool.h"
#include "stdint.h"
#endif

#ifndef EPSILON
#define EPSILON 1e-5
#endif

#undef bool
#define bool s32

 // ---- Debugging and Logging ----
#ifdef ENABLE_DEBUG_LOGS
#ifdef __cplusplus
#include <iostream>
#define DEBUG_LOG(fmt, ...) std::cout << fmt
#else
#define DEBUG_LOG(fmt, ...) printf(fmt, ##__VA_ARGS__)
#endif
#else
#define DEBUG_LOG(fmt, ...)
#endif

// ---- Type Definitions ----

typedef struct {
		float x, y, z;
} Vector3;

typedef struct {
		float m[4][4];
} Matrix4;

// ---- Utility Functions ----

/**
 * Clamps a value between a minimum and maximum.
 */
static inline float clamp(float value, float min, float max) {
		return (value > max) ? max : (value < min) ? min : value;
}

/**
 * Creates a new vector with given x, y, z values.
 */
static inline Vector3 Vec3_New(float x, float y, float z) {
		return (Vector3) { x, y, z };
}

/**
 * Adds two vectors component-wise.
 */
static inline Vector3 Vec3_Add(Vector3 a, Vector3 b) {
		return Vec3_New(a.x + b.x, a.y + b.y, a.z + b.z);
}

/**
 * Subtracts one vector from another.
 */
static inline Vector3 Vec3_Subtract(Vector3 a, Vector3 b) {
		return Vec3_New(a.x - b.x, a.y - b.y, a.z - b.z);
}

/**
 * Scales a vector by a scalar value.
 */
static inline Vector3 Vec3_Scale(Vector3 v, float scalar) {
		return Vec3_New(v.x * scalar, v.y * scalar, v.z * scalar);
}

/**
 * Computes the dot product of two vectors.
 */
static inline float Vec3_Dot(Vector3 a, Vector3 b) {
		return a.x * b.x + a.y * b.y + a.z * b.z;
}

/**
 * Computes the cross product of two vectors.
 */
static inline Vector3 Vec3_Cross(Vector3 a, Vector3 b) {
		return Vec3_New(
				a.y * b.z - a.z * b.y,
				a.z * b.x - a.x * b.z,
				a.x * b.y - a.y * b.x
		);
}

/**
 * Computes the magnitude (length) of a vector.
 */
static inline float Vec3_Magnitude(Vector3 v) {
		return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

/**
 * Normalizes a vector (scales it to unit length).
 */
static inline Vector3 Vec3_Normalize(Vector3 v) {
		float magnitude = Vec3_Magnitude(v);
		if (magnitude < EPSILON) {
				DEBUG_LOG("Warning: Attempted to normalize a near-zero vector.\n");
				return Vec3_New(0.0f, 0.0f, 0.0f);
		}
		return Vec3_Scale(v, 1.0f / magnitude);
}

/**
 * Checks if a vector is near zero (all components close to zero).
 */
static inline bool Vec3_IsZero(Vector3 v) {
		return (fabsf(v.x) < EPSILON && fabsf(v.y) < EPSILON && fabsf(v.z) < EPSILON);
}

/**
 * Performs linear interpolation between two vectors.
 */
static inline Vector3 Vec3_Lerp(Vector3 a, Vector3 b, float t) {
		return Vec3_Add(a, Vec3_Scale(Vec3_Subtract(b, a), t));
}

/**
 * Reflects a vector against a normal.
 */
static inline Vector3 Vec3_Reflect(Vector3 v, Vector3 normal) {
		return Vec3_Subtract(v, Vec3_Scale(normal, 2.0f * Vec3_Dot(v, normal)));
}

// ---- Matrix Operations ----

/**
 * Returns an identity matrix.
 */
static inline Matrix4 Matrix4_Identity() {
		Matrix4 matrix = { {
				{1.0f, 0.0f, 0.0f, 0.0f},
				{0.0f, 1.0f, 0.0f, 0.0f},
				{0.0f, 0.0f, 1.0f, 0.0f},
				{0.0f, 0.0f, 0.0f, 1.0f}
		} };
		return matrix;
}

/**
 * Multiplies a matrix by a vector (row-major order).
 */
static inline Vector3 MultiplyMatrixVector(Matrix4 matrix, Vector3 vector) {
		return Vec3_New(
				matrix.m[0][0] * vector.x + matrix.m[1][0] * vector.y + matrix.m[2][0] * vector.z + matrix.m[3][0],
				matrix.m[0][1] * vector.x + matrix.m[1][1] * vector.y + matrix.m[2][1] * vector.z + matrix.m[3][1],
				matrix.m[0][2] * vector.x + matrix.m[1][2] * vector.y + matrix.m[2][2] * vector.z + matrix.m[3][2]
		);
}

// ---- Global Gravity Vector Management ----

/**
 * Global gravity vector (default set to (0, 1, 0)).
 */
static Vector3 gravNorm = { 0.0f, 1.0f, 0.0f };

/**
 * Updates the global gravity vector using sensor fusion.
 *
 * This blends the existing gravity estimate with the accelerometer reading,
 * ensuring smoother transitions without sudden jumps.
 *
 * @param accel The raw accelerometer reading.
 * @param gyroRotation The gyroscope-based rotation adjustment.
 * @param fusionFactor A smoothing factor (suggest ~0.02 to 0.10).
 */
static inline void UpdateGravityVector(Vector3 accel, Vector3 gyroRotation, float fusionFactor) {
		// Normalize the accelerometer input to ensure valid gravity direction
		Vector3 accelNorm = Vec3_Normalize(accel);

		// Rotate existing gravity vector using gyroscope
		Vector3 rotatedGravity = Vec3_Add(gravNorm, Vec3_Cross(gyroRotation, gravNorm));

		// Blend the rotated gravity vector with accelerometer data using linear interpolation
		gravNorm = Vec3_Lerp(rotatedGravity, accelNorm, fusionFactor);
		gravNorm = Vec3_Normalize(gravNorm);
}

/**
 * Sets and normalizes the gravity vector manually. If the input is zero or NaN,
 * it retains the previous value.
 *
 * @param x X-component of the gravity vector.
 * @param y Y-component of the gravity vector.
 * @param z Z-component of the gravity vector.
 */
static inline void SetGravityVector(float x, float y, float z) {
		if (isnan(x) || isnan(y) || isnan(z)) {
				DEBUG_LOG("Error: Gravity vector contains NaN values. Resetting to default (0,1,0).\n");
				gravNorm = Vec3_New(0.0f, 1.0f, 0.0f);
				return;
		}

		Vector3 newGravNorm = Vec3_New(x, y, z);

		// Ensure gravity vector isn't near-zero
		if (Vec3_Magnitude(newGravNorm) < EPSILON) {
				DEBUG_LOG("Warning: Gravity vector magnitude too small. Retaining previous value.\n");
				return;
		}

		gravNorm = Vec3_Normalize(newGravNorm);
}

/**
 * Resets the gravity vector to its default value (0, 1, 0).
 */
static inline void ResetGravityVector(void) {
		gravNorm = Vec3_New(0.0f, 1.0f, 0.0f);
}

/**
 * Retrieves the current normalized gravity vector.
 *
 * @return The normalized gravity vector.
 */
static inline Vector3 GetGravityVector(void) {
		return gravNorm;
}

// ---- Gyro Space Transformations ----

#ifdef __cplusplus
extern "C" {
#endif

		// Function declarations for gyro space transformations
		Vector3 TransformToLocalSpace(float yaw, float pitch, float roll,
				float yawSensitivity, float pitchSensitivity,
				float rollSensitivity, float couplingFactor);

		Vector3 TransformToPlayerSpace(float yaw_input, float pitch_input, float roll_input,
				Vector3 gravNorm, float yawSensitivity, float pitchSensitivity, float rollSensitivity);

		Vector3 TransformToWorldSpace(float yaw_input, float pitch_input, float roll_input,
				Vector3 gravNorm, float yawSensitivity, float pitchSensitivity, float rollSensitivity);

#ifdef __cplusplus
}
#endif


// Dynamic Orientation Adjustment

/**
 * Dynamically adjusts gyro space transformation based on controller orientation.
 */
Vector3 TransformWithDynamicOrientation(float yaw_input, float pitch_input, float roll_input,
		float yawSensitivity, float pitchSensitivity, float rollSensitivity, bool useWorldSpace) {

		Vector3 gravNorm = GetGravityVector();

		// ---- Compute Tilt Factor ----
		float tiltFactor = fabsf(gravNorm.y); // Higher means Yaw Mode, lower means Roll Mode

		// ---- Blend between modes smoothly ----
		float blendAmount = clamp(tiltFactor, 0.0f, 1.0f); // Ensures smooth switching

		// ---- Corrected Manual Float Lerp ----
		float dynamicYaw = (blendAmount * yaw_input * yawSensitivity) + ((1.0f - blendAmount) * roll_input * rollSensitivity);
		float dynamicPitch = pitch_input * pitchSensitivity;
		float dynamicRoll = (blendAmount * roll_input * rollSensitivity) + ((1.0f - blendAmount) * yaw_input * yawSensitivity);

		// ---- Apply Either Player or World Space Transformation ----
		if (useWorldSpace) {
				return TransformToWorldSpace(dynamicYaw, dynamicPitch, dynamicRoll, gravNorm, yawSensitivity, pitchSensitivity, rollSensitivity);
		}
		else {
				return TransformToPlayerSpace(dynamicYaw, dynamicPitch, dynamicRoll, gravNorm, yawSensitivity, pitchSensitivity, rollSensitivity);
		}

		// ---- Default Fallback ----
		return Vec3_New(0.0f, 0.0f, 0.0f);
}

// Gyro Space Transformation Functions

/**
 * Transforms gyro inputs to Local Space
 */
Vector3 TransformToLocalSpace(float yaw, float pitch, float roll,
		float yawSensitivity, float pitchSensitivity,
		float rollSensitivity, float couplingFactor) {

		// ---- Adjust roll to compensate for yaw-roll coupling ----
		float adjustedRoll = (roll * rollSensitivity) - (yaw * couplingFactor);

		// ---- Apply individual sensitivity scaling ----
		Vector3 rawGyro = Vec3_New(
				(yaw * yawSensitivity) - adjustedRoll,
				pitch * pitchSensitivity,
				roll * rollSensitivity
		);

		// ---- Define Local Space Transformation Matrix ----
		Matrix4 localTransformMatrix = Matrix4_Identity();

		// ---- Apply transformation matrix to gyro input ----
		Vector3 localGyro = MultiplyMatrixVector(localTransformMatrix, rawGyro);

		// ---- Prevent unintended roll drift (Lean Fix) ----
		localGyro.z = -localGyro.z;

		// ---- Return the transformed vector ----
		return localGyro;
}

/**
 * Transforms gyro inputs to Player Space
 */
Vector3 TransformToPlayerSpace(float yaw_input, float pitch_input, float roll_input,
		Vector3 gravNorm, float yawSensitivity, float pitchSensitivity, float rollSensitivity) {

		// ---- Retrieve updated gravity vector ----
		gravNorm = GetGravityVector();

		// ---- Compute Tilt Factor for Dynamic Orientation Adjustment ----
		float tiltFactor = fabsf(gravNorm.y); // Higher means Yaw Mode, lower means Roll Mode
		float blendAmount = clamp(tiltFactor, 0.0f, 1.0f); // Ensures smooth switching

		// ---- Adjust Inputs Dynamically ----
		float adjustedYaw = (blendAmount * yaw_input * yawSensitivity) + ((1.0f - blendAmount) * roll_input * rollSensitivity);
		float adjustedPitch = pitch_input * pitchSensitivity;
		float adjustedRoll = (blendAmount * roll_input * rollSensitivity) + ((1.0f - blendAmount) * yaw_input * yawSensitivity);

		// ---- Define Roll Axis Independent of Gravity ----
		Vector3 rollAxis = Vec3_New(1.0f, 0.0f, 0.0f);

		// ---- Flip roll BEFORE matrix transformation ----
		adjustedRoll = -adjustedRoll;

		// ---- Apply Player View Matrix ----
		Vector3 adjustedGyro = Vec3_New(adjustedYaw, adjustedPitch, adjustedRoll);
		Matrix4 playerViewMatrix = Matrix4_Identity();

		Vector3 playerGyro = MultiplyMatrixVector(playerViewMatrix, adjustedGyro);

		// ---- Return the transformed vector ----
		return playerGyro;
}

/**
 * Transforms gyro inputs to World Space
 */
Vector3 TransformToWorldSpace(float yaw_input, float pitch_input, float roll_input,
		Vector3 gravNorm, float yawSensitivity, float pitchSensitivity, float rollSensitivity) {

		// ---- Retrieve updated gravity vector ----
		gravNorm = GetGravityVector();

		// ---- Compute Tilt Factor for Dynamic Orientation Adjustment ----
		float tiltFactor = fabsf(gravNorm.y); // Higher means Yaw Mode, lower means Roll Mode
		float blendAmount = clamp(tiltFactor, 0.0f, 1.0f); // Ensures smooth switching

		// ---- Adjust Inputs Dynamically ----
		Vector3 rawGyro = Vec3_New(
				pitch_input * pitchSensitivity,
				-yaw_input * yawSensitivity,
				(blendAmount * roll_input * rollSensitivity) + ((1.0f - blendAmount) * yaw_input * yawSensitivity)
		);

		// ---- Flip Roll BEFORE Gravity Alignment ----
		rawGyro.z = -rawGyro.z;

		// ---- Align Pitch Using Gravity ----
		float gravDotPitch = Vec3_Dot(gravNorm, Vec3_New(1.0f, 0.0f, 0.0f));
		Vector3 pitchAxis = Vec3_Subtract(Vec3_New(1.0f, 0.0f, 0.0f), Vec3_Scale(gravNorm, gravDotPitch));

		if (!Vec3_IsZero(pitchAxis)) {
				pitchAxis = Vec3_Normalize(pitchAxis);
		}

		// ---- Calculate Transformed Values ----
		Vector3 worldGyro = Vec3_New(
				-Vec3_Dot(rawGyro, gravNorm),
				Vec3_Dot(rawGyro, pitchAxis),
				rawGyro.z
		);

		// ---- Return the transformed vector ----
		return worldGyro;
}

#ifdef __cplusplus
}
#endif

#endif // GYROSPACE_H
