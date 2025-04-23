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

 // Ensure `s32` and `u32` are correctly defined
#ifndef s32
#define s32 int32_t
#endif

#ifndef u32
#define u32 uint32_t
#endif

// Apply correct boolean type definitions
#ifndef bool
#define bool s32
#endif

#ifndef ubool
#define ubool u32
#endif

#ifndef EPSILON
#define EPSILON 1e-5
#endif

 // Debugging and Logging 
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

// Type Definitions 

typedef struct {
	float x, y, z;
} Vector3;

typedef struct {
	float m[4][4];
} Matrix4;

// Utility Functions 

/* Scalar Operations */

/**
 * Clamps a value between a minimum and maximum.
 */
static inline float clamp(float value, float min, float max) {
	return (value > max) ? max : (value < min) ? min : value;
}

/* Vector Creation */

/**
 * Creates a new vector with given x, y, z values.
 */
static inline Vector3 Vec3_New(float x, float y, float z) {
	return (Vector3) { x, y, z };
}

/* Basic Vector Operations */

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

/* Advanced Vector Operations */

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
		return Vec3_Scale(v, 1.0f);
	}
	return Vec3_Scale(v, 1.0f / magnitude);
}

/**
 * Checks if a vector is near zero (all components close to zero).
 */
static inline bool Vec3_IsZero(Vector3 v) {
	return (fabsf(v.x) < EPSILON && fabsf(v.y) < EPSILON && fabsf(v.z) < EPSILON);
}

/* Specialized Vector Operations */

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

// Matrix Operations 

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
 * Creates a transformation matrix based on the gravity vector.
 * The gravity vector determines the "up" direction.
 */
static inline Matrix4 Matrix4_FromGravity(Vector3 gravNorm) {
	gravNorm = Vec3_Normalize(gravNorm);

	Vector3 upVector = Vec3_New(0.0f, 1.0f, 0.0f);
	Vector3 zAxis = Vec3_Cross(upVector, gravNorm);
	if (Vec3_IsZero(zAxis)) zAxis = Vec3_New(0.0f, 0.0f, 1.0f);

	Matrix4 matrix = { {
		{1.0f, 0.0f, 0.0f, 0.0f},
		{0.0f, gravNorm.y, gravNorm.z, 0.0f},
		{zAxis.x, -gravNorm.z, gravNorm.y, 0.0f},
		{0.0f, 0.0f, 0.0f, 1.0f}
	} };
	return matrix;
}

/**
 * Multiplies a matrix by a vector (row-major order).
 */
static inline Vector3 MultiplyMatrixVector(Matrix4 matrix, Vector3 vector) {
	// Assume w = 1.0 for 3D vectors
	float w = 1.0f;

	return Vec3_New(
		matrix.m[0][0] * vector.x + matrix.m[1][0] * vector.y + matrix.m[2][0] * vector.z + matrix.m[3][0] * w,
		matrix.m[0][1] * vector.x + matrix.m[1][1] * vector.y + matrix.m[2][1] * vector.z + matrix.m[3][1] * w,
		matrix.m[0][2] * vector.x + matrix.m[1][2] * vector.y + matrix.m[2][2] * vector.z + matrix.m[3][2] * w
	);
}

// Global Gravity Vector Management 

/**
 * Global gravity vector (default set to (0, 1, 0)).
 */
static Vector3 gravNorm = { 0.0f, 1.0f, 0.0f };

/**
 * Updates the global gravity vector using sensor fusion.
 */
static inline void UpdateGravityVector(Vector3 accel, Vector3 gyroRotation, float fusionFactor) {
	// Validate fusionFactor
	if (fusionFactor < 0.0f || fusionFactor > 1.0f) {
		DEBUG_LOG("Error: Invalid fusionFactor (%f). Must be between 0.0 and 1.0.\n", fusionFactor);
		return;
	}

	// Validate inputs for NaN values
	if (isnan(accel.x) || isnan(accel.y) || isnan(accel.z) ||
		isnan(gyroRotation.x) || isnan(gyroRotation.y) || isnan(gyroRotation.z)) {
		DEBUG_LOG("Error: NaN values detected in sensor inputs. Skipping update.\n");
		return;
	}

	// Normalize the accelerometer input to ensure valid gravity direction
	Vector3 accelNorm = Vec3_Normalize(accel);

	// Rotate existing gravity vector using gyroscope
	Vector3 rotatedGravity = Vec3_Add(gravNorm, Vec3_Cross(gyroRotation, gravNorm));

	// Blend the rotated gravity vector with accelerometer data using linear interpolation
	gravNorm = Vec3_Lerp(rotatedGravity, accelNorm, fusionFactor);

	// Normalize the updated gravity vector
	if (!Vec3_IsZero(gravNorm)) {
		gravNorm = Vec3_Normalize(gravNorm);
	}
	else {
		DEBUG_LOG("Warning: Gravity vector became near-zero. Resetting to default (0,1,0).\n");
		gravNorm = Vec3_New(0.0f, 1.0f, 0.0f);
	}

	DEBUG_LOG("UpdateGravityVector called - gravNorm = (%f, %f, %f)\n", gravNorm.x, gravNorm.y, gravNorm.z);
}

