#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <iostream>
#include <OpenGLHeaders/Shader.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define STB_IMAGE_IMPLEMENTATION
#define ARRAY_SIZE(array) (sizeof((array)) / sizeof((array[0])))
#include <GLFW/stb_image.h>
#include <vector>
#include <splines.h>
#include <chrono>
///VBOs Vertex Buffer Objects contain vertex data that is sent to memory in the GPU, vertex attrib calls config bound VBO
///VAOs Vertex Array Objects when bound, any vertex attribute calls and attribute configs are stored in VAO
///Having multiple VAOs allow storage of multiple VBO configs, before drawing, binding VAO with right config applies to draw
///Vertex attributes are simply inputs of the vertex shader, configured so vertex shader knows how to interpret data
///EBOs Element Buffer Objects stores indices used for OpenGL to know which vertices to draw
///EBOs used when needing multiple triangles to render but triangles have overlapping points
///E.G. rectangle made with 2 triangles have 2 overlapping vertices
///Vertex shader changes 3D coords of vertices
///Fragment shader sets color of pixels
///OpenGL objects are used to reference pieces of OpenGL state machine
///E.G. when creating a shader program,  a program is created in the state machine and its ID
///is passed to the program object
///Similarly, with the VBO, the real VBO is stored in the background state machine, the
///object holds the ID of the real object and its value is bound to the real object
///With ALL (or most) OpenGL objects, they must be bound so that any function calls for that object type configures the
///ID you created
///Depth information stored in Z buffer, depth testing done automatically, must be enabled
///Depth buffer must also be cleared in the clear function

float deltaTime = 0.0f;
float lastFrame = 0.0f;
bool restrictY = true;

const float dimension = 800;
float numberOfPoints;

//Define offset variables
float lastX = dimension / 2;
float lastY = dimension / 2;
float yaw = -90;
float pitch = 0;
bool firstMouse = true;
float fov = 45.0f;
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

glm::mat4 zoom;
double zoomScaleFactor = 1;
const float zoomStep = 0.05f;

bool firstPoint = true;

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
	if(yoffset > 0) {
		zoom = glm::scale(zoom, glm::vec3(1.0f + zoomStep));
		zoomScaleFactor *= (1 + zoomStep);
	}
	else {
		zoom = glm::scale(zoom, glm::vec3(1.0f - zoomStep));
		zoomScaleFactor *= (1 - zoomStep);
	}
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT)
	{
		if (action == GLFW_PRESS)
		{
			if(firstPoint) {
				firstPoint = false;
				controlPoints = std::vector<glm::vec2>();
			}
			double xpos, ypos;
			glfwGetCursorPos(window, &xpos, &ypos);
			xpos -= dimension/2;
			ypos -= dimension/2;
			ypos = -ypos;
			xpos /= dimension;
			ypos /= dimension;
			controlPoints.push_back(glm::vec2(xpos, ypos) * (float)(1.0f/zoomScaleFactor) * 2.0f);
			// controlPoints.erase(controlPoints.begin());
			// controlPoints = {glm::vec2(-1, 2), glm::vec2(0, 0), glm::vec2(1, -2), glm::vec2(2, 0)};
			// calculateCubic(controlPoints);
			auto start = std::chrono::high_resolution_clock::now();
			// cubicSpline = calculateCubicStitched(controlPoints, 0, 0);
			std::vector<std::vector<CubicSplineSegment>> xySplines = calculateFreeSpaceCubic(controlPoints, 0, 0);
			xCubicSpline = xySplines[0];
			yCubicSpline = xySplines[1];
			auto end = std::chrono::high_resolution_clock::now();
			auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
			// generatePointsCubic();
			generatePointsFreeSpaceCubic();
			// std::cout << duration << std::endl;
		}
	}
}

