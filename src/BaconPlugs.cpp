#include "BaconPlugs.hpp"


Plugin *plugin;

void init(rack::Plugin *p) {
	plugin = p;
	p->slug = "BaconMusic";
#ifdef VERSION
	p->version = TOSTRING(VERSION);
#endif
	p->website = "https://github.com/baconpaul/Bacon Music";

        // remember these tags are in [plugins.hpp
        p->addModel( modelALingADing );
        p->addModel( modelHarMoNee );
        p->addModel( modelGlissinator );
        p->addModel( modelBitulator );
        p->addModel( modelQuantEyes ); 
}
