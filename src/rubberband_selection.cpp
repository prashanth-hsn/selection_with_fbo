#include <functional>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "gl/GLU.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include "cube_vbo.h"
#include "cube_legacy.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <set>

#define BUFSIZE 512
#define MAX_OBJECTS 3

// Window dimensions
int windowWidth = 800;
int windowHeight = 600;

// Selection buffer
GLuint selectBuffer[BUFSIZE];

// GLFW window pointer
GLFWwindow* window;

typedef struct
{
    glm::mat4 view, projection;
    CubeRenderer* cube_renderer;
    CubeLegacy* cube_legacy;
} MyWindowData;

// Rubberband selection state
typedef struct {
    int active;           // Is rubberband selection active?
    double startX, startY; // Start position
    double currentX, currentY; // Current mouse position
} RubberbandState;

RubberbandState rubberband = { 0, 0, 0, 0, 0 };

std::set<int> selected;

// Draw the 3D scene
void drawScene(int selectionMode, glm::mat4 view, glm::mat4 projection, CubeRenderer& cube_renderer, CubeLegacy& cube_legacy) {
    if (!selectionMode) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    // Set up camera
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0.0, 0.0, 8.0,   // eye position
        0.0, 0.0, 0.0,   // look at point
        0.0, 1.0, 0.0);  // up vector

    // Initialize name stack for selection
    if (selectionMode) {
        glInitNames();
        glPushName(0);
    }

    glm::mat4 v = glm::mat4(1.0);
    glGetFloatv(GL_MODELVIEW_MATRIX, glm::value_ptr(v));

    glm::mat4 p = glm::mat4(1.0);;
    glGetFloatv(GL_PROJECTION_MATRIX, glm::value_ptr(p));

    //glm::mat4 model = glm::mat4(1.0);

    std::vector<glm::vec3> positions;
    positions.push_back({ -3.0f, -2.0f, 0.0f });
    //positions.push_back({ 0.0f, -2.0f, 0.0f });
    //positions.push_back({ 3.0f, -2.0f, 0.0f });
    //positions.push_back({ -3.0f,  2.0f, 0.0f });
    //positions.push_back({ 0.0f,  2.0f, 0.0f });
    //positions.push_back({ 3.0f,  2.0f, 0.0f });

    cube_legacy.render(positions, selected, selectionMode);



    std::vector<glm::mat4> models;
    auto getModelMat = [](glm::vec3 pos) -> glm::mat4
    {
        glm::mat4 model3 = glm::mat4(1.0f);
        model3 = glm::translate(model3, pos);
        model3 = glm::rotate(model3, glm::radians(30.f), glm::vec3(1.0f, 0.0f, 0.0f));
        model3 = glm::rotate(model3, glm::radians(15.f), glm::vec3(0.0f, 1.0f, 0.0f));
        return model3;
    };

    models.push_back(getModelMat(glm::vec3(3.0f, 2.0f, 0.0f)));
    //models.push_back(getModelMat(glm::vec3( 0.0f, 0.0f, 0.0f)));
    //models.push_back(getModelMat(glm::vec3( 3.0f, 0.0f, 0.0f)));



    cube_renderer.set_section_mode(selectionMode);
    cube_renderer.render(v, p, models, selected);

}

// Draw the rubberband rectangle in 2D overlay
void drawRubberband() {
    if (!rubberband.active) return;

    // Switch to 2D overlay mode
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, windowWidth, windowHeight, 0, -1, 1); // Screen coordinates

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Disable depth testing and lighting for overlay
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);

    // Draw rubberband rectangle
    glColor3f(1.0f, 1.0f, 1.0f); // White color
    glLineWidth(1.0f);

    // Draw rectangle outline
    glBegin(GL_LINE_LOOP);
    glVertex2f(rubberband.startX, rubberband.startY);
    glVertex2f(rubberband.currentX, rubberband.startY);
    glVertex2f(rubberband.currentX, rubberband.currentY);
    glVertex2f(rubberband.startX, rubberband.currentY);
    glEnd();

    // Optional: Draw semi-transparent fill
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.5f, 0.5f, 1.0f, 0.2f); // Semi-transparent blue

    glBegin(GL_QUADS);
    glVertex2f(rubberband.startX, rubberband.startY);
    glVertex2f(rubberband.currentX, rubberband.startY);
    glVertex2f(rubberband.currentX, rubberband.currentY);
    glVertex2f(rubberband.startX, rubberband.currentY);
    glEnd();

    glDisable(GL_BLEND);

    // Restore states
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);

    // Restore matrices
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

// Process hit records from selection
void processHits(GLint hits, GLuint buffer[]) {
    GLuint* ptr = buffer;

    printf("Rubberband selection hits: %d\n", hits);

    // Clear previous selection
    selected.clear();

    // Process all hits
    for (int i = 0; i < hits; i++) {
        GLuint names = *ptr++;     // Number of names in this hit record
        GLuint z1 = *ptr++;        // Minimum Z value
        GLuint z2 = *ptr++;        // Maximum Z value

        // Get object names
        for (GLuint j = 0; j < names; j++) {
            GLuint objectId = *ptr++;
            if (objectId >= 1 ) {
                selected.insert(objectId);
                printf("Selected object: %d\n", objectId);
            }

            if(objectId >= 100)
            {
                printf("VBO object selected: %d\n", objectId);
            }
        }
    }
}

