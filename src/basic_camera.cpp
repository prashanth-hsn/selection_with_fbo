#pragma once

#include "basic_camera.h"

#include <iostream>
#include <iomanip> // For std::fixed and std::setprecision

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

namespace {
	// --- Helper for printing GLM vectors and matrices ---
	// Overload for glm::vec3
	std::ostream& operator<<(std::ostream& os, const glm::vec3& v) {
		os << std::fixed << std::setprecision(2) << "(" << v.x << ", " << v.y << ", " << v.z << ")";
		return os;
	}

	// Overload for glm::mat4 (to match previous custom Mat4 output format)
	std::ostream& operator<<(std::ostream& os, const glm::mat4& m) {
		os << std::fixed << std::setprecision(2);
		for (int i = 0; i < 4; ++i) { // Iterate rows
			for (int j = 0; j < 4; ++j) { // Iterate columns
				os << std::setw(8) << m[j][i] << (j == 3 ? "" : " "); // m[column][row] for GLM
			}
			os << "\n";
		}
		return os;
	}

	bool isZero(float value) {
    	return std::fabs(value) < std::numeric_limits<float>::epsilon(); 
	}
}

// Constructor
Camera::Camera(glm::vec3 initialPosition, glm::vec3 initialTarget, glm::vec3 upVector,
	float initialFovY, float initialAspect, float initialNear, float initialFar) :
	position(initialPosition),
	target(initialTarget),
	worldUp(glm::normalize(upVector)), // Ensure worldUp is normalized
	fovY(initialFovY),
	aspectRatio(initialAspect),
	nearClip(initialNear),
	farClip(initialFar),
	panSpeed(0.001f),  // Default pan speed
	zoomSpeed(0.1f),  // Default zoom speed
	orbitSpeed(0.1f), // Default orbit speed (radians per pixel),
	rotation_sensitivity(0.1),
	pan_sensitivity(0.1)
{
	updateDistanceToTarget();
}

// Update the aspect ratio if the viewport changes
void Camera::setAspectRatio(float newAspect) {
	aspectRatio = newAspect;
}

// Pan the camera
// dx, dy are mouse movement deltas (e.g., in pixels)
void Camera::pan(float dx, float dy) {
	glm::vec3 forward = glm::normalize(target - position);
	glm::vec3 right = glm::normalize(glm::cross(forward, worldUp)); // Camera's right vector
	glm::vec3 up = glm::cross(right, forward);                  // Camera's up vector (re-orthogonalized)

	// Scale pan movement by distance to target for natural feel
	// and by a sensitivity factor
	float moveAmount = distanceToTarget * panSpeed * pan_sensitivity;

	position = position - right * dx * moveAmount;
	target = target - right * dx * moveAmount;

	position = position + up * dy * moveAmount;
	target = target + up * dy * moveAmount;
}

// Zoom the camera (move closer/further from target)
// delta is scroll wheel delta or mouse movement delta
void Camera::zoom(float delta) {
	glm::vec3 direction = glm::normalize(target - position);
	float zoomAmount = delta * zoomSpeed;

	// Prevent zooming too close or past the target
	if (distanceToTarget - zoomAmount < nearClip * 2.0f) { // Prevent going too close
		zoomAmount = distanceToTarget - nearClip * 2.0f;
	}
	if (distanceToTarget - zoomAmount > farClip * 0.9f) { // Prevent going too far
		zoomAmount = distanceToTarget - farClip * 0.9f;
	}

	position = position + direction * zoomAmount;
	updateDistanceToTarget(); // Recalculate distance after zoom
}


