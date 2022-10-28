#include "BaconPlugs.hpp"

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

#ifdef BUILD_SORTACHORUS
    p->addModel(modelSortaChorus);
#endif

    p->addModel(modelChipNoise);
    p->addModel(modelChipWaves);
    p->addModel(modelChipYourWave);

    p->addModel(modelOpen303);

#ifdef BUILD_GENERICLSFR
    p->addModel(modelGenericLFSR);
#endif

    p->addModel(modelKarplusStrongPoly);

    p->addModel(modelALingADing);
    p->addModel(modelBitulator);
#ifdef BUILD_PHASER
    p->addModel(modelPhaser);
#endif

    p->addModel(modelPolyGenerator);
}
