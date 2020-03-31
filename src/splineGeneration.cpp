#include "splines.h"

std::vector<glm::vec2> controlPoints;
Eigen::Vector4d coefficients;

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

    coefficients = mat.inverse() * y;
}
