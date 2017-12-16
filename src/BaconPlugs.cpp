#include "BaconPlugs.hpp"


Plugin *plugin;

void init(rack::Plugin *p) {
	plugin = p;
	p->slug = "BaconPlugs";
#ifdef VERSION
	p->version = TOSTRING(VERSION);
#endif
	p->website = "https://github.com/baconpaul/BaconPlugs";

	p->addModel(createModel<AddOneWidget>("BaconPlugs", "HarMoNee", "HarMoNee", AMPLIFIER_TAG));
        p->addModel(createModel<GlissinatorWidget>("BaconPlugs", "Glissinator", "Glissinator", AMPLIFIER_TAG)); // TODO: Fix that tag
        p->addModel(createModel<ALingADingWidget>("BaconPlugs", "ALingADing", "ALingADing", AMPLIFIER_TAG)); // TODO: Fix that tag
        p->addModel(createModel<BitulatorWidget>("BaconPlugs", "Bitulator", "Bitulator", AMPLIFIER_TAG)); // TODO: Fix that tag
}
