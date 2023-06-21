#include "BaconPlugs.hpp"
#include <initializer_list>

#include "BaconModule.hpp"
#include "BaconModuleWidget.h"

#include <random>

namespace bp = baconpaul::rackplugs;



struct PolyArp : virtual bp::BaconModule
{
    enum ParamIds
    {
        NUM_PARAMS
    };

    enum InputIds
    {
        NUM_INPUTS
    };

    enum OutputIds
    {
        NUM_OUTPUTS
    };

    enum LightIds
    {
        NUM_LIGHTS,
    };

    PolyArp() : bp::BaconModule()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

    }

    void process(const ProcessArgs &args) override
    {
    }
};

struct PolyArpWidget : bp::BaconModuleWidget
{
    typedef PolyArp M;
    PolyArpWidget(PolyArp *model);
};

PolyArpWidget::PolyArpWidget(PolyArp *m) : bp::BaconModuleWidget()
{
    setModule(m);
    box.size = Vec(SCREW_WIDTH * 9, RACK_HEIGHT);

    BaconBackground *bg = new BaconBackground(box.size, "PolyArp");
    addChild(bg->wrappedInFramebuffer());
}

Model *modelPolyArp = createModel<PolyArp, PolyArpWidget>("PolyArp");
