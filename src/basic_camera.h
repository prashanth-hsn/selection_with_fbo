#pragma once

#include <vector>
#include <cmath>

// Include GLM headers
#define GLM_ENABLE_EXPERIMENTAL // Required for some experimental features like glm::to_string
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> // For glm::lookAt, glm::perspective, glm::rotate, glm::translate
#include <glm/gtc/type_ptr.hpp>         // For glm::value_ptr (if needed, not directly used for basic ops)
#include <glm/gtx/string_cast.hpp>      // For glm::to_string (useful for debugging)


// --- Camera Class: Mimics 3D Modeling Software Camera ---
class Camera {
public:
    glm::vec3 position;      // Camera's world position
    glm::vec3 target;        // Point the camera is looking at
    glm::vec3 worldUp;       // World's up direction (e.g., (0,1,0))

    float fovY;         // Vertical field of view in degrees
    float aspectRatio;  // Viewport width / height
    float nearClip;     // Near clipping plane distance
    float farClip;      // Far clipping plane distance

    float panSpeed;     // Multiplier for panning
    float zoomSpeed;    // Multiplier for zooming
    float orbitSpeed;   // Multiplier for orbiting

    float rotation_sensitivity;
    float pan_sensitivity;
private:
    float distanceToTarget; // Current distance from position to target

public:
    // Constructor
    Camera(glm::vec3 initialPosition = glm::vec3(0.0f, 0.0f, 10.0f),
        glm::vec3 initialTarget = glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3 upVector = glm::vec3(0.0f, 1.0f, 0.0f),
        float initialFovY = 45.0f,
        float initialAspect = 16.0f / 9.0f,
        float initialNear = 0.1f,
        float initialFar = 1000.0f);

    // Update the aspect ratio if the viewport changes
    void setAspectRatio(float newAspect);

    // Pan the camera
    // dx, dy are mouse movement deltas (e.g., in pixels)
    void pan(float dx, float dy);

    // Zoom the camera (move closer/further from target)
    // delta is scroll wheel delta or mouse movement delta
    void zoom(float delta);

    // Orbit the camera around the target point
    // angleX, angleY are mouse movement deltas (e.g., in pixels)
    void orbit(float angleX_delta, float angleY_delta);


    // Get the current view matrix
    glm::mat4 getViewMatrix() const;

    // Get the current projection matrix
    glm::mat4 getProjectionMatrix() const;

    // Print camera state for debugging
    void printState() const;

    void reset();
private:
    // Helper to update distanceToTarget after position/target changes
    void updateDistanceToTarget();
};


// Input handler for arcball camera
class CameraController {
private:
    Camera* camera;
    float screenWidth, screenHeight;
    bool leftMouseDown;
    bool middleMouseDown;
    bool shiftPressed;
    glm::vec2 last_mouse_pos;
public:
    CameraController(Camera* cam, float width, float height);
    ~CameraController() = default;

    // Disable copy constructor and assignment operator
    CameraController(const CameraController&) = delete;
    CameraController& operator=(const CameraController&) = delete;

    // Move constructor and assignment operator
    CameraController(CameraController&& other) = default;
    CameraController& operator=(CameraController&& other) = default;

    void setScreenSize(float width, float height);
    void mouseButtonCallback(int button, int action, float mouseX, float mouseY);
    void mouseMoveCallback(float mouseX, float mouseY);
    void scrollCallback(float yOffset);
    void keyCallback(int key, int action);
};

