#pragma once

#include "texture.h"

struct MaterialParams {
  std::vector<const wchar_t*> diffPaths;
  const wchar_t* normalPath;
  float shines;
};
