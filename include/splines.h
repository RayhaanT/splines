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

struct CubicSplineSegment {
    float a, b, c, d;
    float lowerBound;
    float upperBound;

    CubicSplineSegment(const Eigen::Vector4d &v) {
        a = v[0];
        b = v[1];
        c = v[2];
        d = v[3];
    }
};

// Spline data
extern std::vector<glm::vec2> controlPoints;
extern std::vector<CubicSplineSegment> cubicSpline;
extern float numberOfPoints;

// GL containers
extern GLuint splineVBO;
extern GLuint splineVAO;
extern GLuint pointsVBO;
extern GLuint pointsVAO;

void calculateCubic(std::vector<glm::vec2> points);
void generatePointsCubic();
