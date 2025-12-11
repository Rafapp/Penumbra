#include "sampling.h"


Sampler::Sampler(uint32_t seed) {
    generator.seed(seed);
}

float Sampler::Sample1D() {
    std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
    return distribution(generator);
}

inline float Halton(uint32_t i, uint32_t base) {
    float f = 1.0f / base;
    float result = 0.0f;
    while (i > 0) {
        result += f * (i % base);
        i /= base;
        f /= base;
    }
    return result;
}

// TODO: Precompute and cache
glm::vec2 Sampler::SampleHalton2D(uint32_t b1, uint32_t b2, uint32_t index) {
    float u = Sample1D();
    float v = Sample1D();
    float x = Halton(index, b1) + u;
    x = x - int(x);
    float y = Halton(index, b2) + v;
    y = y - int(y);
    return glm::vec2(x, y);
}

int Sampler::SampleInt(int min, int max) {
    if (min >= max) return min;
    std::uniform_int_distribution<int> distribution(min, max - 1);
    return distribution(generator);
}

glm::vec2 Sampler::SampleUnitDiskUniform() {
    float u1 = Sample1D();
    float u2 = Sample1D();

    float r = sqrt(u1);
    float theta = 2.0f *M_PI * u2;

    float x = r * cos(theta);
    float y = r * sin(theta);

    return glm::vec2(x, y);
}

glm::vec3 Sampler::SampleSphereUniform() {
    float u1 = Sample1D();
    float u2 = Sample1D();

    float z = 1.0f - 2.0f * u1;
    float r = sqrt(glm::max(0.0f, 1.0f - z * z));
    float phi = 2.0f *M_PI * u2;

    float x = r * cos(phi);
    float y = r * sin(phi);

    return glm::vec3(x, y, z);
}

glm::vec3 Sampler::SampleHemisphereUniform(const glm::vec3& n) {

   // Sample hemisphere uniformly around the normal
   float u1 = Sample1D();
   float u2 = Sample1D();

   float r = sqrt(u1);
   float theta = 2.0f *M_PI * u2;

   float x = r * cos(theta);
   float y = r * sin(theta);
   float z = sqrt(glm::max(0.0f, 1.0f - u1));

   glm::vec3 b1, b2;
   Utils::Orthonormals(n, b1, b2);

   // Transform sample to world space
   glm::vec3 sampleDir = x * b1 + y * b2 + z * n;
   return glm::normalize(sampleDir);
}

glm::vec3 Sampler::SampleHemisphereCosine(const glm::vec3& n) {
    // Sample hemisphere with cosine-weighted distribution around the normal
    float u1 = Sample1D();
    float u2 = Sample1D();

    float r = sqrt(u1);
    float theta = 2.0f *M_PI * u2;

    float x = r * cos(theta);
    float y = r * sin(theta);
    float z = sqrt(glm::max(0.0f, 1.0f - u1));

    // Create an orthonormal basis (T, B, N)
    glm::vec3 N = glm::normalize(n);
    glm::vec3 T, B;
    if (fabs(N.x) > fabs(N.z)) {
        T = glm::normalize(glm::vec3(-N.y, N.x, 0.0f));
    } else {
        T = glm::normalize(glm::vec3(0.0f, -N.z, N.y));
    }
    B = glm::cross(N, T);

    // Transform sample to world space
    glm::vec3 sampleDir = x * T + y * B + z * N;
    return glm::normalize(sampleDir);
}

glm::vec3 Sampler::SampleHemisphereGGX(const glm::vec3& n, float r) {
    float x1 = Sample1D();
	float x2 = Sample1D();
	float theta = atan(r * sqrt(x1) / sqrt(1.0f - x1));
	float phi = 2.0f * M_PI * x2;
	glm::vec3 dLocal = glm::vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
    glm::vec3 b1, b2;
	Utils::Orthonormals(n, b1, b2);
	return dLocal.x * b1 + dLocal.y * b2 + dLocal.z * n;
}