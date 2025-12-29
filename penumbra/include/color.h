#pragma once

#include "glm/glm.hpp"

namespace Color {
	void GammaCorrect(glm::vec3& color);
	void SRGBToLinear(glm::vec3& color);
	void UnchartedTonemapPartial(glm::vec3& color);
	void UnchartedTonemapFilmic(glm::vec3& color, float bias = 2.0f);
}