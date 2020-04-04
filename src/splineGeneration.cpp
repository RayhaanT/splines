#include "splines.h"
#include "Eigen/Core"
#include <iostream>
#include <algorithm>

using namespace Eigen;

//Extern definitions
std::vector<glm::vec2> controlPoints;
std::vector<glm::vec2> controlSlopes;
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

float safeDivision(float numerator, float denominator) {
    if(denominator == 0) {
        return 0;
    }
    else {
        return numerator / denominator;
    }
}

std::vector<std::vector<CubicSplineSegment>> calculateFreeSpaceCubicHermite(std::vector<glm::vec2> points, std::vector<glm::vec2> slopes) {
    if(slopes.size() < 2) {
        return {std::vector<CubicSplineSegment>(), std::vector<CubicSplineSegment>()};
    } 
    
    std::vector<glm::vec2> xPoints;
    std::vector<glm::vec2> yPoints;
    for (int i = 0; i < slopes.size(); i++) {
        xPoints.push_back(glm::vec2(i, points[i].x));
        yPoints.push_back(glm::vec2(i, points[i].y));
    }

    std::vector<float> xSlopes;
    std::vector<float> ySlopes;
    for(int i = 0; i < slopes.size() - 1; i++) {
        float xDerivative = abs(safeDivision(1.0f, points[i + 1].x - points[i].x));
        float yDerivative = abs(safeDivision(1.0f, points[i + 1].y - points[i].y));

        xSlopes.push_back(xDerivative * slopes[i].x);
        xSlopes.push_back(xDerivative * slopes[i + 1].x);
        ySlopes.push_back(yDerivative * slopes[i].y);
        ySlopes.push_back(yDerivative * slopes[i + 1].y);
    }

    for(int i = 1; i < xSlopes.size() - 2; i+=2) {
        glm::vec2 v1 = glm::normalize(glm::vec2(xSlopes[i], ySlopes[i]));
        glm::vec2 v2 = glm::normalize(glm::vec2(xSlopes[i + 1], ySlopes[i + 1]));
        std::cout << "1: " << v1.x << " " <<v1.y << " 2: " << v2.x << " " << v2.y << std::endl;
    }

    std::vector<CubicSplineSegment> xSpline;
    std::vector<CubicSplineSegment> ySpline;

    xSpline = calculateCubicHermite(xPoints, xSlopes);
    ySpline = calculateCubicHermite(yPoints, ySlopes);

    return {xSpline, ySpline};
}

std::vector<CubicSplineSegment> calculateCubicHermite(std::vector<glm::vec2> points, std::vector<float> slopes) {
    Matrix4d mat;
    Vector4d y;

    for(int a = 0; a < 4; a++) {
        for(int b = 0; b < 4; b++) {
            mat(a, b) = 0;
        }
    }

    //X is always 0 for first point and 1 for second due to paramaterization
    //Complies with first point slope: b + 2cx + 3dx^2 = s (x = 0)
    mat(0, 1) = 1;

    //Pass point through point 0: a + bx + cx^2 + dx^3 = y (x = 0)
    mat(1, 0) = 1;

    //Pass point through point 1: a + bx + cx^2 + dx^3 = y (x = 1)
    mat(2, 0) = 1;
    mat(2, 1) = 1;
    mat(2, 2) = 1;
    mat(2, 3) = 1;

    //Complies with second point slope: b + 2cx + 3dx^2 = s (x = 1)
    mat(3, 1) = 1;
    mat(3, 2) = 2;
    mat(3, 3) = 3;

    Matrix4d invMat = mat.inverse();

    std::vector<CubicSplineSegment> allSegments;

    for(int i = 0; i < points.size() - 1; i++) {
        //First slope
        y(0) = slopes[i * 2];
        //First point
        y(1) = points[i].y;
        //Second point
        y(2) = points[i + 1].y;
        //Second slope
        y(3) = slopes[i * 2 + 1];

        Vector4d coefficients = mat.inverse() * y;
        CubicSplineSegment c(coefficients);
        c.parameterOffset = points[i].x;
        c.outputOffset = points[i].y;
        c.parameterMultiplier = points[i + 1].x - points[i].x;
        allSegments.push_back(c);
    }

    return allSegments;
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
        float startXDerivative = abs(safeDivision(1.0f, (points[1].x - points[0].x)));
        float startYDerivative = abs(safeDivision(1.0f, (points[1].y - points[0].y)));
        
        float startXSlope = startXDerivative * startSlope.x;
        float startYSlope = startYDerivative * startSlope.y;

        int n = points.size() - 1;
        float endXDerivative = abs(safeDivision(1.0f, (points[n].x - points[n-1].x)));
        float endYDerivative = abs(safeDivision(1.0f, (points[n].y - points[n-1].y)));

        float endXSlope = endXDerivative * endSlope.x;
        float endYSlope = endYDerivative * endSlope.y;

        xSpline = calculateCubicStitched(xPoints, startXSlope, endXSlope, false);
        ySpline = calculateCubicStitched(yPoints, startYSlope, endYSlope, false);
    }
    else {
        xSpline = calculateCubicStitched(xPoints, 1, 1, false);
        ySpline = calculateCubicStitched(yPoints, 1, 1, false);
    }

    return {xSpline, ySpline};
}

// Non-paramaterized, non-localized
std::vector<CubicSplineSegment> calculateCubicStitched(std::vector<glm::vec2> points, float startSlope, float endSlope, bool linear) {
    if(linear) {
        std::sort(points.begin(), points.end(), xValueSort); 
    }
    int numVar = (points.size()-1)*4;
    MatrixXd mat(numVar, numVar);
    VectorXd y(numVar);
    int matIndex = 0;

    for(int a = 0; a < numVar; a++) {
        for(int b = 0; b < numVar; b++) {
            mat(a, b) = 0;
        }
    }

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