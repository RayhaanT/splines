#include <splines.h>
#include <GLFW/glfw3.h>
#include <glad/glad.h>

GLuint splineVBO;
GLuint splineVAO;
GLuint pointsVBO;
GLuint pointsVAO;

void generatePointsCubic() {
    std::vector<float> splinePoints;
    for(CubicSplineSegment s : cubicSpline) {
        for(float x = s.lowerBound; x < s.upperBound; x+=0.01) {
            splinePoints.push_back(x);
            float y = x * x * x * s.d;
            y += x * x * s.c;
            y += x * s.b;
            y += s.a;
            splinePoints.push_back(y);
            splinePoints.push_back(0.0f);
        }
    }

    std::vector<float> controlFloats;
    for (glm::vec2 v : controlPoints)
    {
        controlFloats.push_back(v.x);
        controlFloats.push_back(v.y);
        controlFloats.push_back(0.0f);
    }

    glBindVertexArray(splineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, splineVBO);
    glBufferData(GL_ARRAY_BUFFER, splinePoints.size() * sizeof(GLfloat), splinePoints.data(), GL_STATIC_DRAW);
    numberOfPoints = splinePoints.size() / 3;

    glBindVertexArray(pointsVAO);
    glBindBuffer(GL_ARRAY_BUFFER, pointsVAO);
    glBufferData(GL_ARRAY_BUFFER, controlFloats.size() * sizeof(GLfloat), controlFloats.data(), GL_STATIC_DRAW);
}
