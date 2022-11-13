#include "BaconPlugs.hpp"
#include <initializer_list>

#include "BaconModule.hpp"
#include "BaconModuleWidget.h"

#define SCALE_LENGTH 12

namespace bp = baconpaul::rackplugs;



struct LuckyHold : virtual bp::BaconModule
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

    LuckyHold() : bp::BaconModule()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    }


    void process(const ProcessArgs &args) override
    {
    }
};

struct LuckyHoldWidget : bp::BaconModuleWidget
{
    LuckyHoldWidget(LuckyHold *model);
};

LuckyHoldWidget::LuckyHoldWidget(LuckyHold *m) : bp::BaconModuleWidget()
{
    setModule(m);
    box.size = Vec(SCREW_WIDTH * 18, RACK_HEIGHT);

    BaconBackground *bg = new BaconBackground(box.size, "LuckyHold");
    addChild(bg->wrappedInFramebuffer());
}

Model *modelLuckyHold = createModel<LuckyHold, LuckyHoldWidget>("LuckyHold");
