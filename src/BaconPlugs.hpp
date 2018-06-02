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

extern Model *modelHarMoNee;
extern Model *modelGlissinator;
extern Model *modelQuantEyes;
extern Model *modelPolyGnome;
extern Model *modelSampleDelay;


#ifdef BUILD_SORTACHORUS
extern Model *modelSortaChorus;
#endif

extern Model *modelChipNoise;
extern Model *modelChipWaves;
extern Model *modelChipYourWave;

extern Model *modelGenericLFSR;

extern Model *modelKarplusStrongPoly;

extern Model *modelALingADing;
extern Model *modelBitulator;

#endif