int main()
{
	controlPoints = {glm::vec2(0.0f, 0.0f), glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 1.0f), glm::vec2(-1.0f, -1.0f)};

	//Initialize GLFW
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//Create a GLFW Window
	GLFWwindow *window = glfwCreateWindow(dimension, dimension, "Splines", NULL, NULL);
	glfwMakeContextCurrent(window);

	//glad init: intializes all OpenGL function pointers
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	//Set size of rendering window
	glViewport(0, 0, dimension, dimension);

	void framebuffer_size_callback(GLFWwindow * window, int width, int height);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);	
	
	//Create a Vertex Array Object
	unsigned int VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	float vertices[]{
		//Vertices
		RANGE, RANGE, 0.0f,
		RANGE, -RANGE, 0.0f,
		-RANGE, -RANGE, 0.0f,
		-RANGE, RANGE, 0.0f,
		RANGE, RANGE, 0.0f,
		-RANGE, -RANGE, 0.0f
	};

	//Create a Vertex Buffer Object
	unsigned int VBO;
	glGenBuffers(1, &VBO);
	//Bind the VBO to the object type and copy its data to the state
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(0);

	//Create a Vertex Array Object
	glGenVertexArrays(1, &splineVAO);
	glBindVertexArray(splineVAO);

	std::vector<float> splinePoints;
	for (float x = -RANGE; x <= RANGE; x += 0.1) {
		splinePoints.push_back(x);
		splinePoints.push_back(x * x * x);
		splinePoints.push_back(0.0f);
	}
	
	glGenBuffers(1, &splineVBO);
	glBindBuffer(GL_ARRAY_BUFFER, splineVBO);
	glBufferData(GL_ARRAY_BUFFER, splinePoints.size() * sizeof(GLfloat), splinePoints.data(), GL_STATIC_DRAW);
	numberOfPoints = splinePoints.size() / 3;

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(0);

	glGenVertexArrays(1, &pointsVAO);
	glBindVertexArray(pointsVAO);

	std::vector<float> controlFloats;
	for(int i = 0; i < controlPoints.size(); i++) {
		controlFloats.push_back(controlPoints[i].x);
		controlFloats.push_back(controlPoints[i].y);
		controlFloats.push_back(0.0f);
	}

	glGenBuffers(1, &pointsVBO);
	glBindBuffer(GL_ARRAY_BUFFER, pointsVBO);
	glBufferData(GL_ARRAY_BUFFER, controlFloats.size() * sizeof(GLfloat), controlFloats.data(), GL_STATIC_DRAW);

	//Configure vertex data so readable by vertex shader
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(0);

	glm::vec3 objectColour = glm::vec3(1.0f, 0.5f, 0.31f);
	glm::vec3 lightColour = glm::vec3(1.0f, 1.0f, 1.0f);

	unsigned int splineShader = 0;
	Shader("Shaders/VeShPhong.vs", "Shaders/FrShColour.fs", splineShader);
	glUseProgram(splineShader);

	unsigned int backgroundShader = splineShader;

	glEnable(GL_DEPTH_TEST);
	//Set mouse input callback function
	void mouse_callback(GLFWwindow * window, double xpos, double ypos);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glPointSize(8);
	glLineWidth(3);

	//Render Loop
	while (!glfwWindowShouldClose(window))
	{
		if (glfwGetKey(window, GLFW_KEY_ESCAPE)) {
			glfwSetWindowShouldClose(window, true);
		}

		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Change colour over time
		lightColour.x = sin(glfwGetTime() * 2.0f);
		lightColour.y = sin(glfwGetTime() * 0.7f);
		lightColour.z = sin(glfwGetTime() * 1.3f);

		glm::vec3 ambientColour = lightColour * glm::vec3(0.5f);

		glUseProgram(splineShader);

		glm::mat4 model;
		//Pass matrices to the shader through a uniform
		setMat4(splineShader, "model", zoom);

		setVec3(splineShader, "colour", glm::vec3(1.0f, 0.0f, 0.0f));
		glBindVertexArray(pointsVAO);
		glBindBuffer(GL_ARRAY_BUFFER, pointsVBO);
		glDrawArrays(GL_POINTS, 0, controlPoints.size());

		setVec3(splineShader, "colour", glm::vec3(1.0f));
		glBindVertexArray(splineVAO);
		glBindBuffer(GL_ARRAY_BUFFER, splineVBO);
		glDrawArrays(GL_LINE_STRIP, 0, numberOfPoints);
		// glDrawArrays(GL_POINTS, 0, numberOfPoints);

		glBindVertexArray(VAO);

		glUseProgram(backgroundShader);

		setVec3(backgroundShader, "colour", glm::vec3(0.2f));
		setMat4(backgroundShader, "model", zoom);

		// Draw cube
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		// glDrawArrays(GL_TRIANGLES, 0, ARRAY_SIZE(vertices));

		//Swap buffer and poll IO events
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}