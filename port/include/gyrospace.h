/*
 * =======================================================================
 *
 * Gyro Space to Play - A plug-and-play Gyro Space Transformer code
 *
 * Provides functionality for transforming gyro inputs into Local Space,
 * Player Space, and World Space, while handling sensitivity adjustments
 * and gravity alignment. Compatible with both C and C++ environments.
 *
 * Also compatible with GamepadMotionHelpers!
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

 // ---- Debugging and Logging ----
#ifdef ENABLE_DEBUG_LOGS
#ifdef __cplusplus
#include <iostream>
#define DEBUG_LOG(fmt, ...) std::cout << fmt << std::endl
#else
#include <stdio.h>
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

static inline float clamp(float value, float min, float max) {
		return (value > max) ? max : (value < min) ? min : value;
}

static inline Vector3 Vec3_New(float x, float y, float z) {
		Vector3 v = { x, y, z };
		return v;
}

static inline Vector3 Vec3_Subtract(Vector3 a, Vector3 b) {
		return Vec3_New(a.x - b.x, a.y - b.y, a.z - b.z);
}

static inline Vector3 Vec3_Scale(Vector3 v, float scalar) {
		return Vec3_New(v.x * scalar, v.y * scalar, v.z * scalar);
}

static inline float Vec3_Dot(Vector3 a, Vector3 b) {
		return a.x * b.x + a.y * b.y + a.z * b.z;
}

static inline Vector3 Vec3_Cross(Vector3 a, Vector3 b) {
		return Vec3_New(
				a.y * b.z - a.z * b.y,
				a.z * b.x - a.x * b.z,
				a.x * b.y - a.y * b.x
		);
}

static inline float Vec3_Magnitude(Vector3 v) {
		return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

static inline Vector3 Vec3_Normalize(Vector3 v) {
		float lengthSquared = v.x * v.x + v.y * v.y + v.z * v.z;
		if (lengthSquared < EPSILON) {
				DEBUG_LOG("Warning: Attempted to normalize a near-zero vector.\n");
				return Vec3_New(0.0f, 0.0f, 0.0f);
		}
		return Vec3_Scale(v, 1.0f / sqrtf(lengthSquared));
}

static inline bool Vec3_IsZero(Vector3 v) {
		return (fabsf(v.x) < EPSILON && fabsf(v.y) < EPSILON && fabsf(v.z) < EPSILON);
}

static inline Vector3 Vec3_Lerp(Vector3 a, Vector3 b, float t) {
		return Vec3_New(
				a.x + t * (b.x - a.x),
				a.y + t * (b.y - a.y),
				a.z + t * (b.z - a.z)
		);
}

static inline Vector3 Vec3_Reflect(Vector3 v, Vector3 normal) {
		return Vec3_Subtract(v, Vec3_Scale(normal, 2.0f * Vec3_Dot(v, normal)));
}

static inline Matrix4 Matrix4_Identity() {
		Matrix4 matrix = { {{1.0f, 0.0f, 0.0f, 0.0f},
											 {0.0f, 1.0f, 0.0f, 0.0f},
											 {0.0f, 0.0f, 1.0f, 0.0f},
											 {0.0f, 0.0f, 0.0f, 1.0f}} };
		return matrix;
}

// ---- Matrix Operations ----

/**
 * Multiplies a vector by a matrix (row-major).
 * This implementation must be in the header for `static inline` to work.
 */
static inline Vector3 MultiplyMatrixVector(Matrix4 matrix, Vector3 vector) {
		return Vec3_New(
				matrix.m[0][0] * vector.x + matrix.m[1][0] * vector.y + matrix.m[2][0] * vector.z,
				matrix.m[0][1] * vector.x + matrix.m[1][1] * vector.y + matrix.m[2][1] * vector.z,
				matrix.m[0][2] * vector.x + matrix.m[1][2] * vector.y + matrix.m[2][2] * vector.z
		);
}

// ---- Global Gravity Vector Management ----

/**
 * Global gravity vector (default set to (0, 1, 0)).
 */
static Vector3 gravNorm = { 0.0f, 1.0f, 0.0f };

/**
 * Sets and normalizes the gravity vector. If the input is zero or NaN,
 * it retains the current value.
 *
 * @param x X-component of the gravity vector.
 * @param y Y-component of the gravity vector.
 * @param z Z-component of the gravity vector.
 */
