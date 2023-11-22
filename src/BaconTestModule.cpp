#include "BaconPlugs.hpp"
#include "BaconModule.hpp"
#include "BaconModuleWidget.h"

namespace bp = baconpaul::rackplugs;

struct BaconTest : bp::BaconModule
{
    static constexpr int nParams{8}, nPorts{4};
    enum ParamIds
    {
        PARAM_0,
        NUM_PARAMS = PARAM_0 + nParams
    };

    enum InputIds
    {
        INPUT_0,
        NUM_INPUTS = INPUT_0 + nPorts
    };

    enum OutputIds
    {
        OUTPUT_0,
        NUM_OUTPUTS = OUTPUT_0 + nPorts
    };

    enum LightIds
    {
        NUM_LIGHTS
    };

    BaconTest()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        for (int i = 0; i < nParams; ++i)
            configParam(PARAM_0 + i, 0, 1, 0, "Param " + std::to_string(i));
        for (int i = 0; i < nPorts; ++i)
        {
            configInput(INPUT_0 + i, "Input " + std::to_string(i));
            configInput(OUTPUT_0 + i, "Output " + std::to_string(i));
        }
    }

    void process(const ProcessArgs &args) override {}
};

struct BaconTestWidget : bp::BaconModuleWidget
{
    BaconTestWidget(BaconTest *model);
};

BaconTestWidget::BaconTestWidget(BaconTest *model)
{
    setModule(model);
    box.size = Vec(SCREW_WIDTH * 8, RACK_HEIGHT);

    BaconBackground *bg = new BaconBackground(box.size, "BaconTest");
    addChild(bg->wrappedInFramebuffer());

    Vec cr(5, 35);
    auto dSp = (box.size.x - 10) / BaconTest::nPorts;
    for (int i = 0; i < BaconTest::nPorts; ++i)
    {
        addInput(createInput<PJ301MPort>(cr, module, BaconTest::INPUT_0 + i));
        cr.x += dSp;
    }

    cr = Vec(10, 90);
    auto kSp = (box.size.x - 20) / 2;
    for (int i = 0; i < BaconTest::nParams; ++i)
    {
        addParam(createParam<RoundLargeBlackKnob>(cr, module, BaconTest::PARAM_0 + i));
        if (i % 2 == 0)
            cr.x += kSp;
        else
        {
            cr.x = 10;
            cr.y += kSp;
        }
    }

    cr = Vec(5, 300);
    for (int i = 0; i < BaconTest::nPorts; ++i)
    {
        addOutput(createOutput<PJ301MPort>(cr, module, BaconTest::OUTPUT_0 + i));
        cr.x += dSp;
    }

#if 0
    Vec knobPos = Vec(cr.x + 12, cr.y + 25);
    Vec knobCtr = knobPos.plus(Vec(18, 18));
    addParam(createParam<RoundLargeBlackKnob>(knobPos, module, Bitulator::WET_DRY_MIX));
    bg->addLabel(knobCtr.plus(Vec(8, 21)), "dist", 10, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
    bg->addLabel(knobCtr.plus(Vec(-8, 21)), "clean", 10, NVG_ALIGN_RIGHT | NVG_ALIGN_TOP);
    knobPos.x = cr.x + rs.x - 3 - 24;
    ;
    knobPos.y += diffY2c<RoundLargeBlackKnob, PJ301MPort>();
    addInput(createInput<PJ301MPort>(knobPos, module, Bitulator::MIX_CV));

    cr.y += 88;
    bg->addRoundedBorder(cr, rs);
    bg->addLabel(Vec(cr.x + 3, cr.y + 3), "Quantize", 14, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
    addChild(createLight<SmallLight<BlueLight>>(cr.plus(Vec(rs.x - 27, 6)), module,
                                                Bitulator::BITULATING_LIGHT));

    addParam(createParam<CKSS>(cr.plus(Vec(rs.x - 17, 5)), module, Bitulator::BITULATE));

    knobPos = Vec(cr.x + 12, cr.y + 25);
    knobCtr = knobPos.plus(Vec(18, 18));
    addParam(createParam<RoundLargeBlackKnob>(knobPos, module, Bitulator::STEP_COUNT));
    bg->addLabel(knobCtr.plus(Vec(8, 21)), "smth", 10, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
    bg->addLabel(knobCtr.plus(Vec(-8, 21)), "crnch", 10, NVG_ALIGN_RIGHT | NVG_ALIGN_TOP);
    knobPos.x = cr.x + rs.x - 3 - 24;
    ;
    knobPos.y += diffY2c<RoundLargeBlackKnob, PJ301MPort>();
    addInput(createInput<PJ301MPort>(knobPos, module, Bitulator::BIT_CV));

    cr.y += 88;
    bg->addRoundedBorder(cr, rs);
    bg->addLabel(Vec(cr.x + 3, cr.y + 3), "Amp/Clip", 14, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
    addChild(createLight<SmallLight<BlueLight>>(cr.plus(Vec(rs.x - 27, 6)), module,
                                                Bitulator::CRUNCHING_LIGHT));

    addParam(createParam<CKSS>(cr.plus(Vec(rs.x - 17, 5)), module, Bitulator::CLIPULATE));

    knobPos = Vec(cr.x + 12, cr.y + 25);
    knobCtr = knobPos.plus(Vec(18, 18));
    addParam(createParam<RoundLargeBlackKnob>(knobPos, module, Bitulator::AMP_LEVEL));
    bg->addLabel(knobCtr.plus(Vec(8, 21)), "11", 10, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
    bg->addLabel(knobCtr.plus(Vec(-8, 21)), "one", 10, NVG_ALIGN_RIGHT | NVG_ALIGN_TOP);
    knobPos.x = cr.x + rs.x - 3 - 24;
    ;
    knobPos.y += diffY2c<RoundLargeBlackKnob, PJ301MPort>();
    addInput(createInput<PJ301MPort>(knobPos, module, Bitulator::AMP_CV));

    Vec inP = Vec(10, RACK_HEIGHT - 15 - 43);
    Vec outP = Vec(box.size.x - 24 - 10, RACK_HEIGHT - 15 - 43);

    bg->addPlugLabel(inP, BaconBackground::SIG_IN, "in");
    addInput(createInput<PJ301MPort>(inP, module, Bitulator::SIGNAL_INPUT));

    bg->addPlugLabel(outP, BaconBackground::SIG_OUT, "out");
    addOutput(createOutput<PJ301MPort>(outP, module, Bitulator::CRUNCHED_OUTPUT));
#endif
}

Model *modelBaconTest = createModel<BaconTest, BaconTestWidget>("BaconTest");
