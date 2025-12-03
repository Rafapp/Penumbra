#include "sampling.h"

Sampler::Sampler(uint32_t seed) {
    generator.seed(seed);
}

float Sampler::Sample1D() {
    std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
    return distribution(generator);
}

int Sampler::SampleInt(int min, int max) {
    std::uniform_int_distribution<int> distribution(min, max - 1);
    return distribution(generator);
}

glm::vec2 Sampler::SampleUnitDisk() {
    float u1 = Sample1D();
    float u2 = Sample1D();

    float r = sqrt(u1);
    float theta = 2.0f * M_PI * u2;

    float x = r * cos(theta);
    float y = r * sin(theta);

    return glm::vec2(x, y);
}

glm::vec3 Sampler::SampleHemisphereUniform(const glm::vec3& normal) {
   // Sample hemisphere uniformly around the normal
   float u1 = Sample1D();
   float u2 = Sample1D();

   float r = sqrt(u1);
   float theta = 2.0f * M_PI * u2;

   float x = r * cos(theta);
   float y = r * sin(theta);
   float z = sqrt(glm::max(0.0f, 1.0f - u1));

   // Create an orthonormal basis (T, B, N)
   glm::vec3 N = glm::normalize(normal);
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

glm::vec3 Sampler::SampleHemisphereCosine(const glm::vec3& normal) {
    // Sample hemisphere with cosine-weighted distribution around the normal
    float u1 = Sample1D();
    float u2 = Sample1D();

    float r = sqrt(u1);
    float theta = 2.0f * M_PI * u2;

    float x = r * cos(theta);
    float y = r * sin(theta);
    float z = sqrt(glm::max(0.0f, 1.0f - u1));

    // Create an orthonormal basis (T, B, N)
    glm::vec3 N = glm::normalize(normal);
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