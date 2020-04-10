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
    float parameterMultiplier;
    float parameterOffset, outputOffset;

    CubicSplineSegment(const Eigen::Vector4d &v) {
        a = v[0];
        b = v[1];
        c = v[2];
        d = v[3];
    }

    CubicSplineSegment() {}
};

// Spline data
extern std::vector<glm::vec2> controlPoints;
extern std::vector<glm::vec2> debugPoints;
extern std::vector<glm::vec2> controlSlopes;
extern std::vector<CubicSplineSegment> cubicSpline;
extern std::vector<CubicSplineSegment> xCubicSpline;
extern std::vector<CubicSplineSegment> yCubicSpline;
extern float numberOfPoints;

// GL containers
extern GLuint splineVBO;
extern GLuint splineVAO;
extern GLuint pointsVBO;
extern GLuint pointsVAO;

void generatePointsCubic();
void generatePointsFreeSpaceCubic();
void calculateCubic(std::vector<glm::vec2> points);
std::vector<CubicSplineSegment> calculateCubicStitched(std::vector<glm::vec2> points, float startSlope, float endSlope, bool linear);
std::vector<std::vector<CubicSplineSegment>> calculateFreeSpaceCubic(std::vector<glm::vec2> points, glm::vec2 startSlope, glm::vec2 endSlope);
std::vector<CubicSplineSegment> calculateCubicHermite(std::vector<glm::vec2> points, std::vector<float> slopes);
std::vector<std::vector<CubicSplineSegment>> calculateFreeSpaceCubicHermite(std::vector<glm::vec2> points, std::vector<glm::vec2> slopes);
std::vector<CubicSplineSegment> calculateCubicHermite1Dimensional(std::vector<glm::vec2> points, std::vector<glm::vec2> slopes);