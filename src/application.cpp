#include "application.h"

#include "glad/glad.h"
#include <iostream>
#include "GLFW/glfw3.h"

// Example usage with GLFW
Application::Application(){
	initOpenGL();
	init_fbo();
	rubberband = new RubberbandSelection(static_cast<float> (windowWidth), static_cast<float> (windowHeight));
	//instanced_renderer_ = new InstancedRenderer;
	cube_renderer_ = new CubeRenderer;
	cube_renderer_->set_section_mode(false);
	camera = new Camera(glm::vec3(0.f, 0.f, 8.f));
	cam_ctrl = new CameraController(camera, static_cast<float> (windowWidth), static_cast<float> (windowHeight));
	camera->setAspectRatio(static_cast<float>(windowWidth) / windowHeight);
	//camera->setCameraType(CameraType::PERSPECTIVE);
	setupCallbacks();
}

Application::~Application() {
	delete rubberband;
	delete cam_ctrl;
	delete camera;
	glfwTerminate();
}

void Application::initOpenGL() {

	if (!glfwInit())
	{
		std::cout << "Failed to initialize GLFW" << std::endl;
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(windowWidth, windowHeight, "Rubberband Selection", NULL, NULL);
	if (!window) {
		std::cout << "Failed to create GLFW window\n";
		glfwTerminate();
		return;
	}

	glfwMakeContextCurrent(window);


	// Enable v-sync
	glfwSwapInterval(1);

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

	//glEnable(GL_DEPTH_TEST);


}

void Application::setupCallbacks()
{
	glfwSetWindowUserPointer(window, this);

	glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods) {
		Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
	app->mouseButtonCallback(button, action, mods);
		});

	glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xpos, double ypos) {
		Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
	    app->mouseMoveCallback(xpos, ypos);
	});

	glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height) {
		Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
	    app->framebufferSizeCallback(width, height);
	});

	glfwSetScrollCallback(window, [](GLFWwindow* window, double xoffset, double yoffset) {
		Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
		app->cam_ctrl->scrollCallback(static_cast<float>(yoffset)); });

	glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scan, int action, int mods)
	{
		Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
		app->cam_ctrl->keyCallback(key, action); });
}

void Application::mouseButtonCallback(int button, int action, int mods) {

	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	int leftCtrlState = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL);
	int rightCtrlState = glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL);
	// Check if Ctrl (either left or right) and Left Mouse Button are pressed.
	
	if (button == GLFW_MOUSE_BUTTON_LEFT && 
		action == 1 &&
		(leftCtrlState == GLFW_PRESS || rightCtrlState == GLFW_PRESS)) 
	{
		rubberband_active = true;
		rubberband->startSelection(xpos, ypos);
		cube_renderer_->set_section_mode(false);

	}
	else if (rubberband_active  && 
		button == GLFW_MOUSE_BUTTON_LEFT &&
		action == 0)
	{
		rubberband_active = false;
		glm::vec2 start, end;
		rubberband->endSelection(start, end);
		cube_renderer_->set_section_mode(true);

		float x = std::min(start.x, end.x);
		float w = std::max(start.x, end.x) - x;
		float y = std::min(start.y, end.y);
		float h = std::max(start.y, end.y) - y;

		cube_renderer_->set_selection_rectangle(x, y, w, h);
		//select_in_rectangle(start.x, start.y, end.x, end.y);
	}
	else
	{
		cam_ctrl->mouseButtonCallback(button, action, xpos, ypos);
	}
}

void Application::mouseMoveCallback(double xpos, double ypos) {

	if(rubberband_active)
	{
		rubberband->updateSelection(xpos, ypos);
	}
	else
	{
		cam_ctrl->mouseMoveCallback(xpos, ypos);
	}
	//
}

void Application::framebufferSizeCallback(int width, int height) {
	windowWidth = width;
	windowHeight = height;
	glViewport(0, 0, width, height);
	rubberband->updateScreenSize(width, height);
}

void Application::select_in_rectangle(float st_x, float st_y, float end_x, float end_y)
{
	// Clear screen
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// Render your scene here
	glm::mat4 view = camera->getViewMatrix();
	glm::mat4 projection = camera->getProjectionMatrix();

		cube_renderer_->set_section_mode(false);
		cube_renderer_->pick_render(view, projection, m_models, {});
		glfwSwapBuffers(window);

	
}

void Application::update_models()
{
	auto getModelMat = [](glm::vec3 pos) -> glm::mat4
	{
		glm::mat4 model3 = glm::mat4(1.0f);
		model3 = glm::translate(model3, pos);

		model3 = glm::rotate(model3, glm::radians(30.f), glm::vec3(1.0f, 0.0f, 0.0f));
		model3 = glm::rotate(model3, glm::radians(15.f), glm::vec3(0.0f, 1.0f, 0.0f));
		return model3;
	};

	m_models.push_back(getModelMat(glm::vec3(3.0f, 0.0f, 0.0f)));
	m_models.push_back(getModelMat(glm::vec3(0.0f, 0.0f, 0.0f)));
	m_models.push_back(getModelMat(glm::vec3(-3.0f, 0.0f, 0.0f)));
}

void Application::draw_scene()
{
	// Clear screen
	bool fbo_on = false;

	if (cube_renderer_->get_section_mode())
	{
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);
        glEnable(GL_DEPTH_TEST);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		fbo_on = true;
	}
	else
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// Render your scene here
	glm::mat4 view = camera->getViewMatrix();
	glm::mat4 projection = camera->getProjectionMatrix();
	//cube_renderer_->set_section_mode(false);
	cube_renderer_->render(view, projection, m_models, {});

	if(fbo_on) 
	{
        glBindFramebuffer(GL_FRAMEBUFFER, 0); // Render to screen.
	}
}


void Application::init_fbo() 
{
	// Create a framebuffer object
	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	// create a color attachment texture
    GLuint textureColorbuffer;
    glGenTextures(1, &textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowWidth, windowHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);

// Create depth buffer
    GLuint depthBuffer;
    glGenRenderbuffers(1, &depthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, windowWidth, windowHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
    
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "Framebuffer not complete!" << std::endl;
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

}
void Application::run() {

	//instanced_renderer_->addInstance(Transform(glm::vec3(0.0f, 0.0f, 0.0f)));
	//instanced_renderer_->addInstance(Transform(glm::vec3(2.0f, 0.0f, 0.0f), glm::vec3(30.0f, 30.0f, 0.0f)));
	//instanced_renderer_->addInstance(Transform(glm::vec3(-2.0f, 0.0f, 0.0f), glm::vec3(30.0f, 45.0f, 0.f), glm::vec3(0.5f)));
	update_models();
	glEnable(GL_DEPTH_TEST);
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		draw_scene();
		// Render rubberband selection on top
		rubberband->render();

		glfwSwapBuffers(window);
	}
}
