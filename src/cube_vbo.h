#pragma once

#include <set>
#include <glm/glm.hpp>

class CubeRenderer {
private:
    GLuint VAO, VBO, EBO;
    GLuint shaderProgram;
    GLuint modelLoc, viewLoc, projectionLoc, selectedLoc;
    bool selection_mode;
public:
    CubeRenderer();

    ~CubeRenderer();

    void setupShaders();

    void setupBuffers();

	void set_section_mode(bool flag) { selection_mode = flag; }

    void render(const glm::mat4& view, const glm::mat4& projection, const std::vector<glm::mat4>& models, const std::set<int>& selected);
};
