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
	p->addModel(createModel<HarMoNeeWidget>("Bacon Music", "HarMoNee", "HarMoNee", TUNER_TAG));
        p->addModel(createModel<GlissinatorWidget>("Bacon Music", "Glissinator", "Glissinator", EFFECT_TAG)); 
        p->addModel(createModel<ALingADingWidget>("Bacon Music", "ALingADing", "ALingADing", RING_MODULATOR_TAG)); 
        p->addModel(createModel<BitulatorWidget>("Bacon Music", "Bitulator", "Bitulator", DISTORTION_TAG)); 
        p->addModel(createModel<QuantEyesWidget>("Bacon Music", "QuantEyes", "QuantEyes", QUANTIZER_TAG)); 
}
