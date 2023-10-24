#pragma once

#include <functional>
#include <glm/glm.hpp>
#include <iostream>
#include <memory>
#include <vector>

#include "Mesh.h"

namespace maengsh {

class Rasterization {
  public:
    Rasterization(const int &width, const int &height);

    // 삼각형을 하나만 그리는 함수 추가
    // Render(.)에서 호출
    void DrawIndexedTriangle(const size_t &startIndex, vector<vec4> &pixels);

    vec2 ProjectWorldToRaster(vec3 point);
    float EdgeFunction(const vec2 &v0, const vec2 &v1, const vec2 &point);
    void Render(vector<vec4> &pixels);
    void Update();

    // Clipping 구현
    bool IsClipTriangle(vec3 indices, vec4 plane);
    float SignedDistance(vec4 plane, vec3 vertex);
    float SolvingForT(vec3 pos, vec3 neg, vec4 plane);

    template <typename T>
    T Intersection(T pos, T neg, float t);


  public:
    int width;
    int height;

    vector<shared_ptr<Mesh>> meshes;  

    shared_ptr<Mesh> object;    

    Light light;

    // GPU에서 내부적으로 사용하는 메모리 버퍼라고 가정.
    vector<vec3> vertexBuffer;
    vector<vec3> normalBuffer;
    vector<size_t> indexBuffer;
    vector<vec3> colorBuffer;
    vector<vec2> uvBuffer;

    // 깊이(화면으로부터의 거리)를 저장하는 버퍼
    vector<float> depthBuffer;


    // 삼각형 뒷면 그릴지 여부
    bool cullBackface = true;

    // 정투영(ortho) vs 원근(perspective)투영
    bool usePerspectiveProjection = true;
    bool usePerspectiveCorrectInterpolation = true;

    bool BlinnPhong = true;

    // 눈과 화면의 거리
    float distEyeToScreen = 1.0f;

    // 삼각형을 Clipping할 평면
    vec4 Plane = vec4(0.0f, 0.0f, 1.0f, -1.0f);

	// 현재 사용하는 조명 (0: directional, 1: point, 2: spot)
	int lightType = 0;
    vec3 v3;
    vec2 uv3;
};
}
