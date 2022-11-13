#include "BaconPlugs.hpp"
#include "Style.hpp"

Plugin *pluginInstance;


std::unordered_map<std::string, int> memDebugger;

void init(rack::Plugin *p)
{
    pluginInstance = p;

    p->addModel(modelHarMoNee);
    p->addModel(modelGlissinator);
    p->addModel(modelPolyGnome);
    p->addModel(modelQuantEyes);
    p->addModel(modelSampleDelay);

    p->addModel(modelChipNoise);
    p->addModel(modelChipWaves);
    p->addModel(modelChipYourWave);

    p->addModel(modelOpen303);

    p->addModel(modelKarplusStrongPoly);

    p->addModel(modelALingADing);
    p->addModel(modelBitulator);

    p->addModel(modelPolyGenerator);
    p->addModel(modelLintBuddy);
    p->addModel(modelLuckyHold);

    baconpaul::rackplugs::BaconStyle::get();
}
