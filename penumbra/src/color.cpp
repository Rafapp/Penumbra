#include "color.h"

void Color::SRGBToLinear(glm::vec3& color) {
    color.r = color.r <= 0.04045f ? color.r / 12.92f : glm::pow((color.r + 0.055f) / 1.055f, 2.4f);
    color.g = color.g <= 0.04045f ? color.g / 12.92f : glm::pow((color.g + 0.055f) / 1.055f, 2.4f);
    color.b = color.b <= 0.04045f ? color.b / 12.92f : glm::pow((color.b + 0.055f) / 1.055f, 2.4f);
}

void Color::GammaCorrect(glm::vec3& color) {
	color = glm::vec3(
		glm::pow(color.r, 1.0f / 2.2f),
		glm::pow(color.g, 1.0f / 2.2f),
		glm::pow(color.b, 1.0f / 2.2f)
	);
}

// Matt Taylor: https://64.github.io/tonemapping/
void Color::UnchartedTonemapPartial(glm::vec3& color) {
	glm::vec3 x = glm::max(glm::vec3(0.0f), color - 0.004f);
	color = (x * (6.2f * x + 0.5f)) / (x * (6.2f * x + 1.7f) + 0.06f);
}

void Color::UnchartedTonemapFilmic(glm::vec3& color, float exposureBias) {
	glm::vec3 exposed = color * exposureBias;
	UnchartedTonemapPartial(exposed);
	glm::vec3 W = glm::vec3(11.2f);
	UnchartedTonemapPartial(W);
	glm::vec3 whiteScale = glm::vec3(1.0f) / W;
	color = exposed * whiteScale;
}