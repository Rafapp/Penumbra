#include "pbrtloader.h"

bool PbrtLoader::LoadScene(const std::string& filename) {
    // Check if file exists
    if (!std::filesystem::exists(filename)) {
        fprintf(stderr, "Error: File not found: %s\n", filename.c_str());
        return false;
    }

    // Check if it's actually a file (not directory)
    if (!std::filesystem::is_regular_file(filename)) {
        fprintf(stderr, "Error: Path is not a file: %s\n", filename.c_str());
        return false;
    }

    if (loader.load(filename.c_str())) {
        pbrtScene = loader.take_scene();
        return true;
    } else {
        if (const minipbrt::Error* err = loader.error()) {
            fprintf(stderr, "[%s, line %lld, column %lld] %s\n",
            err->filename(), err->line(), err->column(), err->message());
        } else {
            fprintf(stderr, "Unknown error loading PBRT scene\n");
        }
        return false;
    }
}

minipbrt::Scene* PbrtLoader::GetScene() {
    return this->pbrtScene;
}