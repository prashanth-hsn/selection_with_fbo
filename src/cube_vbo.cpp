#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include "cube_vbo.h"

#include <set>

// Vertex shader source
const char* vertexShaderSource = R"(
#version 330 compatibility
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec3 vertexColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    vertexColor = aColor;
}
)";

// Fragment shader source
const char* fragmentShaderSource = R"(
#version 330 compatibility
in vec3 vertexColor;
out vec4 FragColor;


uniform int selected;

void main()
{
	if(selected == 1)
	{
		FragColor = vec4(0.8, 0.8, 0.8, 1.0);
	}
	else
	{
		FragColor = vec4(vertexColor, 1.0);
	}
}
)";


CubeRenderer::CubeRenderer() {
	setupShaders();
	setupBuffers();
}

CubeRenderer::~CubeRenderer() {
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	glDeleteProgram(shaderProgram);
}

void CubeRenderer::setupShaders() {
	// Compile vertex shader
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	// Check for vertex shader compile errors
	int success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cerr << "Vertex shader compilation failed: " << infoLog << std::endl;
	}

	// Compile fragment shader
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	// Check for fragment shader compile errors
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cerr << "Fragment shader compilation failed: " << infoLog << std::endl;
	}

	// Link shaders
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	// Check for linking errors
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cerr << "Shader program linking failed: " << infoLog << std::endl;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	// Get uniform locations
	modelLoc = glGetUniformLocation(shaderProgram, "model");
	viewLoc = glGetUniformLocation(shaderProgram, "view");
	projectionLoc = glGetUniformLocation(shaderProgram, "projection");
	selectedLoc = glGetUniformLocation(shaderProgram, "selected");

}

void CubeRenderer::setupBuffers() {


	float size = 0.5f;
	// Cube vertices with positions and colors
	float vertices[] = {
		// Front face (red)
		-size, -size,  size,  1.0f, 0.0f, 0.0f,
		 size, -size,  size,  1.0f, 0.0f, 0.0f,
		 size,  size,  size,  1.0f, 0.0f, 0.0f,
		-size,  size,  size,  1.0f, 0.0f, 0.0f,

		// Back face (green)
		-size, -size, -size,  0.0f, 1.0f, 0.0f,
		 size, -size, -size,  0.0f, 1.0f, 0.0f,
		 size,  size, -size,  0.0f, 1.0f, 0.0f,
		-size,  size, -size,  0.0f, 1.0f, 0.0f,

		// Left face (blue)
		-size, -size, -size,  0.0f, 0.0f, 1.0f,
		-size, -size,  size,  0.0f, 0.0f, 1.0f,
		-size,  size,  size,  0.0f, 0.0f, 1.0f,
		-size,  size, -size,  0.0f, 0.0f, 1.0f,

		// Right face (yellow)
		 size, -size, -size,  1.0f, 1.0f, 0.0f,
		 size, -size,  size,  1.0f, 1.0f, 0.0f,
		 size,  size,  size,  1.0f, 1.0f, 0.0f,
		 size,  size, -size,  1.0f, 1.0f, 0.0f,

		 // Top face (magenta)
		 -size,  size, -size,  1.0f, 0.0f, 1.0f,
		  size,  size, -size,  1.0f, 0.0f, 1.0f,
		  size,  size,  size,  1.0f, 0.0f, 1.0f,
		 -size,  size,  size,  1.0f, 0.0f, 1.0f,

		 // Bottom face (cyan)
		 -size, -size, -size,  0.0f, 1.0f, 1.0f,
		  size, -size, -size,  0.0f, 1.0f, 1.0f,
		  size, -size,  size,  0.0f, 1.0f, 1.0f,
		 -size, -size,  size,  0.0f, 1.0f, 1.0f
	};

	// Indices for cube faces
	unsigned int indices[] = {
		// Front face
		0, 1, 2, 2, 3, 0,
		// Back face
		4, 5, 6, 6, 7, 4,
		// Left face
		8, 9, 10, 10, 11, 8,
		// Right face
		12, 13, 14, 14, 15, 12,
		// Top face
		16, 17, 18, 18, 19, 16,
		// Bottom face
		20, 21, 22, 22, 23, 20
	};

	// Generate and bind VAO
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	// Generate and bind VBO
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Generate and bind EBO
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// Color attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
}

void CubeRenderer::render(const glm::mat4& view, const glm::mat4& projection, const std::vector<glm::mat4>& models, const std::set<int>& selected )
{
	glUseProgram(shaderProgram);

	// Set view and projection matrices
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
	
	
	glBindVertexArray(VAO);

	int model_id = 100;
	// Render each cube with its model matrix
	for (const auto& model : models) {

		if (selection_mode) {
			glLoadName(model_id);
		}

		if(selected.count(model_id))
		{
			glUniform1i(selectedLoc, 1 );
		}
		else
		{
			glUniform1i(selectedLoc, 0);
		}

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		++model_id;
	}

	glBindVertexArray(0);
}


/*
int main() {
	// Initialize GLFW
	if (!glfwInit()) {
		std::cerr << "Failed to initialize GLFW" << std::endl;
		return -1;
	}

	// Configure GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Create window
	GLFWwindow* window = glfwCreateWindow(800, 600, "Three Cubes OpenGL", NULL, NULL);
	if (!window) {
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	// Initialize GLEW
	if (glewInit() != GLEW_OK) {
		std::cerr << "Failed to initialize GLEW" << std::endl;
		return -1;
	}

	// Configure OpenGL
	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, 800, 600);

	// Create cube renderer
	CubeRenderer cubeRenderer;

	// Set up camera matrices
	glm::mat4 view = glm::lookAt(
		glm::vec3(4.0f, 3.0f, 6.0f),  // Camera position
		glm::vec3(0.0f, 0.0f, 0.0f),  // Look at origin
		glm::vec3(0.0f, 1.0f, 0.0f)   // Up vector
	);

	glm::mat4 projection = glm::perspective(
		glm::radians(45.0f),           // FOV
		800.0f / 600.0f,               // Aspect ratio
		0.1f,                          // Near plane
		100.0f                         // Far plane
	);

	// Main render loop
	while (!glfwWindowShouldClose(window)) {
		// Input
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);

		// Clear screen
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Create model matrices for three cubes
		std::vector<glm::mat4> models;

		float time = glfwGetTime();

		// Cube 1: Rotating around Y-axis at origin
		glm::mat4 model1 = glm::mat4(1.0f);
		model1 = glm::rotate(model1, time, glm::vec3(0.0f, 1.0f, 0.0f));
		models.push_back(model1);

		// Cube 2: Translated and rotating around X-axis
		glm::mat4 model2 = glm::mat4(1.0f);
		model2 = glm::translate(model2, glm::vec3(2.5f, 0.0f, 0.0f));
		model2 = glm::rotate(model2, time * 0.8f, glm::vec3(1.0f, 0.0f, 0.0f));
		models.push_back(model2);

		// Cube 3: Translated and rotating around Z-axis
		glm::mat4 model3 = glm::mat4(1.0f);
		model3 = glm::translate(model3, glm::vec3(-2.5f, 0.0f, 0.0f));
		model3 = glm::rotate(model3, time * 1.2f, glm::vec3(0.0f, 0.0f, 1.0f));
		models.push_back(model3);

		// Render cubes
		cubeRenderer.render(view, projection, models);

		// Swap buffers and poll events
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// Cleanup
	glfwTerminate();
	return 0;
}

*/

