#include "object2D.h"

#include <vector>

#include "core/engine.h"
#include "utils/gl_utils.h"
#include <cmath>

Mesh* object2D::CreateSquare(
    const std::string& name,
    glm::vec3 leftBottomCorner,
    float length,
    glm::vec3 color,
    bool fill)
{
    glm::vec3 corner = leftBottomCorner;

    std::vector<VertexFormat> vertices =
    {
        VertexFormat(corner, color),
        VertexFormat(corner + glm::vec3(length, 0, 0), color),
        VertexFormat(corner + glm::vec3(length, length, 0), color),
        VertexFormat(corner + glm::vec3(0, length, 0), color)
    };

    Mesh* square = new Mesh(name);
    std::vector<unsigned int> indices = { 0, 1, 2, 3 };

    if (!fill) {
        square->SetDrawMode(GL_LINE_LOOP);
    }
    else {
        // Draw 2 triangles. Add the remaining 2 indices
        indices.push_back(0);
        indices.push_back(2);
    }

    square->InitFromData(vertices, indices);
    return square;
}

Mesh* object2D::CreateTrapezoid(const std::string& name, glm::vec3 color) {
    std::vector<VertexFormat> vertices = {
        VertexFormat(glm::vec3(-30, 0, 0), color),
        VertexFormat(glm::vec3(30, 0, 0), color),
        VertexFormat(glm::vec3(20, 20, 0), color),
        VertexFormat(glm::vec3(-20, 20, 0), color)    
    };

    std::vector<unsigned int> indices = { 0, 1, 2, 3, 0, 2};

    Mesh* trapezoid = new Mesh(name);
    trapezoid->InitFromData(vertices, indices);
    return trapezoid;
}

Mesh* object2D::CreateCircle(const std::string& name, glm::vec3 color, float radius, int numSegments) {
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    vertices.push_back(VertexFormat(glm::vec3(0, 0, 0), color));

    for (int i = 0; i <= numSegments; i++) {
        float theta = 2.0f * M_PI * float(i) / float(numSegments);
        float x = radius * cos(theta);
        float y = radius * sin(theta);
        vertices.push_back(VertexFormat(glm::vec3(x, y, 0), color));

        if (i > 0) {
            indices.push_back(0);
            indices.push_back(i);
            indices.push_back(i + 1);
        }
    }

    Mesh* circle = new Mesh(name);
    circle->InitFromData(vertices, indices);
    circle->SetDrawMode(GL_TRIANGLES);
    return circle;
}

Mesh* object2D::CreateRectangle(const std::string& name, glm::vec3 color) {
    std::vector<VertexFormat> vertices = {
        VertexFormat(glm::vec3(0, 0, 0), color),
        VertexFormat(glm::vec3(35, 0, 0), color),
        VertexFormat(glm::vec3(35, 5, 0), color),
        VertexFormat(glm::vec3(0, 5, 0), color)
    };

	std::vector<unsigned int> indices = { 0, 1, 2, 3, 0, 2 };

    Mesh* rectangle = new Mesh(name);
    rectangle->InitFromData(vertices, indices);
    return rectangle;
}