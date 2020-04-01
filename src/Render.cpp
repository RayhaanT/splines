#include <splines.h>
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <iostream>

GLuint splineVBO;
GLuint splineVAO;
GLuint pointsVBO;
GLuint pointsVAO;

void generateControlPointVertices() {
    std::vector<float> controlFloats;
    for (glm::vec2 v : controlPoints) {
        controlFloats.push_back(v.x);
        controlFloats.push_back(v.y);
        controlFloats.push_back(0.0f);
    }

    glBindVertexArray(pointsVAO);
    glBindBuffer(GL_ARRAY_BUFFER, pointsVAO);
    glBufferData(GL_ARRAY_BUFFER, controlFloats.size() * sizeof(GLfloat), controlFloats.data(), GL_STATIC_DRAW);
}

void generatePointsCubic() {
    std::vector<float> splinePoints;
    for(CubicSplineSegment s : cubicSpline) {
        for(float x = 0; x < 1; x+=0.01) {
            splinePoints.push_back((x * s.parameterMultiplier) + s.parameterOffset);
            float y = x * x * x * s.d;
            y += x * x * s.c;
            y += x * s.b;
            y += s.a;
            splinePoints.push_back(y);
            splinePoints.push_back(0.0f);
        }
    }

    glBindVertexArray(splineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, splineVBO);
    glBufferData(GL_ARRAY_BUFFER, splinePoints.size() * sizeof(GLfloat), splinePoints.data(), GL_STATIC_DRAW);
    numberOfPoints = splinePoints.size() / 3;

    generateControlPointVertices();
}

void generatePointsFreeSpaceCubic() {
    std::vector<float> splinePoints;
    for(int i = 0; i < xCubicSpline.size(); i++) {
        for(float t = 0; t < 1; t+=0.01f) {
            //X coord
            float x = t * t * t * xCubicSpline[i].d;
            x += t * t * xCubicSpline[i].c;
            x += t * xCubicSpline[i].b;
            x += xCubicSpline[i].a;
            splinePoints.push_back(x);

            //Y coord
            float y = t * t * t * yCubicSpline[i].d;
            y += t * t * yCubicSpline[i].c;
            y += t * yCubicSpline[i].b;
            y += yCubicSpline[i].a;
            splinePoints.push_back(y);

            splinePoints.push_back(0.0f);
        }
        std::cout << "I: " << i << std::endl;
        Eigen::Vector4d xCoeffs(xCubicSpline[i].d, xCubicSpline[i].c, xCubicSpline[i].b, xCubicSpline[i].a);
        Eigen::Vector4d yCoeffs(yCubicSpline[i].d, yCubicSpline[i].c, yCubicSpline[i].b, yCubicSpline[i].a);
        std::cout << xCoeffs;
        std::cout << std::endl;
        std::cout << yCoeffs;
        std::cout << std::endl;
    }

    glBindVertexArray(splineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, splineVBO);
    glBufferData(GL_ARRAY_BUFFER, splinePoints.size() * sizeof(GLfloat), splinePoints.data(), GL_STATIC_DRAW);
    numberOfPoints = splinePoints.size() / 3;

    generateControlPointVertices();
}
