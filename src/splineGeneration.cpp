#include "splines.h"
#include "Eigen/Core"
#include <iostream>
#include <algorithm>

using namespace Eigen;

//Extern definitions
std::vector<glm::vec2> controlPoints;
std::vector<CubicSplineSegment> cubicSpline;
std::vector<CubicSplineSegment> xCubicSpline;
std::vector<CubicSplineSegment> yCubicSpline;

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
    c.parameterMultiplier = 40;
    c.parameterOffset = -20;
    cubicSpline = {c};
}

bool xValueSort(glm::vec2 a, glm::vec2 b) {
    return a.x < b.x;
}

std::vector<std::vector<CubicSplineSegment>> calculateFreeSpaceCubic(std::vector<glm::vec2> points, glm::vec2 startSlope, glm::vec2 endSlope) {
    std::vector<glm::vec2> xPoints;
    std::vector<glm::vec2> yPoints;
    for(int i = 0; i < points.size(); i++) {
        xPoints.push_back(glm::vec2(i, points[i].x));
        yPoints.push_back(glm::vec2(i, points[i].y));
    }

    std::vector<CubicSplineSegment> xSpline;
    std::vector<CubicSplineSegment> ySpline;

    if(points.size() > 1) {
        float startXDerivative = 1.0f / (points[1].x - points[0].x);
        float startYDerivative = 1.0f / (points[1].y - points[0].y);
        
        float startXSlope = startXDerivative * startSlope.x;
        float startYSlope = startYDerivative * startSlope.y;

        int n = points.size() - 1;
        float endXDerivative = 1.0f / (points[n].x - points[n-1].x);
        float endYDerivative = 1.0f / (points[n].y - points[n-1].y);

        float endXSlope = endXDerivative * endSlope.x;
        float endYSlope = endYDerivative * endSlope.y;

        xSpline = calculateCubicStitched(xPoints, startXSlope, endXSlope);
        ySpline = calculateCubicStitched(yPoints, startYSlope, endYSlope);
    }
    else {
        xSpline = calculateCubicStitched(xPoints, 1, 1);
        ySpline = calculateCubicStitched(yPoints, 1, 1);
    }

    return {xSpline, ySpline};
}

// Non-paramaterized, non-localized
std::vector<CubicSplineSegment> calculateCubicStitched(std::vector<glm::vec2> points, float startSlope, float endSlope) {
    std::sort(points.begin(), points.end(), xValueSort);
    int numVar = (points.size()-1)*4;
    MatrixXd mat(numVar, numVar);
    VectorXd y(numVar);
    int matIndex = 0;

    for(int i = 0; i < points.size() - 1; i++) {
        float x0 = 0;
        float x1 = 1;
        float realX0 = points[i].x;
        float realX1 = points[i+1].x;
        int matOffset = i * 4;
        int secondOffset = matOffset + 4;

        //Starting slope: b + cx + 2dx^2 = s
        if(i == 0) {
            // mat(matIndex, 1) = 1;
            // mat(matIndex, 2) = x0;
            // mat(matIndex, 3) = 2 * x0 * x0;
            // y(matIndex) = startSlope;

            //Starting slope paramaterized: b = (x1 - x0)s
            mat(matIndex, 1) = 1;
            y(matIndex) = (realX1-realX0) * startSlope;
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
            float realX2 = points[i + 2].x;

            //Match slopes: b(i) + 2c(i)x(i+1) + 3d(i)x^2(i+1) − b(i+1) − 2c(i+1)x(i+1) − 3d(i+1)x^2(i+1) = 0
            //Matches derivatives of the 2 cubic adjacent cubic functions (velocity)
            // mat(matIndex, matOffset + 1) = 1;
            // mat(matIndex, matOffset + 2) = 2 * x1;
            // mat(matIndex, matOffset + 3) = 3 * x1 * x1;
            // mat(matIndex, secondOffset + 1) = -1;
            // mat(matIndex, secondOffset + 2) = -2 * x1;
            // mat(matIndex, secondOffset + 3) = -3 * x1 * x1;
            // y(matIndex) = 0;

            //Match slopes paramaterized: bi + 2ci + 3di - ( (x(i+1) - xi) / (x(i+2) - x(i+1) )b(i+1) = 0
            mat(matIndex, matOffset + 1) = 1;
            mat(matIndex, matOffset + 2) = 2;
            mat(matIndex, matOffset + 3) = 3;
            mat(matIndex, secondOffset + 1) = -(realX1 - realX0)/(realX2-realX1);
            y(matIndex) = 0;
            matIndex++;

            //Match curvature: 2c(i) + 6d(i)x(i+1) − 2c(i+1) − 6d(i+1)x(i+1) = 0
            //Matches 2nd derivatives of the 2 cubic adjacent cubic functions (acceleration)
            // mat(matIndex, matOffset + 2) = 2;
            // mat(matIndex, matOffset + 3) = 6 * x1;
            // mat(matIndex, secondOffset + 2) = -2;
            // mat(matIndex, secondOffset + 3) = -6 * x1;
            // y(matIndex) = 0;

            //Match curvature paramaterized: 2ci + 6di - 2( (x(i+1) - xi)^2 / (x(i+2) - x(i+1))^2 )c(i+1) = 0
            mat(matIndex, matOffset + 2) = 2;
            mat(matIndex, matOffset + 3) = 6;
            mat(matIndex, secondOffset + 2) = -2 * ( pow(realX1 - realX0, 2) / pow(realX2 - realX1, 2) );
            y(matIndex) = 0;
            matIndex++;
        }
        else {
            //End slope: b + cx + 2dx^2 = s
            // mat(matIndex, matOffset + 1) = 1;
            // mat(matIndex, matOffset + 2) = x1;
            // mat(matIndex, matOffset + 3) = 2 * x1 * x1;
            // y(matIndex) = endSlope;

            //End slope paramaterized: b + 2c + 3d = (x1 - x0)s
            mat(matIndex, matOffset + 1) = 1;
            mat(matIndex, matOffset + 2) = 2;
            mat(matIndex, matOffset + 3) = 3;
            y(matIndex) = (realX1 - realX0) * endSlope;
            matIndex++;
        }
    }

    std::vector<CubicSplineSegment> allSegments;

    VectorXd coefficients = mat.inverse() * y;
    for(int i = 0; i < points.size() - 1; i++) {
        Vector4d v = coefficients(seq(i*4, (i*4)+3));
        CubicSplineSegment c(v);
        c.parameterOffset = points[i].x;
        c.outputOffset = points[i].y;
        c.parameterMultiplier = points[i + 1].x - points[i].x;
        allSegments.push_back(c);
    }

    return allSegments;
}