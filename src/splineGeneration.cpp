#include "splines.h"
#include "Eigen/Core"
#include <iostream>
#include <algorithm>

using namespace Eigen;

std::vector<glm::vec2> controlPoints;
std::vector<CubicSplineSegment> cubicSpline;

void calculateCubic(std::vector<glm::vec2> points)
{
    Eigen::MatrixXd mat(4, 4);
    Eigen::Vector4d y;
    y(0) = points[0].y;
    y(1) = points[1].y;
    y(2) = points[2].y;
    y(2) = points[2].y;
    for (int i = 0; i < 4; i++)
    {
        float a = pow(points[i].x, 3);
        float b = pow(points[i].x, 2);
        float c = points[i].x;
        mat(i, 0) = 1;
        mat(i, 1) = c;
        mat(i, 2) = b;
        mat(i, 3) = a;
    }

    Eigen::Vector4d coefficients = mat.inverse() * y;
    CubicSplineSegment c(coefficients);
    c.lowerBound = -RANGE;
    c.upperBound = RANGE;
    cubicSpline = {c};
}

bool xValueSort(glm::vec2 a, glm::vec2 b) {
    return a.x < b.x;
}

// Non-paramaterized, non-localized
void calculateCubicStitched(std::vector<glm::vec2> points, float startSlope, float endSlope) {
    std::sort(points.begin(), points.end(), xValueSort);
    int numVar = (points.size()-1)*4;
    MatrixXd mat(numVar, numVar);
    VectorXd y(numVar);
    int matIndex = 0;

    for(int i = 0; i < points.size() - 1; i++) {
        float x0 = points[i].x;
        float x1 = points[i+1].x;
        int matOffset = i * 4;
        int secondOffset = matOffset + 4;

        //Starting slope: b + cx + 2dx^2 = s
        if(i == 0) {
            mat(matIndex, 1) = 1;
            mat(matIndex, 2) = x0;
            mat(matIndex, 3) = 2 * x0 * x0;
            y(matIndex) = startSlope;
            matIndex++;
        }

        //Pass point through point 0: a + bx + cx^2 + dx^3 = y
        mat(matIndex, matOffset) = 1;
        mat(matIndex, matOffset + 1) = x0;
        mat(matIndex, matOffset + 2) = x0 * x0;
        mat(matIndex, matOffset + 3) = x0 * x0 * x0;
        y(matIndex) = points[i].y;
        matIndex++;

        //Pass point through point 1: a + bx + cx^2 + dx^3 = y
        mat(matIndex, matOffset) = 1;
        mat(matIndex, matOffset + 1) = x1;
        mat(matIndex, matOffset + 2) = x1 * x1;
        mat(matIndex, matOffset + 3) = x1 * x1 * x1;
        y(matIndex) = points[i + 1].y;
        matIndex++;

        //Match slopes and curvature (C1 and C2 continuity)
        if(i < points.size() - 2) {
            //Match slopes: b(i) + 2c(i)x(i+1) + 3d(i)x^2(i+1) − b(i+1) − 2c(i+1)x(i+1) − 3d(i+1)x^2(i+1) = 0
            mat(matIndex, matOffset + 1) = 1;
            mat(matIndex, matOffset + 2) = 2 * x1;
            mat(matIndex, matOffset + 3) = 3 * x1 * x1;
            mat(matIndex, secondOffset + 1) = -1;
            mat(matIndex, secondOffset + 2) = -2 * x1;
            mat(matIndex, secondOffset + 3) = -3 * x1 * x1;
            y(matIndex) = 0;
            matIndex++;

            //Match curvature: 2c(i) + 6d(i)x(i+1) − 2c(i+1) − 6d(i+1)x(i+1) = 0
            mat(matIndex, matOffset + 2) = 2;
            mat(matIndex, matOffset + 3) = 6 * x1;
            mat(matIndex, secondOffset + 2) = -2;
            mat(matIndex, secondOffset + 3) = -6 * x1;
            y(matIndex) = 0;
            matIndex++;
        }
        else {
            //End slope: b + cx + 2dx^2 = s
            mat(matIndex, matOffset + 1) = 1;
            mat(matIndex, matOffset + 2) = x1;
            mat(matIndex, matOffset + 3) = 2 * x1 * x1;
            y(matIndex) = endSlope;
            matIndex++;
        }
    }

    cubicSpline.clear();

    VectorXd coefficients = mat.inverse() * y;
    for(int i = 0; i < points.size() - 1; i++) {
        Vector4d v = coefficients(seq(i*4, (i*4)+3));
        CubicSplineSegment c(v);
        c.lowerBound = points[i].x;
        c.upperBound = points[i + 1].x;
        cubicSpline.push_back(c);
    }
}