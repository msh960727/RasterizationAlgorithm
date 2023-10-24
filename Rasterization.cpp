#include "Rasterization.h"
#include <algorithm> 

#include "MyShader.h"

namespace maengsh {

using namespace glm;
using namespace std;

Rasterization::Rasterization(const int &width, const int &height)
    : width(width), height(height) {

    // 조명 설정
    light.strength = vec3(1.0f);
    light.direction = vec3(0.0f, -1.0f, 0.0f);

    auto imageTexture = std::make_shared<Texture>("TextureImage4.jpg");

    this->object = make_shared<Mesh>();
    this->object->InitBox();
    this->object->material.ambTexture = imageTexture;
    this->object->material.difTexture = imageTexture;
    
    this->object->transformation.rotationX = -3.141592f * 60.0f / 180.0f;
    this->object->transformation.rotationY = 0.0f;
    this->object->transformation.scale = vec3(1.0f, 1.0f, 1.0f);
    this->object->transformation.translation = vec3(0.5f, -0.8f, 4.0f);
    this->object->material.ambient = vec3(0.25f);
    this->object->material.diffuse = vec3(0.5f);
    this->object->material.specular = vec3(0.5f);
    this->object->CopyToBuffer();
    meshes.push_back(this->object);

    // 깊이버퍼 초기화
    depthBuffer.resize(width * height, 0.0f);
}

// 3차원 좌표를 2차원 좌표로 변환
vec2 Rasterization::ProjectWorldToRaster(vec3 pointWorld) {

    // 정투영(Orthographic projection)
    vec2 pointProj = vec2(pointWorld.x, pointWorld.y);

    // 원근투영(Perspective projection)
    if (this->usePerspectiveProjection) {
        const float scale =
            distEyeToScreen / (this->distEyeToScreen + pointWorld.z);
        pointProj = vec2(pointWorld.x * scale, pointWorld.y * scale);
    }

    const float aspect = float(width) / height;
    const vec2 pointNDC = vec2(pointProj.x / aspect, pointProj.y);

    // 레스터 좌표의 범위 [-0.5, width - 1 + 0.5] x [-0.5, height - 1 + 0.5]
    const float xScale = 2.0f / width;
    const float yScale = 2.0f / height;

    // NDC -> 레스터 화면 좌표계
    return vec2((pointNDC.x + 1.0f) / xScale - 0.5f,
                (1.0f - pointNDC.y) / yScale - 0.5f);
}

// 외적으로 삼각형의 넓이 계산(뒷면일 경우 음수)
float Rasterization::EdgeFunction(const vec2 &v0, const vec2 &v1,
                                  const vec2 &point) {
    const vec2 a = v1 - v0;
    const vec2 b = point - v0;
    return a.x * b.y - a.y * b.x;
}

// Object를 이루는 삼각형을 그리기
void Rasterization::DrawIndexedTriangle(const size_t &startIndex,
                                        vector<vec4> &pixels) {

    // index값을 순서대로 3개씩 받아 기록
    const size_t i0 = this->indexBuffer[startIndex];
    const size_t i1 = this->indexBuffer[startIndex + 1];
    const size_t i2 = this->indexBuffer[startIndex + 2];

    // index가 가르키는 vertex값을 저장
    vec3 prv0 = this->vertexBuffer[i0];
    vec3 prv1 = this->vertexBuffer[i1];
    vec3 prv2 = this->vertexBuffer[i2];

    // index가 가르키는 uv값을 저장
    vec2 pruv0 = this->uvBuffer[i0];
    vec2 pruv1 = this->uvBuffer[i1];
    vec2 pruv2 = this->uvBuffer[i2];

    // 삼각형이 평면에 의해 잘리는지 판단 및 잘린삼각형으로 vertex, uv값 변경
    if (IsClipTriangle(vec3(i0, i1, i2), constants.plane))
        return;
        
    // 2차원 좌표로 변환
    const auto v0 = ProjectWorldToRaster(this->vertexBuffer[i0]);
    const auto v1 = ProjectWorldToRaster(this->vertexBuffer[i1]);
    const auto v2 = ProjectWorldToRaster(this->vertexBuffer[i2]);


    // 삼각형 전체 넓이의 두 배(뒷면일 경우 음수)
    const float area = EdgeFunction(v0, v1, v2);

    // 뒷면일 경우 그리지 않음
    // Clipping구현 디버깅을 위해 주석처리함
    //if (this->cullBackface && area < 0.0f)
    //   return;

    // const auto &c0 = this->colorBuffer[i0];
    // const auto &c1 = this->colorBuffer[i1];
    // const auto &c2 = this->colorBuffer[i2];

    const auto &uv0 = this->uvBuffer[i0];
    const auto &uv1 = this->uvBuffer[i1];
    const auto &uv2 = this->uvBuffer[i2];

    // 삼각형을 포함하는 부분만 계산하기 위해
    // for 문의 범위를 3개의 vertex의 x, y값들 중 각각 최대 최솟값을 구함
    const auto xMin = size_t(glm::clamp(
        glm::floor(std::min({v0.x, v1.x, v2.x})), 0.0f, float(width - 1)));
    const auto yMin = size_t(glm::clamp(
        glm::floor(std::min({v0.y, v1.y, v2.y})), 0.0f, float(height - 1)));
    const auto xMax = size_t(glm::clamp(glm::ceil(std::max({v0.x, v1.x, v2.x})),
                                        0.0f, float(width - 1)));
    const auto yMax = size_t(glm::clamp(glm::ceil(std::max({v0.y, v1.y, v2.y})),
                                        0.0f, float(height - 1)));

        for (size_t j = yMin; j <= yMax; j++) {
            for (size_t i = xMin; i <= xMax; i++) {

                const vec2 point = vec2(float(i), float(j));

                // 위에서 계산한 삼각형 전체 넓이 area를 재사용
                // area가 음수라면 alpha0, alpha1, alpha2 모두 음수여야
                // 삼각형 안에 포함되는 픽셀로 판단.
                float w0 = EdgeFunction(v1, v2, point) / area;
                float w1 = EdgeFunction(v2, v0, point) / area;
                float w2 = EdgeFunction(v0, v1, point) / area;

                if (w0 >= 0.0f && w1 >= 0.0f && w2 >= 0.0f) {
                    
                    // 삼각형의 왜곡 방지를 위해 Perspective-Correct Interpolation으로
                    // 픽셀에 적용되는 값들을 정의
                    // Perspective-Correct Interpolation
                    // 논문
                    // https://www.comp.nus.edu.sg/~lowkl/publications/lowk_persp_interp_techrep.pdf
                    // 해설글
                    // https://www.scratchapixel.com/lessons/3d-basic-rendering/rasterization-practical-implementation/perspective-correct-interpolation-vertex-attributes
                    // OpenGL 구현
                    // https://stackoverflow.com/questions/24441631/how-exactly-does-opengl-do-perspectively-correct-linear-interpolation

                    const float z0 = this->vertexBuffer[i0].z + distEyeToScreen;
                    const float z1 = this->vertexBuffer[i1].z + distEyeToScreen;
                    const float z2 = this->vertexBuffer[i2].z + distEyeToScreen;

                    const vec3 p0 = this->vertexBuffer[i0];
                    const vec3 p1 = this->vertexBuffer[i1];
                    const vec3 p2 = this->vertexBuffer[i2];

                    // 뒷면일 경우에도 쉐이딩이 가능하도록 normal을 반대로 뒤집어줌.
                    const vec3 n0 = area < 0.0f ? -this->normalBuffer[i0]
                                                : this->normalBuffer[i0];
                    const vec3 n1 = area < 0.0f ? -this->normalBuffer[i1]
                                                : this->normalBuffer[i1];
                    const vec3 n2 = area < 0.0f ? -this->normalBuffer[i2]
                                                : this->normalBuffer[i2];

                    if (this->usePerspectiveProjection &&
                        this->usePerspectiveCorrectInterpolation) {

                        w0 /= z0;
                        w1 /= z1;
                        w2 /= z2;

                        const float wSum = w0 + w1 + w2;

                        w0 /= wSum;
                        w1 /= wSum;
                        w2 /= wSum;
                    }

                    // 이하 동일
                    const float depth = w0 * z0 + w1 * z1 + w2 * z2;
                    // const vec3 color = w0 * c0 + w1 * c1 + w2 * c2;
                    const vec2 uv = w0 * uv0 + w1 * uv1 + w2 * uv2;

                    if (depth < depthBuffer[i + width * j]) {
                        depthBuffer[i + width * j] = depth;

                        PSInput psInput;
                        psInput.position = w0 * p0 + w1 * p1 + w2 * p2;
                        psInput.normal = w0 * n0 + w1 * n1 + w2 * n2;
                        // psInput.color = color;
                        psInput.uv = uv;

                        pixels[i + width * j] = MyPixelShader(psInput);
                    }
                }
            }
        }

        // Object에서 그릴 삼각형을 선택할 때, 중복되는 vertex가 있으므로
        // Clipping으로 인해 변경된 vertex값들을 기존값들로 초기화함
        this->vertexBuffer[i0] = prv0;
        this->vertexBuffer[i1] = prv1;
        this->vertexBuffer[i2] = prv2;

        this->uvBuffer[i0] = pruv0;
        this->uvBuffer[i1] = pruv1;
        this->uvBuffer[i2] = pruv2;

}



void Rasterization::Render(vector<vec4> &pixels) {

    // 깊이 버퍼 초기화
    this->depthBuffer.resize(pixels.size());

    fill(this->depthBuffer.begin(), this->depthBuffer.end(), 7.0f);

    constants.plane = vec4(this->Plane);

    for (const auto& mesh : this->meshes) {

        // 렌더링 하기 전에 필요한 데이터를 복사
        constants.transformation = mesh->transformation;
        constants.material = mesh->material;
        constants.light = light;
        constants.lightType = this->lightType;
        constants.BlinnPhone = this->BlinnPhong;

        this->vertexBuffer.resize(mesh->vertexBuffer.size());
        this->normalBuffer.resize(mesh->normalBuffer.size());
        //this->colorBuffer.resize(mesh->vertexBuffer.size());
        this->uvBuffer.resize(mesh->uvBuffer.size());

        for (size_t i = 0; i < mesh->vertexBuffer.size(); i++) {

            VSInput vsInput;
            vsInput.position = mesh->vertexBuffer[i];
            vsInput.normal = mesh->normalBuffer[i];
            //vsInput.color = mesh->colorBuffer[i];
            vsInput.uv = mesh->uvBuffer[i];
            
            // 들어온 값을 Transform
            auto vsOutput = MyVertexShader(vsInput);
            

            this->vertexBuffer[i] = vsOutput.position;
            this->normalBuffer[i] = vsOutput.normal;
            //this->colorBuffer[i] = vsOutput.color;
            this->uvBuffer[i] = vsOutput.uv;
        }
        
        // Clipping으로 인해 indexBuffer의 크기가 늘어날수 있으므로
        // 다음 도형을 그릴때 초기화 해줌.
        this->indexBuffer.resize(mesh->indexBuffer.size());
        this->indexBuffer = mesh->indexBuffer;

        // indexBuffer에 index값을 3개씩 넣어준다(=삼각형 1개씩)
        for (size_t i = 0; i < this->indexBuffer.size(); i += 3) {

            DrawIndexedTriangle(i, pixels);
        }
    }
}


bool Rasterization::IsClipTriangle(vec3 indices, vec4 plane) {
    
    // 삼각형이 잘리면 vertex값을 변형시키기 위해 vertex값에 대한 참조자 선언
    vec3 &v0 = this->vertexBuffer[indices.x];
    vec3 &v1 = this->vertexBuffer[indices.y];
    vec3 &v2 = this->vertexBuffer[indices.z];

    vec2 &uv0 = this->uvBuffer[indices.x];
    vec2 &uv1 = this->uvBuffer[indices.y];
    vec2 &uv2 = this->uvBuffer[indices.z];

    // 평면과 vertex값 사이의 거리
    float d0 = SignedDistance(plane, this->vertexBuffer[indices.x]);
    float d1 = SignedDistance(plane, this->vertexBuffer[indices.y]);
    float d2 = SignedDistance(plane, this->vertexBuffer[indices.z]);

    float t;


    if (d0 >= 0 && d1 >= 0 && d2 >= 0)
        return false;

    else if (d0 <= 0 && d1 <= 0 && d2 <= 0)
        return true;

    else if (d0 > 0 && d1 <= 0 && d2 <= 0) {

        
        t = SolvingForT(v0, v1, plane);
        v1 = Intersection(v0, v1, t);
        uv1 = Intersection(uv0, uv1, t);

        t = SolvingForT(v0, v2, plane);
        v2 = Intersection(v0, v2, t);        
        uv2 = Intersection(uv0, uv2, t);

        return false;
    }

    else if (d1 > 0 && d0 <= 0 && d2 <= 0) {

        
        t = SolvingForT(v1, v0, plane);
        v0 = Intersection(v1, v0, t);
        uv0 = Intersection(uv1, uv0, t);

        t = SolvingForT(v1, v2, plane);
        v2 = Intersection(v1, v2, t);
        uv2 = Intersection(uv1, uv2, t);

        return false;
    }

    else if (d2 > 0 && d1 <= 0 && d0 <= 0) {

        t = SolvingForT(v2, v1, plane);
        v1 = Intersection(v2, v1, t);
        uv1 = Intersection(uv2, uv1, t);

        t = SolvingForT(v2, v0, plane);
        v0 = Intersection(v2, v0, t);
        uv0 = Intersection(uv2, uv0, t);

        return false;
    }

    else if (d0 < 0 && d1 > 0 && d2 > 0) {

        {
            t = SolvingForT(v2, v0, plane);
            v3 = Intersection(v2, v0, t);
            uv3 = Intersection(uv2, uv0, t);
            

            t = SolvingForT(v1, v0, plane);
            v0 = Intersection(v1, v0, t);
            uv0 = Intersection(uv1, uv0, t);

            this->vertexBuffer.push_back(v0);
            this->uvBuffer.push_back(uv0);
            this->vertexBuffer.push_back(v3);
            this->uvBuffer.push_back(uv3);

            this->normalBuffer.push_back(this->normalBuffer[indices.x]);
            this->normalBuffer.push_back(this->normalBuffer[indices.x]);


            this->indexBuffer.push_back(this->vertexBuffer.size()-2);
            this->indexBuffer.push_back(indices.z);
            this->indexBuffer.push_back(this->vertexBuffer.size()-1);
            

            
            return false;
        }
        
    }

    else if (d0 > 0 && d1 < 0 && d2 > 0) {

        {

            t = SolvingForT(v0, v1, plane);
            v3 = Intersection(v0, v1, t);
            uv3 = Intersection(uv0, uv1, t);

            t = SolvingForT(v2, v1, plane);
            v1 = Intersection(v2, v1, t);
            uv1 = Intersection(uv2, uv1, t);


            this->vertexBuffer.push_back(v3);
            this->uvBuffer.push_back(uv3);
            this->vertexBuffer.push_back(v1);
            this->uvBuffer.push_back(uv1);

            this->normalBuffer.push_back(this->normalBuffer[indices.x]);
            this->normalBuffer.push_back(this->normalBuffer[indices.x]);

            this->indexBuffer.push_back(indices.x);
            this->indexBuffer.push_back(this->vertexBuffer.size() - 2);
            this->indexBuffer.push_back(this->vertexBuffer.size() - 1);


            return false;

        } 
        
    }

    else if (d0 > 0 && d1 > 0 && d2 < 0) {

        {

            t = SolvingForT(v0, v2, plane);
            v3 = Intersection(v0, v2, t);
            uv3 = Intersection(uv0, uv2, t);

            t = SolvingForT(v1, v2, plane);
            v2 = Intersection(v1, v2, t);
            uv2 = Intersection(uv1, uv2, t);

            this->vertexBuffer.push_back(v2);
            this->uvBuffer.push_back(uv2);
            this->vertexBuffer.push_back(v3);
            this->uvBuffer.push_back(uv3);

            this->normalBuffer.push_back(this->normalBuffer[indices.x]);
            this->normalBuffer.push_back(this->normalBuffer[indices.x]);

            this->indexBuffer.push_back(indices.x);
            this->indexBuffer.push_back(this->vertexBuffer.size() - 2);
            this->indexBuffer.push_back(this->vertexBuffer.size()-1);
            
            return false;
        } 
    }
    else
        return false;

}


float Rasterization::SignedDistance(vec4 plane, vec3 vertex){

    float d = (vertex.x * plane.x) + (vertex.y * plane.y) +
              (vertex.z * plane.z) + plane.w;

    if (d > -1.0e-5 && d < 1.0e-5)
        return 0.0f;
    else
        return d;
}

float Rasterization::SolvingForT(vec3 pos, vec3 neg, vec4 plane) {

    vec3 normal = {plane.x, plane.y, plane.z};
    float t = (-plane.w - dot(pos, normal)) / (dot(normal, neg - pos));
    return t;
}

template <typename T>
T Rasterization::Intersection(T pos, T neg, float t) {

    return pos + t * (neg - pos);
}

void Rasterization::Update() {

}

}