static inline void SetGravityVector(float x, float y, float z) {
		if (isnan(x) || isnan(y) || isnan(z)) {
				DEBUG_LOG("Error: Gravity vector contains NaN values. Retaining previous value.\n");
				return;
		}
		float magnitude = sqrtf(x * x + y * y + z * z);
		if (magnitude < EPSILON) {  // Prevent division by zero
				DEBUG_LOG("Warning: Gravity vector magnitude is near zero. Retaining previous value.\n");
				return;
		}
		gravNorm = Vec3_Scale(Vec3_New(x, y, z), 1.0f / magnitude); // Normalize
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



// ---- GamepadMotionHelper ----

/**
 * GamepadMotionHelper compatibility
 *
 * This header provides optional integration with
 * Player Gyro Space's Player Space and World Space
 * transformer.
 *
 * https://github.com/JibbSmart/GamepadMotionHelpers
 *
 * IMPORTANT: if using GamepadMotion.cpp (Valkirie fork): You must create a corresponding
 * GamepadMotion.h file. This header serves as a wrapper to provide C compatibility
 * and access to GamepadMotion.cpp functionality for both C and C++ environments.
 *
 * https://github.com/Valkirie/GamepadMotionHelpers
 *
 */

#ifdef ENABLE_GAMEPAD_MOTION_HELPERS

#ifdef __cplusplus
#include "GamepadMotion.hpp"
#pragma message("GamepadMotionHelpers (C++) is enabled and GamepadMotion.cpp is being used for Player Space and World Space transformations.")
#else
#include "GamepadMotion.h" // C wrapper
#pragma message("GamepadMotionHelpers (C) is enabled and GamepadMotion.cpp is being used for Player Space and World Space transformations.")
#endif

#ifdef __cplusplus

 // ---- Independent Gravity Handling for GamepadMotionHelper ----

static Vector3 helperGravNorm = { 0.0f, 1.0f, 0.0f }; // Separate gravity for GamepadMotionHelper

static inline void SetHelperGravityVector(float x, float y, float z) {
		if (x == 0.0f && y == 0.0f && z == 0.0f) { // Check for zero vector
				DEBUG_LOG("Warning: Gravity vector is zero. Resetting to default.\n");
				helperGravNorm = Vec3_New(0.0f, 1.0f, 0.0f);
				return;
		}

		if (isnan(x) || isnan(y) || isnan(z)) {
				DEBUG_LOG("Error: Gravity vector contains NaN values.\n");
				return;
		}

		float magnitude = sqrtf(x * x + y * y + z * z);
		if (magnitude < EPSILON) {
				DEBUG_LOG("Warning: Gravity vector magnitude is near zero. Resetting to default.\n");
				helperGravNorm = Vec3_New(0.0f, 1.0f, 0.0f);
				return;
		}

		helperGravNorm = Vec3_Scale(Vec3_New(x, y, z), 1.0f / magnitude);
}

static inline void ResetHelperGravityVector(void) {
		SetHelperGravityVector(0.0f, 1.0f, 0.0f); // Delegate to SetHelperGravityVector
}

static inline Vector3 GetHelperGravityVector(void) {
		return helperGravNorm;
}

/**
 * Uses GamepadMotionHelper for Player Space transformation.
 */
inline Vector3 IntegratePlayerSpaceGyro(const GamepadMotionHelpers::MotionData& motionData, float yawRelaxFactor = 1.41f) {
		DEBUG_LOG("GamepadMotionHelper: IntegratePlayerSpaceGyro invoked.\n");
		float x = 0.0f, y = 0.0f;

		GamepadMotionHelpers::CalculatePlayerSpaceGyro(
				x, y,
				motionData.Gyro.x, motionData.Gyro.y, motionData.Gyro.z,
				helperGravNorm.x, helperGravNorm.y, helperGravNorm.z, // Independent gravity
				yawRelaxFactor
		);

		return Vec3_New(x, y, 0.0f);
}

/**
 * Uses GamepadMotionHelper for World Space transformation.
 */
inline Vector3 IntegrateWorldSpaceGyro(const GamepadMotionHelpers::MotionData& motionData, float sideReductionThreshold = 0.125f) {
		DEBUG_LOG("GamepadMotionHelper: IntegrateWorldSpaceGyro invoked.\n");
		float x = 0.0f, y = 0.0f;

		GamepadMotionHelpers::CalculateWorldSpaceGyro(
				x, y,
				motionData.Gyro.x, motionData.Gyro.y, motionData.Gyro.z,
				helperGravNorm.x, helperGravNorm.y, helperGravNorm.z, // Independent gravity
				sideReductionThreshold
		);

		return Vec3_New(x, y, 0.0f);
}

#else // C Implementation using GamepadMotion.h

 /**
	* Uses GamepadMotionHelper for Player Space transformation (C wrapper).
	*/
static inline Vector3 IntegratePlayerSpaceGyro(GamepadMotion* motion, float yawRelaxFactor) {
		if (!motion) {
				DEBUG_LOG("Error: Motion object is NULL in Player Space Gyro.\n");
				return Vec3_New(0.0f, 0.0f, 0.0f);
		}

		DEBUG_LOG("GamepadMotionHelper: IntegratePlayerSpaceGyro invoked (C wrapper).\n");
		float x = 0.0f, y = 0.0f;

		ProcessMotion(motion, motion->gyroX, motion->gyroY, motion->gyroZ,
				motion->accelX, motion->accelY, motion->accelZ, motion->deltaTime);
		GetPlayerSpaceGyro(motion, &x, &y, yawRelaxFactor);

		return Vec3_New(x, y, 0.0f);
}

/**
 * Uses GamepadMotionHelper for World Space transformation (C wrapper).
 */
static inline Vector3 IntegrateWorldSpaceGyro(GamepadMotion* motion, float sideReductionThreshold) {
		if (!motion) {
				DEBUG_LOG("Error: Motion object is NULL in World Space Gyro.\n");
				return Vec3_New(0.0f, 0.0f, 0.0f);
		}

		DEBUG_LOG("GamepadMotionHelper: IntegrateWorldSpaceGyro invoked (C wrapper).\n");
		float x = 0.0f, y = 0.0f;

		ProcessMotion(motion, motion->gyroX, motion->gyroY, motion->gyroZ,
				motion->accelX, motion->accelY, motion->accelZ, motion->deltaTime);
		GetWorldSpaceGyro(motion, &x, &y, sideReductionThreshold);

		return Vec3_New(x, y, 0.0f);
}

#endif // C++ or C
#endif // ENABLE_GAMEPAD_MOTION_HELPERS


// ---- Gyro Space Transformations ----

#ifdef __cplusplus
namespace GyroSpace {
#endif

		// Function declarations for gyro space transformations
		Vector3 TransformToLocalSpace(float yaw, float pitch, float roll,
				float yawSensitivity, float pitchSensitivity,
				float rollSensitivity, float couplingFactor);

		Vector3 TransformToPlayerSpace(float yaw_input, float pitch_input, float roll_input,
				Vector3 gravNorm,
				float yawSensitivity, float pitchSensitivity,
				float rollSensitivity);

		Vector3 TransformToWorldSpace(float yaw_input, float pitch_input, float roll_input,
				Vector3 gravNorm,
				float yawSensitivity, float pitchSensitivity,
				float rollSensitivity);

#ifdef __cplusplus
} // namespace GyroSpace
#endif

#ifdef __cplusplus
using GyroSpace::TransformToLocalSpace;
using GyroSpace::TransformToPlayerSpace;
using GyroSpace::TransformToWorldSpace;
#endif

/**
 * Transforms gyro inputs to Local Space using a transformation matrix.
 */
Vector3 TransformToLocalSpace(float yaw, float pitch, float roll,
		float yawSensitivity, float pitchSensitivity, float rollSensitivity, float couplingFactor) {
		// ---- Adjust Roll and Combine Inputs ----
		float adjustedRoll = (roll * rollSensitivity) - (yaw * couplingFactor);
		Vector3 rawGyro = Vec3_New(
				yaw * yawSensitivity - adjustedRoll,
				pitch * pitchSensitivity,
				0.0f
		);

		// ---- Define Local Transformation Matrix ----
		Matrix4 localTransformMatrix = Matrix4_Identity(); // Identity matrix for simplicity

		// ---- Apply Transformation ----
		Vector3 localGyro = MultiplyMatrixVector(localTransformMatrix, rawGyro);

		// ---- Lean Fix for Roll ----
		localGyro.z = -localGyro.z;

		// ---- Return the Transformed Vector ----
		return localGyro;
}

/**
 * Transforms gyro inputs to Player Space, considering gravity alignment.
 */
Vector3 TransformToPlayerSpace(float yaw_input, float pitch_input, float roll_input, Vector3 gravNorm,
		float yawSensitivity, float pitchSensitivity, float rollSensitivity) {
#ifdef ENABLE_GAMEPAD_MOTION_HELPERS
		DEBUG_LOG("Using GamepadMotionHelper for Player Space Transformation.\n");

		// Initialize GamepadMotion object
		GamepadMotion* motion = CreateGamepadMotion();

		// Ensure gravNorm is valid
		if (Vec3_IsZero(gravNorm)) {
				DEBUG_LOG("Warning: gravNorm is zero. Defaulting to (0, 1, 0).\n");
				gravNorm = Vec3_New(0.0f, 1.0f, 0.0f);
		}
		gravNorm = Vec3_Normalize(gravNorm);

		// Process motion to align inputs with GamepadMotionHelper
		ProcessMotion(motion, yaw_input, pitch_input, roll_input,
				gravNorm.x, gravNorm.y, gravNorm.z, 0.0f); // Set deltaTime to 0.0f for alignment

		// Use Player Space gyro calculation via wrapper
		float x = 0.0f, y = 0.0f;
		GetPlayerSpaceGyro(motion, &x, &y, yawSensitivity);

		// Cleanup GamepadMotion object
		DeleteGamepadMotion(motion);

		return Vec3_New(x, y, 0.0f);
#else
		// Fallback logic for Player Space transformation
		if (Vec3_IsZero(gravNorm)) {
				gravNorm = Vec3_New(0.0f, 1.0f, 0.0f); // Default gravity vector
				DEBUG_LOG("Warning: gravNorm was zero, defaulting to (0, 1, 0)\n");
		}
		gravNorm = Vec3_Normalize(gravNorm);

		float adjustedYaw = (yaw_input * yawSensitivity) * gravNorm.y + pitch_input * gravNorm.z;
		float adjustedPitch = pitch_input * pitchSensitivity;
		float adjustedRoll = (roll_input * rollSensitivity) * gravNorm.x;

		Vector3 adjustedGyro = Vec3_New(adjustedYaw, adjustedPitch, adjustedRoll);

		Matrix4 playerViewMatrix = Matrix4_Identity(); // Placeholder matrix
		return MultiplyMatrixVector(playerViewMatrix, adjustedGyro);
#endif
}


/**
 * Transforms gyro inputs to World Space, considering gravity orientation.
 */
Vector3 TransformToWorldSpace(float yaw_input, float pitch_input, float roll_input, Vector3 gravNorm,
		float yawSensitivity, float pitchSensitivity, float rollSensitivity) {
#ifdef ENABLE_GAMEPAD_MOTION_HELPERS
		DEBUG_LOG("Using GamepadMotionHelper for World Space Transformation.\n");

		// Initialize GamepadMotion object
		GamepadMotion* motion = CreateGamepadMotion();

		// Ensure gravNorm is valid
		if (Vec3_IsZero(gravNorm)) {
				DEBUG_LOG("Warning: gravNorm is zero. Defaulting to (0, 1, 0).\n");
				gravNorm = Vec3_New(0.0f, 1.0f, 0.0f);
		}
		gravNorm = Vec3_Normalize(gravNorm);

		// Process motion to align inputs with GamepadMotionHelper
		ProcessMotion(motion, yaw_input, pitch_input, roll_input,
				gravNorm.x, gravNorm.y, gravNorm.z, 0.0f); // Set deltaTime to 0.0f for alignment

		// Use World Space gyro calculation via wrapper
		float x = 0.0f, y = 0.0f;
		GetWorldSpaceGyro(motion, &x, &y, rollSensitivity);

		// Cleanup GamepadMotion object
		DeleteGamepadMotion(motion);

		return Vec3_New(x, y, 0.0f);
#else
		// Fallback logic for World Space transformation
		gravNorm = Vec3_Normalize(Vec3_IsZero(gravNorm) ? Vec3_New(0.0f, 1.0f, 0.0f) : gravNorm);

		Vector3 rawGyro = Vec3_New(
				pitch_input * pitchSensitivity,
				-yaw_input * yawSensitivity,
				roll_input * rollSensitivity
		);

		float gravDotPitch = Vec3_Dot(gravNorm, Vec3_New(1.0f, 0.0f, 0.0f));
		Vector3 pitchAxis = Vec3_Subtract(Vec3_New(1.0f, 0.0f, 0.0f), Vec3_Scale(gravNorm, gravDotPitch));
		pitchAxis = Vec3_Normalize(Vec3_IsZero(pitchAxis) ? Vec3_New(1.0f, 0.0f, 0.0f) : pitchAxis);

		return Vec3_New(
				-Vec3_Dot(rawGyro, gravNorm),  // Yaw Alignment
				Vec3_Dot(rawGyro, pitchAxis), // Pitch Alignment
				rawGyro.z                     // Roll Mapping
		);
#endif
}

#ifdef __cplusplus
}
#endif


#endif // GYROSPACE_H
