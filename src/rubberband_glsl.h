#pragma once
#include <string>
#include <glm/glm.hpp>
#include <vector>

class RubberbandSelection {
private:
	// Shader sources
	const char* vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec2 aPos;
        
        uniform mat4 projection;
        
        void main() {
            gl_Position = projection * vec4(aPos, 0.0, 1.0);
        }
    )";

	const char* fragmentShaderSource = R"(
        #version 330 core
        out vec4 FragColor;
        
        uniform vec3 color;
        uniform float alpha;
        
        void main() {
            FragColor = vec4(color, alpha);
        }
    )";

	// OpenGL objects
	unsigned int shaderProgram;
	unsigned int VAO, VBO;

	// Selection state
	bool isSelecting;
	glm::vec2 startPos;
	glm::vec2 currentPos;

	// Screen dimensions
	float screenWidth, screenHeight;

	// Colors and alpha values
	glm::vec3 fillColor;
	glm::vec3 borderColor;
	float fillAlpha;
	float borderAlpha;

public:
	RubberbandSelection(float width, float height);
	~RubberbandSelection();

private:
	void setupShaders();

	void setupBuffers();
	void checkShaderCompilation(unsigned int shader, const std::string& type);

	void checkProgramLinking(unsigned int program);

	std::vector<float> generateRectangleVertices(glm::vec2 start, glm::vec2 end);

	std::vector<float> generateBorderVertices(glm::vec2 start, glm::vec2 end);

public:
	void startSelection(double mouseX, double mouseY);

	void updateSelection(double mouseX, double mouseY);

	void endSelection();

	void cancelSelection();

	void render();

private:
	void renderFill();

	void renderBorder();

	glm::vec2 screenToNDC(double screenX, double screenY);

	glm::vec2 ndcToScreen(const glm::vec2& ndc);

public:
	// Callback function - override this or set a callback
	virtual void onSelectionComplete(float startX, float startY, float endX, float endY);
	// Getters and setters for customization
	bool isCurrentlySelecting() const { return isSelecting; }

	void setFillColor(float r, float g, float b, float a = 0.3f);

	void setBorderColor(float r, float g, float b, float a = 0.8f);

	void updateScreenSize(float width, float height);

	// Get current selection bounds in screen coordinates
	void getCurrentSelection(float& startX, float& startY, float& endX, float& endY);
};

