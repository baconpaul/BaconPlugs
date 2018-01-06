#include "BaconPlugs.hpp"


Plugin *plugin;

void init(rack::Plugin *p) {
	plugin = p;
	p->slug = "BaconPlugs";
#ifdef VERSION
	p->version = TOSTRING(VERSION);
#endif
	p->website = "https://github.com/baconpaul/BaconPlugs";

        // remember these tags are in [plugins.hpp
	p->addModel(createModel<HarMoNeeWidget>("BaconPlugs", "HarMoNee", "HarMoNee", TUNER_TAG));
        p->addModel(createModel<GlissinatorWidget>("BaconPlugs", "Glissinator", "Glissinator", EFFECT_TAG)); // TODO: Fix that tag
        p->addModel(createModel<ALingADingWidget>("BaconPlugs", "ALingADing", "ALingADing", RING_MODULATOR_TAG)); // TODO: Fix that tag
        p->addModel(createModel<BitulatorWidget>("BaconPlugs", "Bitulator", "Bitulator", DISTORTION_TAG)); // TODO: Fix that tag
        p->addModel(createModel<QuantEyesWidget>("BaconPlugs", "QuantEyes", "QuantEyes", QUANTIZER_TAG)); // TODO: Fix that tag
}
