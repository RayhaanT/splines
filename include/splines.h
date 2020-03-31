#pragma once
#include "Eigen/Dense"
#include <cmath>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <vector>

#define RANGE 20

struct ControlPoint {
    float x, y, angle;
};

// Spline data
extern std::vector<glm::vec2> controlPoints;
extern Eigen::Vector4d coefficients;

// GL containers
extern GLuint splineVBO;
extern GLuint splineVAO;
extern GLuint pointsVBO;
extern GLuint pointsVAO;

void calculateCubic(std::vector<glm::vec2> points);
void generatePointsCubic();
