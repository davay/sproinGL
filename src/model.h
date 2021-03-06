#ifndef MODEL_H
#define MODEL_H

#include "GLXtras.h"
#include "Mesh.h"
#include "VecMat.h"

#include <iostream>
#include <stdio.h>
#include <vector>

class Model {
public:
    Model(vec3 color) {
        this->color = color;
    };

    bool read(const char *meshName) {
        if (!ReadAsciiObj(meshName, points, triangles, &normals, &uvs)) {
            printf("can't read %s\n", meshName);
            return false;
        }

        buffer();
        return true;
    }

    void buffer() {
        int pointsSize = points.size() * sizeof(vec3);
        int normalsSize = normals.size() * sizeof(vec3);
        int bufferSize = pointsSize + normalsSize;

        // Generate buffers
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);

        // Bind vertex array object
        glBindVertexArray(vao);

        // Bind and set vertex buffer data
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, bufferSize, NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, bufferSize, &points[0]);
        glBufferSubData(GL_ARRAY_BUFFER, pointsSize, normalsSize, &normals[0]);

        // Set vertex attributes
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*) pointsSize);
        glEnableVertexAttribArray(1);

        // Bind and set element buffer data
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangles.size() * sizeof(vec3), &triangles[0], GL_STATIC_DRAW);

        glBindVertexArray(0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    void draw(int shader) {
        int pointsSize = points.size() * sizeof(vec3);

        // Use model's vertex array object
        glBindVertexArray(vao);

        // Use the model's model-to-world transform
        SetUniform(shader, "modelTrans", xform);
        SetUniform(shader, "modelColor", color);

        // Draw triangles
        glDrawElements(GL_TRIANGLES, 3 * triangles.size(), GL_UNSIGNED_INT, 0);

        glBindVertexArray(0);
    }

    void setXform(mat4 xform) { this->xform = xform; }
    void setColor(vec3 color) { this->color = color; }

private:
    vector<vec3> points;
    vector<vec3> normals;
    vector<vec2> uvs;
    vector<int3> triangles;
    unsigned int vbo, vao, ebo;
    mat4 xform;
    vec3 color;
};

#endif
