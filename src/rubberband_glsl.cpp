
#include "rubberband_glsl.h"
#include "glad/glad.h"
#include <string>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>

#ifdef WIN32
#undef min
#undef max
#endif

RubberbandSelection::RubberbandSelection(float width, float height)
	: isSelecting(false), screenWidth(width), screenHeight(height),
	fillColor(0.2f, 0.6f, 1.0f), borderColor(0.0f, 0.4f, 0.8f),
	fillAlpha(0.3f), borderAlpha(0.8f) {

	setupShaders();
	setupBuffers();
}

RubberbandSelection::~RubberbandSelection() {
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteProgram(shaderProgram);
}

void RubberbandSelection::setupShaders() {
	// Compile vertex shader
	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	checkShaderCompilation(vertexShader, "VERTEX");

	// Compile fragment shader
	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	checkShaderCompilation(fragmentShader, "FRAGMENT");

	// Link shader program
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	checkProgramLinking(shaderProgram);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}

void RubberbandSelection::setupBuffers() {
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	// Configure vertex attributes (position only)
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);
}

void RubberbandSelection::checkShaderCompilation(unsigned int shader, const std::string& type) {
	int success;
	char infoLog[512];
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(shader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::" << type << "::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
}

void RubberbandSelection::checkProgramLinking(unsigned int program) {
	int success;
	char infoLog[512];
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(program, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}
}

std::vector<float> RubberbandSelection::generateRectangleVertices(glm::vec2 start, glm::vec2 end) {
	return {
		start.x, start.y,  // Bottom left
		end.x,   start.y,  // Bottom right
		end.x,   end.y,    // Top right
		start.x, end.y     // Top left
	};
}

std::vector<float> RubberbandSelection::generateBorderVertices(glm::vec2 start, glm::vec2 end) {
	return {
		start.x, start.y,  // Bottom left
		end.x,   start.y,  // Bottom right
		end.x,   end.y,    // Top right
		start.x, end.y,    // Top left
		start.x, start.y   // Close the loop
	};
}

void RubberbandSelection::startSelection(double mouseX, double mouseY) {
	isSelecting = true;
	startPos = screenToNDC(mouseX, mouseY);
	currentPos = startPos;
}

void RubberbandSelection::updateSelection(double mouseX, double mouseY) {
	if (isSelecting) {
		currentPos = screenToNDC(mouseX, mouseY);
	}
}

void RubberbandSelection::endSelection(glm::vec2& start, glm::vec2& end) {
	if (isSelecting) {
		isSelecting = false;

		// Convert back to screen coordinates for application use
		glm::vec2 startScreen = ndcToScreen(startPos);
		glm::vec2 endScreen = ndcToScreen(currentPos);

		// Call selection callback with screen coordinates
		onSelectionComplete(startScreen.x, startScreen.y, endScreen.x, endScreen.y);
		start = startScreen;
		end = endScreen;
	}
}

void RubberbandSelection::cancelSelection() {
	isSelecting = false;
}

void RubberbandSelection::render() {
	if (!isSelecting) return;

	glUseProgram(shaderProgram);

	// Set up orthographic projection for NDC coordinates
	glm::mat4 projection = glm::mat4(1.0f);
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	// Enable blending for transparency
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Render filled rectangle
	renderFill();

	// Render border
	renderBorder();

	glDisable(GL_BLEND);
}

void RubberbandSelection::renderFill() {
	// Generate rectangle vertices for filled area
	auto vertices = generateRectangleVertices(startPos, currentPos);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);

	// Set fill color and alpha
	glUniform3fv(glGetUniformLocation(shaderProgram, "color"), 1, glm::value_ptr(fillColor));
	glUniform1f(glGetUniformLocation(shaderProgram, "alpha"), fillAlpha);

	// Draw as triangle fan (4 vertices forming a rectangle)
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void RubberbandSelection::renderBorder() {
	// Generate border vertices (line loop)
	auto vertices = generateBorderVertices(startPos, currentPos);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);

	// Set border color and alpha
	glUniform3fv(glGetUniformLocation(shaderProgram, "color"), 1, glm::value_ptr(borderColor));
	glUniform1f(glGetUniformLocation(shaderProgram, "alpha"), borderAlpha);

	// Set line width
	glLineWidth(2.0f);

	// Draw border as line loop
	glDrawArrays(GL_LINE_LOOP, 0, 4);

	// Reset line width
	glLineWidth(1.0f);
}

glm::vec2 RubberbandSelection::screenToNDC(double screenX, double screenY) {
	// Convert screen coordinates to normalized device coordinates (-1 to 1)
	float x = (2.0f * screenX) / screenWidth - 1.0f;
	float y = 1.0f - (2.0f * screenY) / screenHeight;  // Flip Y axis
	return glm::vec2(x, y);
}

glm::vec2 RubberbandSelection::ndcToScreen(const glm::vec2& ndc) {
	// Convert normalized device coordinates back to screen coordinates
	float x = (ndc.x + 1.0f) * screenWidth / 2.0f;
	float y = (1.0f - ndc.y) * screenHeight / 2.0f;  // Flip Y axis
	return glm::vec2(x, y);
}

// Callback function - override this or set a callback
void RubberbandSelection::onSelectionComplete(float startX, float startY, float endX, float endY) {
	std::cout << "Selection completed: (" << startX << ", " << startY << ") to ("
		<< endX << ", " << endY << ")" << std::endl;

	// Calculate selection rectangle
	float left = std::min(startX, endX);
	float right = std::max(startX, endX);
	float top = std::min(startY, endY);
	float bottom = std::max(startY, endY);

	std::cout << "Selection bounds: left=" << left << ", right=" << right
		<< ", top=" << top << ", bottom=" << bottom << std::endl;
}


void RubberbandSelection::setFillColor(float r, float g, float b, float a) {
	fillColor = glm::vec3(r, g, b);
	fillAlpha = a;
}

void RubberbandSelection::setBorderColor(float r, float g, float b, float a) {
	borderColor = glm::vec3(r, g, b);
	borderAlpha = a;
}

void RubberbandSelection::updateScreenSize(float width, float height) {
	screenWidth = width;
	screenHeight = height;
}

// Get current selection bounds in screen coordinates
void RubberbandSelection::getCurrentSelection(float& startX, float& startY, float& endX, float& endY) {
	glm::vec2 startScreen = ndcToScreen(startPos);
	glm::vec2 endScreen = ndcToScreen(currentPos);
	startX = startScreen.x;
	startY = startScreen.y;
	endX = endScreen.x;
	endY = endScreen.y;
}
