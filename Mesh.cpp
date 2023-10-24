#include "Mesh.h"

namespace maengsh {


void Mesh::InitSquare(const vec3 &v0, const vec3 &v1, const vec3 &v2,
                      const vec3 &v3, const vec3 &normal) {

    const size_t numTriangles = 2;
    this->vertices.reserve(numTriangles + 1);

    const auto scale = 0.7f;
    this->vertices.push_back(v0 * scale);
    this->vertices.push_back(v1 * scale);
    this->vertices.push_back(v2 * scale);
    this->vertices.push_back(v3 * scale);
    this->normals.push_back(normal);
    this->normals.push_back(normal);
    this->normals.push_back(normal);
    this->normals.push_back(normal);

    this->indices = {0, 1, 2, 0, 2, 3};

    this->textureCoords = {
        {0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}};

    
}

void Mesh::InitBox() {
    const float scale = 0.7f;

    // ����
    this->vertices.push_back(vec3(-1.0f, 1.0f, -1.0f) * scale);
    this->vertices.push_back(vec3(-1.0f, 1.0f, 1.0f) * scale);
    this->vertices.push_back(vec3(1.0f, 1.0f, 1.0f) * scale);
    this->vertices.push_back(vec3(1.0f, 1.0f, -1.0f) * scale);
    this->normals.push_back(vec3(0.0f, 1.0f, 0.0f));
    this->normals.push_back(vec3(0.0f, 1.0f, 0.0f));
    this->normals.push_back(vec3(0.0f, 1.0f, 0.0f));
    this->normals.push_back(vec3(0.0f, 1.0f, 0.0f));

    // �Ʒ���
    this->vertices.push_back(vec3(-1.0f, -1.0f, -1.0f) * scale);
    this->vertices.push_back(vec3(1.0f, -1.0f, -1.0f) * scale);
    this->vertices.push_back(vec3(1.0f, -1.0f, 1.0f) * scale);
    this->vertices.push_back(vec3(-1.0f, -1.0f, 1.0f) * scale);
    this->normals.push_back(vec3(0.0f, -1.0f, 0.0f));
    this->normals.push_back(vec3(0.0f, -1.0f, 0.0f));
    this->normals.push_back(vec3(0.0f, -1.0f, 0.0f));
    this->normals.push_back(vec3(0.0f, -1.0f, 0.0f));

    // �ո�
    this->vertices.push_back(vec3(-1.0f, -1.0f, -1.0f) * scale);
    this->vertices.push_back(vec3(-1.0f, 1.0f, -1.0f) * scale);
    this->vertices.push_back(vec3(1.0f, 1.0f, -1.0f) * scale);
    this->vertices.push_back(vec3(1.0f, -1.0f, -1.0f) * scale);
    this->normals.push_back(vec3(0.0f, 0.0f, -1.0f));
    this->normals.push_back(vec3(0.0f, 0.0f, -1.0f));
    this->normals.push_back(vec3(0.0f, 0.0f, -1.0f));
    this->normals.push_back(vec3(0.0f, 0.0f, -1.0f));

    // �޸�
    this->vertices.push_back(vec3(-1.0f, -1.0f, 1.0f) * scale);
    this->vertices.push_back(vec3(1.0f, -1.0f, 1.0f) * scale);
    this->vertices.push_back(vec3(1.0f, 1.0f, 1.0f) * scale);
    this->vertices.push_back(vec3(-1.0f, 1.0f, 1.0f) * scale);
    this->normals.push_back(vec3(0.0f, 0.0f, 1.0f));
    this->normals.push_back(vec3(0.0f, 0.0f, 1.0f));
    this->normals.push_back(vec3(0.0f, 0.0f, 1.0f));
    this->normals.push_back(vec3(0.0f, 0.0f, 1.0f));

    // ����
    this->vertices.push_back(vec3(-1.0f, -1.0f, 1.0f) * scale);
    this->vertices.push_back(vec3(-1.0f, 1.0f, 1.0f) * scale);
    this->vertices.push_back(vec3(-1.0f, 1.0f, -1.0f) * scale);
    this->vertices.push_back(vec3(-1.0f, -1.0f, -1.0f) * scale);
    this->normals.push_back(vec3(-1.0f, 0.0f, 0.0f));
    this->normals.push_back(vec3(-1.0f, 0.0f, 0.0f));
    this->normals.push_back(vec3(-1.0f, 0.0f, 0.0f));
    this->normals.push_back(vec3(-1.0f, 0.0f, 0.0f));

    // ������
    this->vertices.push_back(vec3(1.0f, -1.0f, 1.0f) * scale);
    this->vertices.push_back(vec3(1.0f, -1.0f, -1.0f) * scale);
    this->vertices.push_back(vec3(1.0f, 1.0f, -1.0f) * scale);
    this->vertices.push_back(vec3(1.0f, 1.0f, 1.0f) * scale);
    this->normals.push_back(vec3(1.0f, 0.0f, 0.0f));
    this->normals.push_back(vec3(1.0f, 0.0f, 0.0f));
    this->normals.push_back(vec3(1.0f, 0.0f, 0.0f));
    this->normals.push_back(vec3(1.0f, 0.0f, 0.0f));

    this->indices = {
        0,  1,  2,  0,  2,  3,  // ����
        4,  5,  6,  4,  6,  7,  // �Ʒ���
        8,  9,  10, 8,  10, 11, // �ո�
        12, 13, 14, 12, 14, 15, // �޸�
        16, 17, 18, 16, 18, 19, // ����
        20, 21, 22, 20, 22, 23  // ������
    };

    this->textureCoords = {
        {0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f},
        {0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f},
        {0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, 
        {0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, 
        {0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, 
        {0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}};
}

void Mesh::InitTriangle(const vec3 &v0, const vec3 &v1, const vec3 &v2,
                        const vec3 &normal) {
    const float scale = 0.7f;

    this->vertices.push_back(v0 * scale);
    this->vertices.push_back(v1 * scale);
    this->vertices.push_back(v2 * scale);
    this->normals.push_back(normal);
    this->normals.push_back(normal);
    this->normals.push_back(normal);

    this->indices = {0, 1, 2};

}

void Mesh::CopyToBuffer() {

    // CPU �޸𸮿��� GPU �޸𸮷� �����ϴ� ���̶�� ����
    vertexBuffer = vertices;
    indexBuffer = indices;
    normalBuffer = normals;
    colorBuffer = colors;
    uvBuffer = textureCoords;
     //textureImageBuffer = textureImage;

    // CPU �޸𸮴� ����
    vertices.clear();
    normals.clear();
    indices.clear();
    colors.clear();
    textureCoords.clear();
     
}
}
