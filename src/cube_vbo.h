#pragma once

#include <set>
#include <glm/glm.hpp>

class CubeRenderer {
private:
    GLuint VAO, VBO, EBO;
    GLuint shaderProgram;
    GLuint pickShaderPrg;
    GLint modelLoc, viewLoc, projectionLoc, selectedLoc;
    GLint p_modelLoc, p_viewLoc, p_projectionLoc, p_picking_color;
    bool selection_mode;
    float sel_x, sel_y, sel_w, sel_h;
public:
    CubeRenderer();

    ~CubeRenderer();
    bool get_section_mode();

    static GLuint setupShaders(const char* vertex, const char* frag);
    void getUniformLocations();
    void updatePickingUniformLocs();

    void setupBuffers();

	void set_section_mode(bool flag) { selection_mode = flag; }

    void set_selection_rectangle(float x, float y, float w, float h);

    void render(const glm::mat4& view, const glm::mat4& projection, const std::vector<glm::mat4>& models, const std::set<int>& selected);

    void pick_render(const glm::mat4& view, const glm::mat4& projection, const std::vector<glm::mat4>& models, const std::set<int>& selected);

    std::vector<unsigned char> readFrameBufferPixels(int x, int y, int width, int height);

};
