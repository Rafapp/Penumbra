#include "pbrtloader.h"

bool PbrtLoader::LoadScene(const std::string& filename) {
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