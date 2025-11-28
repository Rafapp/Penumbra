#pragma once

#include <vector>
#include "minipbrt.h"

class PbrtLoader {
public:
  bool LoadScene(const std::string& filename);
  minipbrt::Scene* GetScene();
private:
    minipbrt::Loader loader;
    minipbrt::Scene* pbrtScene = nullptr;
};