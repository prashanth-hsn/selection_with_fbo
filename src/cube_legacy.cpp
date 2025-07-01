#include "cube_legacy.h"
#include "glad/glad.h"
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.inl>

// Populate the color palette with 10 basic colors.
    // Each color is defined with its name and normalized RGB values (0.0 to 1.0).
   

namespace {
// Draw a simple cube
void drawCube(float size) {
    glBegin(GL_QUADS);

    // Front face
    glVertex3f(-size, -size, size);
    glVertex3f(size, -size, size);
    glVertex3f(size, size, size);
    glVertex3f(-size, size, size);

    // Back face
    glVertex3f(-size, -size, -size);
    glVertex3f(-size, size, -size);
    glVertex3f(size, size, -size);
    glVertex3f(size, -size, -size);

    // Top face
    glVertex3f(-size, size, -size);
    glVertex3f(-size, size, size);
    glVertex3f(size, size, size);
    glVertex3f(size, size, -size);

    // Bottom face
    glVertex3f(-size, -size, -size);
    glVertex3f(size, -size, -size);
    glVertex3f(size, -size, size);
    glVertex3f(-size, -size, size);

    // Right face
    glVertex3f(size, -size, -size);
    glVertex3f(size, size, -size);
    glVertex3f(size, size, size);
    glVertex3f(size, -size, size);

    // Left face
    glVertex3f(-size, -size, -size);
    glVertex3f(-size, -size, size);
    glVertex3f(-size, size, size);
    glVertex3f(-size, size, -size);

    glEnd();
}
}

CubeLegacy::CubeLegacy()
{
    colorPalette.emplace_back("Red", 1.0, 0.0, 0.0);       // Pure Red
    colorPalette.emplace_back("Green", 0.0, 1.0, 0.0);     // Pure Green
    colorPalette.emplace_back("Blue", 0.0, 0.0, 1.0);      // Pure Blue
    colorPalette.emplace_back("Yellow", 1.0, 1.0, 0.0);    // Red + Green
    colorPalette.emplace_back("Cyan", 0.0, 1.0, 1.0);      // Green + Blue
    colorPalette.emplace_back("Magenta", 1.0, 0.0, 1.0);   // Red + Blue
    colorPalette.emplace_back("White", 1.0, 1.0, 1.0);     // All components max
    colorPalette.emplace_back("Black", 0.0, 0.0, 0.0);     // All components min
    colorPalette.emplace_back("Gray", 0.5, 0.5, 0.5);      // Mid-range for all components
    colorPalette.emplace_back("Orange", 1.0, 0.65, 0.0);   // Common orange mix
}

void CubeLegacy::render(const std::vector<glm::vec3>& positions, const std::set<int>& selected, bool selection_mode)
{
    // Switch back to fixed-function pipeline.
    glUseProgram(0);
    // Draw all objects
    for (int i = 0; i < positions.size(); i++) 
    {
        if (selection_mode) 
        {
            glLoadName(i + 1); // Object IDs start from 1
        }
        else 
        {
            // Set color based on selection state
            if (selected.count(i+1)) 
            {
                // Selected objects are brighter
                //glColor3f(objects[i].r * 0.7f + 0.3f,
                //    objects[i].g * 0.7f + 0.3f,
                //    objects[i].b * 0.7f + 0.3f);
                glColor3f(0.8, 0.8, 0.8);
            }
            else {
                int icol = (i % 10);
                glColor3f(colorPalette[icol].r, colorPalette[icol].g, colorPalette[icol].b);
            }
        }

        glPushMatrix();
        glm::mat4 mat_i = glm::mat4(1.0);
        mat_i = glm::translate(mat_i, glm::vec3(positions[i].x, positions[i].y, positions[i].z));
        mat_i = glm::rotate(mat_i, glm::radians(30.f), glm::vec3(1.0f, 0.0f, 0.0f));
        mat_i = glm::rotate(mat_i, glm::radians(15.f), glm::vec3(0.0f, 1.0f, 0.0f));
        glMultMatrixf(glm::value_ptr(mat_i));
        drawCube(0.5f);
        glPopMatrix();
    }

}
