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
}
