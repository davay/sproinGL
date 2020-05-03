#include "GLXtras.h"
#include "Mesh.h"
#include "VecMat.h"

#include <iostream>
#include <stdio.h>

class Model {
public:
    vector<vec3> points;
    vector<vec3> normals;
    vector<vec2> uvs;
    vector<int3> triangles;
    mat4 xform;
    unsigned int vbo, vao, ebo;

    Model() { };

    bool read(const char *meshName, const char *textureName) {
        if (!ReadAsciiObj(meshName, points, triangles, &normals, &uvs)) {
            printf("can't read %s\n", meshName);
            return false;
        }

        buffer();
        return true;
    }

    void buffer() {
        int bufferSize = points.size() * sizeof(vec3);

        // Generate buffers
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);

        // Bind vertex array
        glBindVertexArray(vao);

        // Bind and set vertex buffer data
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, bufferSize, NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, bufferSize, &points[0]);

        // Bind and set element buffer data
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangles.size() * sizeof(vec3), &triangles[0], GL_STATIC_DRAW);
    }

    void draw(int shaderProgram) {
        glBindVertexArray(vao);
        VertexAttribPointer(shaderProgram, "point", 3, 0, (void *) 0);
        glDrawElements(GL_TRIANGLES, 3 * triangles.size(), GL_UNSIGNED_INT, 0);
    }
};
