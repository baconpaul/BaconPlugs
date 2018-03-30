#include "BaconPlugs.hpp"


Plugin *plugin;

void init(rack::Plugin *p) {
  plugin = p;
  p->slug = "BaconMusic";
#ifdef VERSION
  p->version = TOSTRING(VERSION);
#endif
  p->website = "https://github.com/baconpaul/Bacon Music";
  
  p->addModel( modelALingADing );
  p->addModel( modelHarMoNee );
  p->addModel( modelGlissinator );
  p->addModel( modelBitulator );
  p->addModel( modelQuantEyes );
  
#ifdef BUILD_SORTACHORUS
  p->addModel( modelSortaChorus );
#endif
  
  p->addModel( modelChipNoise );
  p->addModel( modelChipWaves );
}