/**
 * Sets the gravity vector manually and ensures normalization.
 */
static inline void SetGravityVector(float x, float y, float z) {
	// Validate inputs
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

	// Normalize and update the gravity vector
	gravNorm = Vec3_Normalize(newGravNorm);

	DEBUG_LOG("SetGravityVector called - gravNorm = (%f, %f, %f)\n", gravNorm.x, gravNorm.y, gravNorm.z);
}

/**
 * Retrieves the current normalized gravity vector.
 */
static inline Vector3 GetGravityVector(void) {
	DEBUG_LOG("GetGravityVector called - gravNorm = (%f, %f, %f)\n", gravNorm.x, gravNorm.y, gravNorm.z);
	return gravNorm;
}


// Dynamic Orientation Adjustment

/**
 * Adapts gyro input for switching between normal controller grip and handheld-style grip.
 * This function takes yaw, pitch, and roll inputs and adjusts them to maintain consistent orientation.
 */

Vector3 TransformWithDynamicOrientation(float yaw_input, float pitch_input, float roll_input) {

	// Validate Inputs 
	if (isnan(yaw_input) || isnan(pitch_input) || isnan(roll_input)) {
		DEBUG_LOG("Error: NaN detected in inputs. Returning zero vector.\n");
		return Vec3_New(0.0f, 0.0f, 0.0f);
	}

	// Clamp Inputs to Prevent Extreme Values 
	yaw_input = clamp(yaw_input, -360.0f, 360.0f);
	pitch_input = clamp(pitch_input, -360.0f, 360.0f);
	roll_input = clamp(roll_input, -360.0f, 360.0f);

	// Debug Logs 
	DEBUG_LOG("Dynamic Orientation Adjustment (Raw Inputs): Yaw = %f, Pitch = %f, Roll = %f\n", yaw_input, pitch_input, roll_input);

	// Combine Inputs Into a Single Vector 
	Vector3 combinedVector = Vec3_New(yaw_input, pitch_input, roll_input);

	// Normalize Output to Maintain Orientation Consistency 
	Vector3 normalizedVector = Vec3_Normalize(combinedVector);

	// Return the Adjusted Orientation 
	return normalizedVector;
}

// Gyro Space Transformation Function

/**
 * These functions transform gyro inputs into different spaces (Local, Player, World)
 * while handling sensitivity adjustments and gravity alignment.
 */


 // C wrapper for gyro transformation functions, ensuring compatibility with both C and C++.
 // Prevents name mangling, allowing seamless function calls between languages.
