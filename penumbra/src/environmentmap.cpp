#include "environmentmap.h"

EnvironmentMap::EnvironmentMap(std::string filepath) {
    this->filepath = filepath;
}

bool EnvironmentMap::Load(){
    std::vector<uint8_t> pixels;
    int xres, yres, nchannels; 
    if(!Image::LoadImage(filepath.c_str(), pixels, xres, yres, nchannels)){
        std::cerr << "Failed to load environment map image: " << filepath << std::endl; 
        return false;
    }
}

EnvironmentMap::~EnvironmentMap() {
}