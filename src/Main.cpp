#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <iostream>
#include <OpenGLHeaders/Shader.h>
#include <OpenGLHeaders/texture.h>
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

const float dimension = 800;
float numberOfPoints;

glm::mat4 zoom;
double zoomScaleFactor = 1;
const float zoomStep = 0.05f;

glm::mat4 pan;
glm::vec2 rightMouseRef;
glm::vec2 panOffset;
bool rightMousePressed = false;

bool firstPoint = true;
bool tabPressed = false;
bool shiftPressed = false;
unsigned int slopeVAO;
unsigned int slopeVBO;
float slopePoints[] {1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f};
unsigned int textShader = 0;
unsigned int initialSlopeText; //Texture 0
unsigned int finalSlopeText;   //Texture 1
unsigned int generalSlopeText; //Texture 2

glm::vec2 startSlope = glm::vec2(0.0f);
glm::vec2 endSlope = glm::vec2(0.0f);

bool configureSlope = false;

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
	glViewport(0, 0, width, height);
}

glm::vec2 screenToWorldCoordinates(glm::vec2 screenPos) {
	float xpos = screenPos.x;
	float ypos = screenPos.y;
	xpos -= dimension / 2;
	ypos -= dimension / 2;
	ypos = -ypos;
	screenPos = glm::vec2(xpos, ypos) * (1.0f / dimension);
	screenPos *= (float)(1.0f / zoomScaleFactor) * 2.0f;
	//screenPos = glm::vec3(glm::vec4(screenPos, 0.0f, 0.0f) * pan);
	return screenPos;
}

