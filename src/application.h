#pragma once

#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include "basic_camera.h"
#include "rubberband_glsl.h"
//#include "instanced_renderer.h"
#include "cube_vbo.h"

// Example usage with GLFW
class Application {
private:
    GLFWwindow* window;
    RubberbandSelection* rubberband;
    //InstancedRenderer* instanced_renderer_;
    CubeRenderer* cube_renderer_;
    int windowWidth = 800;
    int windowHeight = 600;
    Camera* camera;
    CameraController* cam_ctrl;
    bool rubberband_active = false;
    std::vector<glm::mat4> m_models;
    GLuint FBO;
public:
    Application();

    ~Application();

private:
    void initOpenGL();

    void setupCallbacks();

    void mouseButtonCallback(int button, int action, int mods);
    void mouseMoveCallback(double xpos, double ypos);

    void framebufferSizeCallback(int width, int height);
    void select_in_rectangle(float st_x, float st_y, float end_x, float end_y);
    void update_models();
    void draw_scene();
    void init_fbo();
public:
    void run();
}; 
