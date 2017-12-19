#ifndef INCLUDE_BACONPLUGS_HPP
#define INCLUDE_BACONPLUGS_HPP

#include "rack.hpp"

#include <map>
#include <vector>
#include <string>

using namespace rack;

#define SCREW_WIDTH 15
#define RACK_HEIGHT 380

extern Plugin *plugin;

#include "Components.hpp"

struct HarMoNeeWidget : BaconStyleModuleWidget {
  HarMoNeeWidget();
};

struct GlissinatorWidget : BaconStyleModuleWidget {
  GlissinatorWidget();
};

struct ALingADingWidget : BaconStyleModuleWidget {
  ALingADingWidget();
};

struct BitulatorWidget : BaconStyleModuleWidget {
  BitulatorWidget();
};

#endif
