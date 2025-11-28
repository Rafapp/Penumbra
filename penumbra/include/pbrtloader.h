#pragma once

#include <vector>
#include "minipbrt.h"

class PbrtLoader {
public:
  bool LoadScene(const std::string& filename);
  minipbrt::Scene GetScene(){ return* pbrtScene;}
private:
    minipbrt::Loader loader;
    minipbrt::Scene* pbrtScene = nullptr;
};

/*

minipbrt::Loader loader;
if (loader.load(filename)) {
	minipbrt::Scene* scene = loader.take_scene();
	// ... process the scene, then delete it ...
	delete scene;
}
else {
  // If parsing failed, the parser will have an error object.
  const minipbrt::Error* err = loader.error();
  fprintf(stderr, "[%s, line %lld, column %lld] %s\n",
      err->filename(), err->line(), err->column(), err->message());
  // Don't delete err, it's still owned by the parser.
}
*/