// Orbit the camera around the target point
// angleX, angleY are mouse movement deltas (e.g., in pixels)
void Camera::orbit(float angleX_delta, float angleY_delta) {
	// Convert pixel deltas to radians using orbitSpeed
	float yawAngle = angleX_delta * orbitSpeed * rotation_sensitivity;   // Rotation around worldUp (Y-axis)
	float pitchAngle = angleY_delta * orbitSpeed * rotation_sensitivity; // Rotation around camera's right vector

	// Vector from target to current camera position
	glm::vec3 relativePosition = position - target;

	// Get current camera's right vector for pitch rotation
	glm::vec3 currentForward = glm::normalize(target - position);
	glm::vec3 currentRight = glm::normalize(glm::cross(currentForward, worldUp));

    // Create quaternions for yaw and pitch rotations
	
    glm::quat yawQuat = glm::angleAxis(yawAngle, worldUp);
    glm::quat pitchQuat = glm::angleAxis(pitchAngle, currentRight);

    // Apply yaw rotation
    glm::vec3 tempRelativePosition = yawQuat * relativePosition;
	// Recalculate the right vector based on the yawed relative position
	glm::vec3 newForwardAfterYaw = glm::normalize(tempRelativePosition) * -1.0f;
	glm::vec3 newRightAfterYaw = glm::normalize(glm::cross(newForwardAfterYaw, worldUp));

    // Apply pitch rotation around the *new* right vector (after yaw)
    glm::vec3 finalRelativePosition = pitchQuat * tempRelativePosition;

	// Update camera position
	position = target + finalRelativePosition;

	// Re-orthogonalize worldUp if needed, though for fixed worldUp, this might be less critical
	// For CATIA-like camera, worldUp usually stays fixed.
	// However, if we orbit too far, the camera's internal 'up' can flip.
	// A more robust solution might clamp pitch angles or use quaternions.
	// For now, we'll rely on the fixed worldUp and GLM's lookAt to handle the 'up' direction.
	// If the camera's up vector needs to be dynamic, it would be calculated here.

	updateDistanceToTarget(); // Recalculate distance after orbit
}


// Get the current view matrix
glm::mat4 Camera::getViewMatrix() const {
	return glm::lookAt(position, target, worldUp);
}

// Get the current projection matrix
glm::mat4 Camera::getProjectionMatrix() const {
	return glm::perspective(glm::radians(fovY), aspectRatio, nearClip, farClip);
}

// Print camera state for debugging
void Camera::printState() const {
	std::cout << "--- Camera State ---\n";
	std::cout << "Position: " << position << "\n";
	std::cout << "Target:   " << target << "\n";
	std::cout << "World Up: " << worldUp << "\n";
	std::cout << "Distance to Target: " << std::fixed << std::setprecision(2) << distanceToTarget << "\n";
	std::cout << "FOV Y: " << fovY << ", Aspect: " << aspectRatio << ", Near: " << nearClip << ", Far: " << farClip << "\n";
	std::cout << "--------------------\n";
}

void Camera::reset()
{
	position = glm::vec3(0.f, 0.f, 8.f);
	target = glm::vec3(0.0f, 0.0f, 0.0f);
	worldUp = (glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f)));
	nearClip = 0.1f;
	farClip = 100.f;
	updateDistanceToTarget();
}

// Helper to update distanceToTarget after position/target changes
void Camera::updateDistanceToTarget() {
	distanceToTarget = glm::length(position - target);
	if (distanceToTarget < 0.001f) { // Prevent division by zero or very small numbers
		distanceToTarget = 0.001f;
	}
}


// ArcballController Implementation

CameraController::CameraController(Camera* cam, float width, float height)
	: camera(cam), screenWidth(width), screenHeight(height),
	leftMouseDown(false), middleMouseDown(false), shiftPressed(false),
	last_mouse_pos(0.f, 0.f)
{

}

void CameraController::setScreenSize(float width, float height) {
	screenWidth = width;
	screenHeight = height;
	camera->setAspectRatio(width / height);
}

void CameraController::mouseButtonCallback(int button, int action, float mouseX, float mouseY) {

	if (button == 0) { // Left mouse button
		if (action == 1) { // Press
			leftMouseDown = true;
		}
		else if (action == 0) { // Release
			leftMouseDown = false;
		}
	}
	else if (button == 2) { // Middle mouse button
		if (action == 1) { // Press
			middleMouseDown = true;			
		}
		else if (action == 0) { // Release
			middleMouseDown = false;
		}
	}

	last_mouse_pos = glm::vec2(mouseX, mouseY);
}

void CameraController::mouseMoveCallback(float mouseX, float mouseY) {

	glm::vec2 delta = glm::vec2(mouseX, mouseY) - last_mouse_pos;
	if (leftMouseDown) {
		if (shiftPressed) {
			camera->pan(delta.x, delta.y);
		}
		else {
			camera->orbit(delta.x, delta.y);
		}
	}
	else if (middleMouseDown) {
		camera->pan(delta.x, delta.y);
	}

	last_mouse_pos = glm::vec2(mouseX, mouseY);

}

void CameraController::scrollCallback(float yOffset) {
	camera->zoom(yOffset);
}

void CameraController::keyCallback(int key, int action) {
	if (key == 340) { // Left Shift
		shiftPressed = (action != 0);
	}
	else if (key == 82 && action == 1) { // R key - reset
		camera->reset();
	}
}