glm::vec2 screenToWorldCoordinates(float x, float y) {
	return screenToWorldCoordinates(glm::vec2(x, y));
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

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
	if(glfwGetKey(window, GLFW_KEY_TAB)) {
		tabPressed = true;
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, finalSlopeText);

		glm::vec2 lastPoint = controlPoints.back();
		slopePoints[0] = lastPoint.x; slopePoints[1] = lastPoint.y;
		glBindVertexArray(slopeVAO);
		glBindBuffer(GL_ARRAY_BUFFER, slopeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(slopePoints), slopePoints, GL_DYNAMIC_DRAW);
	}
	else if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT)) {
		shiftPressed = true;
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, initialSlopeText);

		glm::vec2 firstPoint = controlPoints.front();
		slopePoints[0] = firstPoint.x; slopePoints[1] = firstPoint.y;
		glBindVertexArray(slopeVAO);
		glBindBuffer(GL_ARRAY_BUFFER, slopeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(slopePoints), slopePoints, GL_DYNAMIC_DRAW);
	}
	if(glfwGetKey(window, GLFW_KEY_Z)) {
		controlPoints.erase(controlPoints.end());
		controlSlopes.erase(controlSlopes.end());
		// std::vector<std::vector<CubicSplineSegment>> xySplines = calculateFreeSpaceCubic(controlPoints, startSlope, endSlope);
		std::vector<std::vector<CubicSplineSegment>> xySplines = calculateFreeSpaceCubicHermite(controlPoints, controlSlopes);
		xCubicSpline = xySplines[0];
		yCubicSpline = xySplines[1];
		generatePointsFreeSpaceCubic();
	}
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
	if(rightMousePressed) {
		glm::vec2 newMouse = screenToWorldCoordinates(xpos, ypos);
		glm::vec2 newPan = newMouse - rightMouseRef;
		panOffset -= newPan;
		pan = glm::translate(pan, glm::vec3(newPan, 0.0f));
		rightMouseRef = newMouse;
	}
	if(shiftPressed || tabPressed || configureSlope) {
		glm::vec2 mousePoint = screenToWorldCoordinates(xpos, ypos) + panOffset;
		slopePoints[3] = mousePoint.x; slopePoints[4] = mousePoint.y;
		glBindVertexArray(slopeVAO);
		glBindBuffer(GL_ARRAY_BUFFER, slopeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(slopePoints), slopePoints, GL_DYNAMIC_DRAW);
	}
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT)
	{
		if (action == GLFW_PRESS)
		{
			if (firstPoint) {
				firstPoint = false;
				controlPoints = std::vector<glm::vec2>();
			}

			double xpos, ypos;
			glfwGetCursorPos(window, &xpos, &ypos);
			glm::vec2 gridPos = screenToWorldCoordinates(xpos, ypos) + panOffset;

			// if (tabPressed) {
			// 	endSlope = gridPos - controlPoints.back();
			// }
			// else if(shiftPressed) {
			// 	startSlope = gridPos - controlPoints.front();
			// }
			// else {
			// 	controlPoints.push_back(gridPos);
			// }
			// shiftPressed = false;
			// tabPressed = false;

			// controlPoints.erase(controlPoints.begin());
			// controlPoints = {glm::vec2(-1, 2), glm::vec2(0, 0), glm::vec2(1, -2), glm::vec2(2, 0)};
			// calculateCubic(controlPoints);
			// cubicSpline = calculateCubicStitched(controlPoints, 0, 0);
			// controlPoints = {glm::vec2(-0.54, 0.3375),
			// glm::vec2(0.0425, 0.36),
			// glm::vec2(0.0375, -0.115),
			// glm::vec2(-0.535, -0.175)};
			if(!configureSlope) {
				controlPoints.push_back(gridPos);
				configureSlope = true;

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, generalSlopeText);

				glm::vec2 lastPoint = gridPos;
				slopePoints[0] = lastPoint.x; slopePoints[1] = lastPoint.y;
				slopePoints[2] = lastPoint.x; slopePoints[3] = lastPoint.y;
				glBindVertexArray(slopeVAO);
				glBindBuffer(GL_ARRAY_BUFFER, slopeVBO);
				glBufferData(GL_ARRAY_BUFFER, sizeof(slopePoints), slopePoints, GL_DYNAMIC_DRAW);
			}
			else {
				configureSlope = false;
				controlSlopes.push_back(gridPos - controlPoints.back());
				// std::vector<std::vector<CubicSplineSegment>> xySplines = calculateFreeSpaceCubic(controlPoints, startSlope, endSlope);
			}
			auto start = std::chrono::high_resolution_clock::now();
			// std::vector<std::vector<CubicSplineSegment>> xySplines = calculateFreeSpaceCubic(controlPoints, startSlope, endSlope);
			std::vector<std::vector<CubicSplineSegment>> xySplines = calculateFreeSpaceCubicHermite(controlPoints, controlSlopes);
			// cubicSpline = calculateCubicHermite1Dimensional(controlPoints, controlSlopes);
			xCubicSpline = xySplines[0];
			yCubicSpline = xySplines[1];
			auto end = std::chrono::high_resolution_clock::now();
			auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

			generatePointsFreeSpaceCubic();
			// generatePointsCubic();
			// std::cout << duration << std::endl;
		}
	}

	if(button == GLFW_MOUSE_BUTTON_RIGHT) {
		if (action == GLFW_PRESS) {
			double xpos, ypos;
			glfwGetCursorPos(window, &xpos, &ypos);
			rightMousePressed = true;
			rightMouseRef = screenToWorldCoordinates(xpos, ypos);
		}
		else {
			rightMousePressed = false;
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
	
	//Drawing the line from point to cursor for slope
	glGenVertexArrays(1, &slopeVAO);
	glBindVertexArray(slopeVAO);
	glGenBuffers(1, &slopeVBO);
	glBindBuffer(GL_ARRAY_BUFFER, slopeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(slopePoints), slopePoints, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(0);

	//Create a Vertex Array Object
	unsigned int textVAO;
	glGenVertexArrays(1, &textVAO);
	glBindVertexArray(textVAO);

	float textVertices[]{
		//Vertices            //Texture coords
		-1.0f, -1.0f, 0.0f,   0.0f, 1.0f,
		-1.0f, -0.8f, 0.0f,   0.0f, 0.0f,
		0.0f,  -1.0f, 0.0f,   1.0f, 1.0f,
		0.0f,  -1.0f, 0.0f,   1.0f, 1.0f,
		-1.0f, -0.8f, 0.0f,   0.0f, 0.0f,
		0.0f,  -0.8f, 0.0f,   1.0f, 0.0f
	};

	float squareVertices[] {
		//Vertices            //Texture coords
		-1.0f, -1.0f, 0.0f,   0.0f, 1.0f,
		-1.0f,  1.0f, 0.0f,   0.0f, 0.0f,
		1.0f,  -1.0f, 0.0f,   1.0f, 1.0f,
		1.0f,  -1.0f, 0.0f,   1.0f, 1.0f,
		-1.0f,  1.0f, 0.0f,   0.0f, 0.0f,
		1.0f,   1.0f, 0.0f,   1.0f, 0.0f
	};

	//Create a Vertex Buffer Object
	unsigned int textVBO;
	glGenBuffers(1, &textVBO);
	//Bind the VBO to the object type and copy its data to the state
	glBindBuffer(GL_ARRAY_BUFFER, textVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(textVertices), textVertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	//Background data
	unsigned int backgroundVAO;
	glGenVertexArrays(1, &backgroundVAO);
	glBindVertexArray(backgroundVAO);
	unsigned int backgroundVBO;
	glGenBuffers(1, &backgroundVBO);
	glBindBuffer(GL_ARRAY_BUFFER, backgroundVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(squareVertices), squareVertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

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
	glBufferData(GL_ARRAY_BUFFER, splinePoints.size() * sizeof(GLfloat), splinePoints.data(), GL_DYNAMIC_DRAW);
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
	glBufferData(GL_ARRAY_BUFFER, controlFloats.size() * sizeof(GLfloat), controlFloats.data(), GL_DYNAMIC_DRAW);

	//Configure vertex data so readable by vertex shader
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(0);

	glm::vec3 objectColour = glm::vec3(1.0f, 0.5f, 0.31f);
	glm::vec3 lightColour = glm::vec3(1.0f, 1.0f, 1.0f);

	unsigned int splineShader = 0;
	Shader("Shaders/VertexSplines.vs", "Shaders/FragmentSplines.fs", splineShader);
	glUseProgram(splineShader);

	Shader("Shaders/VertexTexture.vs", "Shaders/FragmentTexture.fs", textShader);
	glUseProgram(textShader);
	unsigned int backgroundShader;
	Shader("Shaders/VertexTexture.vs", "Shaders/FragmentTexture.fs", backgroundShader);

	setInt(textShader, "text", 0);
	loadTexture(initialSlopeText, "textures/ConfiguringInitial.png");
	loadTexture(finalSlopeText, "textures/ConfiguringFinal.png");
	loadTexture(generalSlopeText, "textures/ConfiguringSlope.png");
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, initialSlopeText);

	glUseProgram(backgroundShader);
	setInt(backgroundShader, "text", 1);
	unsigned int backgroundTexture;
	loadTexture(backgroundTexture, "textures/VexField.png");
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, backgroundTexture);

	glEnable(GL_DEPTH_TEST);
	//Set mouse input callback function
	void mouse_callback(GLFWwindow * window, double xpos, double ypos);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetKeyCallback(window, key_callback);
	glPointSize(8);
	glLineWidth(3);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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

		//Pass matrices to the shader through a uniform
		glUseProgram(splineShader);
		setMat4(splineShader, "model", zoom * pan);

		if (tabPressed || shiftPressed || configureSlope) {
			// Draw text
			glActiveTexture(GL_TEXTURE0);
			glUseProgram(textShader);
			glBindVertexArray(textVAO);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glDrawArrays(GL_TRIANGLES, 0, ARRAY_SIZE(textVertices));

			//Draw Slope
			glUseProgram(splineShader);
			setVec3(splineShader, "colour", glm::vec3(1.0f, 0.0f, 0.0f));
			glBindVertexArray(slopeVAO);
			glDrawArrays(GL_LINE_STRIP, 0, 2);
		}

		glUseProgram(splineShader);

		setVec3(splineShader, "colour", glm::vec3(1.0f, 0.0f, 0.0f));
		glBindVertexArray(pointsVAO);
		glBindBuffer(GL_ARRAY_BUFFER, pointsVBO);
		glDrawArrays(GL_POINTS, 0, controlPoints.size() + debugPoints.size());

		setVec3(splineShader, "colour", glm::vec3(1.0f));
		glBindVertexArray(splineVAO);
		glBindBuffer(GL_ARRAY_BUFFER, splineVBO);
		glDrawArrays(GL_LINE_STRIP, 0, numberOfPoints);
		// glDrawArrays(GL_POINTS, 0, numberOfPoints);

		//Draw Background
		glActiveTexture(GL_TEXTURE1);
		glUseProgram(backgroundShader);
		setMat4(backgroundShader, "model", zoom * pan);
		glBindVertexArray(backgroundVAO);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDrawArrays(GL_TRIANGLES, 0, ARRAY_SIZE(squareVertices));

		//Swap buffer and poll IO events
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}