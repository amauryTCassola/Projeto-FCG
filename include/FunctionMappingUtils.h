#ifndef FUNCTIONMAPPINGUITLS
#define FUNCTIONMAPPINGUITLS

#include <cmath>
#include <cstdio>
#include <cstdlib>

// Headers abaixo são específicos de C++
#include <map>
#include <stack>
#include <string>
#include <vector>
#include <limits>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>

#include "SceneLoadSaveUtils.h"

std::function<void(std::vector<SceneObject>&, int callerIndex)> FunctionMapping(std::string functionName);
std::function<void(std::vector<SceneObject>&, int, int)> CollisionFunctionMapping(std::string functionName);
#endif // FUNCTIONMAPPINGUITLS
