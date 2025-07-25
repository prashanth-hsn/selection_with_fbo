#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include "cube_vbo.h"

#include <algorithm>
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



const char* picking_vertexSrc = R"(
#version 330 compatibility
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	 gl_Position = projection * view * model * vec4(aPos, 1.0);	
}
)";


const char* picking_fragmentSrc = R"(
#version 330 compatibility
// Ouput data
out vec4 color;

uniform vec4 PickingColor;

void main()
{
	color = PickingColor;
}
)";

CubeRenderer::CubeRenderer() {
	shaderProgram = setupShaders(vertexShaderSource, fragmentShaderSource);
	getUniformLocations();
	pickShaderPrg = setupShaders(picking_vertexSrc, picking_fragmentSrc);
	updatePickingUniformLocs();
	setupBuffers();
}

CubeRenderer::~CubeRenderer() {
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	glDeleteProgram(shaderProgram);
}

bool CubeRenderer::get_section_mode()
{
	return selection_mode;
}

GLuint CubeRenderer::setupShaders(const char* vertex_src, const char* frag_src) {
	// Compile vertex shader
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertex_src, NULL);
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
	glShaderSource(fragmentShader, 1, &frag_src, NULL);
	glCompileShader(fragmentShader);

	// Check for fragment shader compile errors
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cerr << "Fragment shader compilation failed: " << infoLog << std::endl;
	}

	// Link shaders
	GLuint shader_program = glCreateProgram();
	glAttachShader(shader_program, vertexShader);
	glAttachShader(shader_program, fragmentShader);
	glLinkProgram(shader_program);

	// Check for linking errors
	glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shader_program, 512, NULL, infoLog);
		std::cerr << "Shader program linking failed: " << infoLog << std::endl;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);


	return shader_program;

}

void CubeRenderer::getUniformLocations()
{
	// Get uniform locations
	modelLoc = glGetUniformLocation(shaderProgram, "model");
	viewLoc = glGetUniformLocation(shaderProgram, "view");
	projectionLoc = glGetUniformLocation(shaderProgram, "projection");
	selectedLoc = glGetUniformLocation(shaderProgram, "selected");

}

void CubeRenderer::updatePickingUniformLocs()
{
	// Get uniform locations
	p_modelLoc = glGetUniformLocation(pickShaderPrg, "model");
	p_viewLoc= glGetUniformLocation(pickShaderPrg, "view");
	p_projectionLoc = glGetUniformLocation(pickShaderPrg, "projection");
	p_picking_color = glGetUniformLocation(pickShaderPrg, "PickingColor");
}

void CubeRenderer::setupBuffers() {


	float size = 0.1f;
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

void CubeRenderer::set_selection_rectangle(float x, float y, float w, float h)
{
	sel_x = std::abs(x);
	sel_y = std::abs(y);
	sel_w = std::abs(w);
	sel_h = std::abs(h);

}

void CubeRenderer::render(const glm::mat4& view, const glm::mat4& projection, const std::vector<glm::mat4>& models, const std::set<int>& selected )
{

	glUseProgram(selection_mode ? pickShaderPrg : shaderProgram);

	if(selection_mode)
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	// Set view and projection matrices

	glUniformMatrix4fv((selection_mode) ? p_viewLoc : viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv((selection_mode) ? p_projectionLoc : projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
	
	glBindVertexArray(VAO);

	int model_id = 100;
	// Render each cube with its model matrix
	for (const auto& model : models) {

		glUniformMatrix4fv((selection_mode) ? p_modelLoc: modelLoc, 1, GL_FALSE, glm::value_ptr(model));

		// Convert "i", the integer mesh ID, into an RGB color
		if(selection_mode)
		{
			int r = (model_id & 0x000000FF) >> 0;
			int g = (model_id & 0x0000FF00) >> 8;
			int b = (model_id & 0x00FF0000) >> 16;
			// OpenGL expects colors to be in [0,1], so divide by 255.
			glUniform4f(p_picking_color, r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);
		}

		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		++model_id;
	}


	if(selection_mode)
	{
		glFlush();
		glFinish();

		std::set<int> sel_ids;
		//glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		int width = static_cast<int>(sel_w);
		int height = static_cast<int>(sel_h);


		std::vector<unsigned char> pixels = readFrameBufferPixels(static_cast<int>(sel_x), static_cast<int>(sel_y), width, height);
		for(size_t i = 0; i < pixels.size()/4; i++)
		{
			int pickedID = pixels [i*4] + pixels[i*4 + 1] * 256 + pixels[ i*4 + 2] * 256 * 256;
			if (pickedID != 0x00ffffff) {
				sel_ids.insert(pickedID);
			}
		}

		std::ranges::for_each(sel_ids, [](int i) { std::cout << "selected id: " << i << "\n"; });
		selection_mode = false;
	}

	glBindVertexArray(0);
}

void CubeRenderer::pick_render(const glm::mat4& view, const glm::mat4& projection, const std::vector<glm::mat4>& models,
	const std::set<int>& selected)
{

	glUseProgram(pickShaderPrg);

	// Set view and projection matrices
	glUniformMatrix4fv(p_viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(p_projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));


	glBindVertexArray(VAO);

	int model_id = 100;
	// Render each cube with its model matrix
	for (const auto& model : models) {


		// Convert "i", the integer mesh ID, into an RGB color
		int r = (model_id & 0x000000FF) >> 0;
		int g = (model_id & 0x0000FF00) >> 8;
		int b = (model_id & 0x00FF0000) >> 16;

		// OpenGL expects colors to be in [0,1], so divide by 255.
		glUniform4f(p_picking_color, r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		++model_id;
	}

	glBindVertexArray(0);
}

std::vector<unsigned char> CubeRenderer::readFrameBufferPixels(int x, int y, int width, int height)
{
	// Function to read pixels from the framebuffer
	GLenum format = GL_RGBA;
	GLenum type = GL_UNSIGNED_BYTE;
	int numChannels = 4;

	// Determine the size of each component in bytes
	int bytesPerComponent = 1;

	size_t bufferSize = static_cast<size_t>(width) * height * numChannels * bytesPerComponent;
	std::vector<unsigned char> pixels(bufferSize);

	// It's good practice to ensure all pending OpenGL commands are executed
	// before reading pixels. This can prevent unexpected results, though
	// glReadPixels often implicitly flushes.
	glFinish();

	// Read the pixels
	glReadPixels(x, y, width, height, format, type, pixels.data());

	// Check for OpenGL errors
	GLenum error = glGetError();
	if (error != GL_NO_ERROR) {
		std::cerr << "OpenGL Error: " << error << " during glReadPixels." << std::endl;
	}

	return pixels;
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

