#pragma once
#include <rack.hpp>
#include "chowdsp_filters/chowdsp_filters.h"

using namespace rack;

// Declare the Plugin, defined in plugin.cpp
extern Plugin* pluginInstance;

// Declare each Model, defined in each module source file
extern Model* modelWDR8sd;
