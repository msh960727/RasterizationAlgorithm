#pragma once

#include <glm/glm.hpp>
#include <iostream>
#include <vector>
#include "Texture.h"

namespace maengsh {

using namespace glm;
using namespace std;

// 변환
struct Transformation {
    vec3 scale = vec3(1.0f);
    vec3 translation = vec3(0.0f);
    float rotationX = 0.0f;
    float rotationY = 0.0f;
    float rotationZ = 0.0f;
};

// 재질
struct Material {
    vec3 ambient = vec3(0.1f);
    vec3 diffuse = vec3(1.0f);
    vec3 specular = vec3(1.0f);
    float shininess;

    std::shared_ptr<Texture> ambTexture;
    std::shared_ptr<Texture> difTexture;
};


struct Light {
    vec3 strength = vec3(1.0f);
    vec3 direction = vec3(0.0f, -1.0f, 0.0f); // directional/spot light only
    vec3 position = vec3(0.0f, 1.0f, 0.5f);   // point/spot light only
    float fallOffStart = 0.0f;                // point/spot light only
    float fallOffEnd = 1.8f;                 // point/spot light only
    float spotPower = 0.0f;                   // spot light only
};

// 폴리곤 메쉬를 다루는 클래스
class Mesh {
  public:
    void InitBox();
    void InitTriangle(const vec3 &v0, const vec3 &v1, const vec3 &v2,
                      const vec3 &normal);
    void InitSquare(const vec3 &v0, const vec3 &v1, const vec3 &v2, const vec3 &v3, 
                    const vec3 &normal);
    void CopyToBuffer();

  public:
    // CPU 메모리의 기하 정보
    vector<vec3> vertices;
    vector<size_t> indices;
    vector<vec3> normals;
    vector<vec3> colors;// 색을 Shading으로 결정
    vector<vec2> textureCoords; // Texture Coordinates
    

    // GPU 버퍼라고 가정.
    vector<vec3> vertexBuffer;
    vector<vec3> normalBuffer;
    vector<size_t> indexBuffer;
    vector<vec3> colorBuffer;
    vector<vec2> uvBuffer;


    // 모든 버텍스에 공통으로 적용되는 변환(Transformations)
    Transformation transformation;

    // 재질(Material)
    Material material;
};
} 