// Perform rubberband selection
void performRubberbandSelection(glm::mat4 view, glm::mat4 projection, CubeRenderer& cube_renderer, CubeLegacy& cube_legacy) {
    GLint viewport[4];

    // Get current viewport
    glGetIntegerv(GL_VIEWPORT, viewport);

    // Calculate selection rectangle bounds
    double minX = fmin(rubberband.startX, rubberband.currentX);
    double maxX = fmax(rubberband.startX, rubberband.currentX);
    double minY = fmin(rubberband.startY, rubberband.currentY);
    double maxY = fmax(rubberband.startY, rubberband.currentY);

    // Convert to OpenGL coordinates (flip Y)
    minY = viewport[3] - maxY;
    maxY = viewport[3] - fmin(rubberband.startY, rubberband.currentY);

    double width = maxX - minX;
    double height = maxY - minY;
    double centerX = minX + width / 2.0;
    double centerY = minY + height / 2.0;

    // Skip if selection area is too small
    if (width < 5 || height < 5) return;

    // Save current OpenGL state
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    // Disable lighting and other effects during selection
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);

    // Set up selection buffer
    glSelectBuffer(BUFSIZE, selectBuffer);
    glRenderMode(GL_SELECT);

    // Save current projection matrix
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    // Create pick region covering the rubberband area
    gluPickMatrix(centerX, centerY, width, height, viewport);

    // Apply the same projection as in display
    gluPerspective(45.0, (GLdouble)windowWidth / windowHeight, 1.0, 100.0);
    //glMatrixMode(GL_PROJECTION);
    //glMultMatrixf(glm::value_ptr(projection));
    // Draw the scene in selection mode
    drawScene(1, view, projection, cube_renderer, cube_legacy);

    // Restore projection matrix
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    // Return to render mode and get hit count
    GLint hits = glRenderMode(GL_RENDER);

    // Restore all OpenGL state
    glPopAttrib();

    // Process the hits
    processHits(hits, selectBuffer);
}

// GLFW mouse button callback
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        if (action == GLFW_PRESS) {
            // Start rubberband selection
            rubberband.active = 1;
            rubberband.startX = xpos;
            rubberband.startY = ypos;
            rubberband.currentX = xpos;
            rubberband.currentY = ypos;

            // Clear selection if not holding Ctrl
            if (!(mods & GLFW_MOD_CONTROL)) {
                selected.clear();
            }
        }
        else if (action == GLFW_RELEASE) {
            // End rubberband selection
            if (rubberband.active) {

                MyWindowData* data = static_cast<MyWindowData*>(glfwGetWindowUserPointer(window));
                performRubberbandSelection(data->view, data->projection, *data->cube_renderer, *data->cube_legacy);
                rubberband.active = 0;
            }
        }
    }
}

// GLFW cursor position callback
void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
    if (rubberband.active) {
        rubberband.currentX = xpos;
        rubberband.currentY = ypos;
    }
}

// GLFW framebuffer size callback
void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    windowWidth = width;
    windowHeight = height;

    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (GLdouble)width / height, 1.0, 100.0);
}

// GLFW error callback
void errorCallback(int error, const char* description) {
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

// Initialize OpenGL
void initGL() {


    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return;
    }

    std::cout << "OpenGL " << GLVersion.major << ", " << GLVersion.minor << std::endl;

    const GLubyte* vendor = glGetString(GL_VENDOR);
    std::cout << "OpenGL Vendor: " << vendor << "\n";


    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glEnable(GL_DEPTH_TEST);

    // Enable lighting for better cube visibility
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    GLfloat lightPos[] = { 5.0f, 5.0f, 5.0f, 1.0f };
    GLfloat lightAmbient[] = { 0.3f, 0.3f, 0.3f, 1.0f };
    GLfloat lightDiffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };

    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    // Set up initial projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (GLdouble)windowWidth / windowHeight, 1.0, 100.0);
}

int main() {
    // Initialize GLFW
    glfwSetErrorCallback(errorCallback);

    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }


    // --- Request OpenGL 3.3 Compatibility Profile ---
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE); // The key part!

    // Create window
    window = glfwCreateWindow(windowWidth, windowHeight,
        "OpenGL Rubberband Selection Demo", NULL, NULL);
    if (!window) {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }

    // Make the OpenGL context current
    glfwMakeContextCurrent(window);

    // Enable v-sync
    glfwSwapInterval(1);

    // Initialize OpenGL
    initGL();

    printf("Mixed Pipeline Drawing Selection Demo\n");
    printf("Instructions:\n");
    printf("- Click and drag to create selection rectangle\n");
    printf("- Hold Ctrl while selecting to add to selection\n");
    printf("- Selected objects will appear gray\n");
    printf("- Press ESC to exit\n\n");


    // Create cube renderer
    CubeRenderer cubeRenderer;

    // Set up camera matrices
    glm::mat4 view = glm::lookAt(
        glm::vec3(.0f, .0f, 8.0f),  // Camera position
        glm::vec3(0.0f, 0.0f, 0.0f),  // Look at origin
        glm::vec3(0.0f, 1.0f, 0.0f)   // Up vector
    );


    glm::mat4 projection = glm::perspective(
        glm::radians(45.0f),           // FOV
        (float)windowWidth / windowHeight,               // Aspect ratio
        1.f,                          // Near plane
        100.0f                         // Far plane
    );

    CubeLegacy cube_legacy;

    MyWindowData my_matrices{ view, projection, &cubeRenderer, &cube_legacy };
    glfwSetWindowUserPointer(window, &my_matrices);

    // Set callbacks
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPositionCallback);

    // Main render loop
    while (!glfwWindowShouldClose(window)) {
        // Poll for and process events
        glfwPollEvents();

        // Check for ESC key
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        // Render 3D scene
        drawScene(0, view, projection, cubeRenderer, cube_legacy);

        // Draw rubberband overlay
        drawRubberband();

        // Swap front and back buffers
        glfwSwapBuffers(window);
    }

    // Clean up
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
