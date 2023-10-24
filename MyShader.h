#pragma once

#include "Mesh.h" // struct Material, Light


namespace maengsh {
using namespace glm;


// https://en.wikipedia.org/wiki/Rotation_matrix
vec3 RotateAboutZ(const vec3 &v, const float &theta) {
    return vec3(v.x * cos(theta) - v.y * sin(theta),
                v.x * sin(theta) + v.y * cos(theta), v.z);
}

vec3 RotateAboutY(const vec3 &v, const float &theta) {
    return vec3(v.x * cos(theta) + v.z * sin(theta), v.y,
                -v.x * sin(theta) + v.z * cos(theta));
}

vec3 RotateAboutX(const vec3 &v, const float &theta) {
    return vec3(v.x, v.y * cos(theta) - v.z * sin(theta),
                v.y * sin(theta) + v.z * cos(theta));
}

// 쉐이더에서 공통적으로 사용하는 상수들
struct Constants {
    Transformation transformation;
    Material material;
    Light light;
    int lightType = 0;
    bool BlinnPhone = true;
    vec4 plane;
} constants;




// 조명효과 구현

// BlinnPhong
vec3 BlinnPhong(vec3 lightStrength, vec3 lightVec, vec3 normal, vec2 uv,
                vec3 toEye,
                Material mat) {

    vec3 halfway = normalize(toEye + lightVec);
    vec3 specular =
        mat.specular * pow(glm::max(dot(halfway, normal), 0.0f), mat.shininess);

    vec3 phongColor(0.0f);

    if (mat.ambTexture)
        phongColor += mat.ambient * mat.ambTexture->SampleLinear(uv);
    else
        phongColor += mat.ambient;

    if (mat.difTexture)
        phongColor +=
            mat.diffuse * mat.difTexture->SampleLinear(uv) * lightStrength;
    else
        phongColor += mat.diffuse * lightStrength;

    return phongColor + specular * lightStrength;
}

// Phong
vec3 Phong(vec3 lightStrength, vec3 lightVec, vec3 normal, vec2 uv, vec3 toEye,
           Material mat) {

    vec3 reflectDir = normalize(normal*2.0f*dot(lightVec, normal) - lightVec);
    vec3 specular =
        mat.specular * pow(glm::max(dot(reflectDir, toEye), 0.0f), mat.shininess);
    vec3 phongColor(0.0f);

    if (mat.ambTexture)
        phongColor += mat.ambient * mat.ambTexture->SampleLinear(uv);
    else
        phongColor += mat.ambient;

    if (mat.difTexture)
        phongColor += mat.diffuse * mat.difTexture->SampleLinear(uv) * lightStrength;
    else
        phongColor += mat.diffuse * lightStrength;




    return phongColor + specular;
}

vec3 ComputeDirectionalLight(Light L, Material mat, vec3 normal, vec2 uv,
                             vec3 toEye) {
    vec3 lightVec = -L.direction;

    float ndotl = glm::max(dot(normalize(lightVec), normal), 0.0f);
    vec3 lightStrength = L.strength * ndotl;


    if (constants.BlinnPhone)
        return BlinnPhong(lightStrength, lightVec, normal, uv, toEye, mat);

    return Phong(lightStrength, lightVec, normal, uv, toEye, mat);
}


float Saturate(float x) { return glm::max(0.0f, glm::min(1.0f, x)); }

float CalcAttenuation(float d, float falloffStart, float falloffEnd) {

    return Saturate((falloffEnd - d) / (falloffEnd - falloffStart));
}

vec3 ComputePointLight(Light L, Material mat, vec3 pos, vec3 normal, vec2 uv,
                       vec3 toEye) {
    vec3 lightVec = L.position - pos;

    // 쉐이딩할 지점부터 조명까지의 거리 계산
    float d = length(lightVec);

    // 너무 멀면 조명이 적용되지 않음
    if (d > L.fallOffEnd)
        return vec3(0.0f);

    lightVec /= d;

    float ndotl = glm::max(dot(lightVec, normal), 0.0f);
    vec3 lightStrength = L.strength * ndotl;

    float att = CalcAttenuation(d, L.fallOffStart, L.fallOffEnd);
    lightStrength *= att;

    if (constants.BlinnPhone)
        return BlinnPhong(lightStrength, lightVec, normal, uv, toEye, mat);

    return Phong(lightStrength, lightVec, normal, uv, toEye, mat);

    
}

vec3 ComputeSpotLight(Light L, Material mat, vec3 pos, vec3 normal, vec2 uv,
                      vec3 toEye) {
    vec3 lightVec = L.position - pos;

    float d = length(lightVec);

    if (d > L.fallOffEnd)
        return vec3(0.0f);

    lightVec /= d;

    float ndotl = glm::max(dot(lightVec, normal), 0.0f);
    vec3 lightStrength = L.strength * ndotl;

    float att = CalcAttenuation(d, L.fallOffStart, L.fallOffEnd);
    lightStrength *= att;

    float spotFactor = glm::pow(glm::max(dot(-lightVec, L.direction), 0.0f), L.spotPower);
    lightStrength *= spotFactor;

    if (constants.BlinnPhone)
        return BlinnPhong(lightStrength, lightVec, normal, uv, toEye, mat);

    return Phong(lightStrength, lightVec, normal, uv, toEye, mat);
}

// 버텍스 쉐이더 (Vertex shader)
struct VSInput {
    vec3 position;
    vec3 normal;
    vec3 color;
    vec2 uv;
};

struct VSOutput {
    vec3 position;
    vec3 normal;
    vec3 color;
    vec2 uv;
};

VSOutput MyVertexShader(const VSInput vsInput) {
    VSOutput vsOutput;

    vsOutput.position =
        RotateAboutX(
            RotateAboutY(vsInput.position * constants.transformation.scale,
                         constants.transformation.rotationY),
            constants.transformation.rotationX) +
        constants.transformation.translation;

    vsOutput.normal =
        RotateAboutX(
            RotateAboutY(vsInput.normal, constants.transformation.rotationY),
            constants.transformation.rotationX);
    //vsOutput.color = vsInput.color;
    vsOutput.uv = vsInput.uv;

    return vsOutput;
}

// 픽셀 쉐이더 (Pixel shader)
struct PSInput {
    vec3 position;
    vec3 normal;
    vec3 color;
    vec2 uv;
};



vec4 MyPixelShader(const PSInput psInput) {    

    vec3 eye = vec3(0.0f, 0.0f, -1.0f); // -distEyeToScreen
    vec3 toEye = glm::normalize(eye - psInput.position);


    vec3 color = vec3(0.0f);    

    if (constants.lightType == 0) {
        color += ComputeDirectionalLight(constants.light, constants.material,
                                        psInput.normal, psInput.uv, toEye);
    } else if (constants.lightType == 1) {
        color += ComputePointLight(constants.light, constants.material,
                                   psInput.position, psInput.normal, psInput.uv,
                                   toEye);
    } else {
        color += ComputeSpotLight(constants.light, constants.material,
                                  psInput.position, psInput.normal, psInput.uv,
                                  toEye);
    }


    return vec4(color, 1.0f);
}

// 여기까지 쉐이더 정의
}