#ifdef __cplusplus
extern "C" {
#endif

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

/**
 * Transforms gyro inputs to Local Space.
 *
 * Converts raw gyro input into direct motion scaling.
 * Preserves natural gyro responsiveness for intuitive movement.
 */

Vector3 TransformToLocalSpace(float yaw, float pitch, float roll,
	float yawSensitivity, float pitchSensitivity,
	float rollSensitivity, float couplingFactor) {

	// Adjust roll to compensate for yaw-roll coupling  
	float adjustedRoll = (roll * rollSensitivity) - (yaw * couplingFactor);

	// Apply individual sensitivity scaling  
	Vector3 localGyro = Vec3_New(
		yaw * yawSensitivity - adjustedRoll,
		pitch * pitchSensitivity,
		roll * rollSensitivity
	);

	// Return the transformed vector  
	return localGyro;
}

/**
 * Transforms gyro inputs to Player Space.
 *
 * Adjusts gyro input relative to the player's perspective.
 * Ensures consistent aiming and movement, adapting to gravity alignment.
 */

Vector3 TransformToPlayerSpace(float yaw_input, float pitch_input, float roll_input,
	Vector3 gravNorm, float yawSensitivity, float pitchSensitivity, float rollSensitivity) {

	//  Validate Gravity Vector 
	if (Vec3_IsZero(gravNorm)) {
		DEBUG_LOG("Warning: Gravity vector is near-zero. Resetting to default (0,1,0).\n");
		gravNorm = Vec3_New(0.0f, 1.0f, 0.0f);
	}
	else {
		gravNorm = Vec3_Normalize(gravNorm);
	}

	//  Compute World-Aligned Yaw 
	float worldYaw = yaw_input * gravNorm.y + roll_input * gravNorm.z;

	//  Compute Local Combined Yaw 
	float combinedYaw = Vec3_Magnitude(Vec3_New(yaw_input, roll_input, 0.0f)); // Treat yaw & roll as rotation about a single axis

	//  Determine Yaw Direction Based on World Yaw 
	float yawDirection = (worldYaw >= 0) ? 1.0f : -1.0f;

	//  Apply Yaw Relaxation Factor 
	float yawRelaxFactor = 1.41;  // Expands buffer zone for natural transitions
	float adjustedYaw = yawDirection * fminf(fabsf(worldYaw) * yawRelaxFactor, combinedYaw) * yawSensitivity;

	//  Compute Local Pitch 
	float adjustedPitch = pitch_input * pitchSensitivity;

	//  Compute Player View Matrix 
	Matrix4 playerViewMatrix = Matrix4_FromGravity(gravNorm);

	//  Apply Player View Matrix 
	Vector3 playerGyro = Vec3_New(adjustedYaw, adjustedPitch, roll_input * rollSensitivity);
	playerGyro = MultiplyMatrixVector(playerViewMatrix, playerGyro);

	//  Return the transformed vector 
	return playerGyro;
}

/**
 * Transforms gyro inputs to World Space.
 *
 * Aligns gyro input with the game world for precise aiming.
 * Maintains spatial consistency, ensuring smooth transitions between perspectives.
 */

Vector3 TransformToWorldSpace(float yaw_input, float pitch_input, float roll_input,
    Vector3 gravNorm, float yawSensitivity, float pitchSensitivity, float rollSensitivity) {

    // Validate Gravity Vector  
    if (Vec3_IsZero(gravNorm)) {
        gravNorm = Vec3_New(0.0f, 1.0f, 0.0f);
    } else {
        gravNorm = Vec3_Normalize(gravNorm);
    }

    // Reduce hard spring effect by adjusting gravity alignment influence
    float worldYaw = yaw_input * gravNorm.y + roll_input * gravNorm.z;

    // Compute Side Reduction Factor with less aggressive clamping  
    float flatness = fabsf(gravNorm.y); // 1 when controller is flat
    float upness = fabsf(gravNorm.z);   // 1 when controller is upright
    float sideReduction = clamp((fmaxf(flatness, upness) - 0.125f) / 0.25f, 0.0f, 1.0f); // Adjusted divisor to smooth transitions

    // Project Pitch Axis onto Gravity Plane with stabilized calculations  
    float gravDotPitchAxis = gravNorm.x;
    Vector3 pitchVector = Vec3_Subtract(Vec3_New(1.0f, 0.0f, 0.0f), Vec3_Scale(gravNorm, gravDotPitchAxis));

    // Normalize if not parallel to gravity  
    if (!Vec3_IsZero(pitchVector)) {
        pitchVector = Vec3_Normalize(pitchVector);
    }

    // Adjust Pitch with refined damping to prevent sudden diagonal prioritization  
    float adjustedPitch = sideReduction * Vec3_Dot(Vec3_New(pitch_input, 0.0f, roll_input), pitchVector) * pitchSensitivity;

    // Compute World View Matrix  
    Matrix4 worldViewMatrix = Matrix4_FromGravity(gravNorm);

    // Apply refined world space transformation  
    Vector3 adjustedGyro = Vec3_New(worldYaw * yawSensitivity, adjustedPitch, roll_input * rollSensitivity);
    Vector3 worldGyro = MultiplyMatrixVector(worldViewMatrix, adjustedGyro);

    // Return the transformed vector  
    return worldGyro;
}

#ifdef __cplusplus
}
#endif

#endif // GYROSPACE_H
