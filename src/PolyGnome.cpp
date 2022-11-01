
#include "PolyGnome.hpp"
#include "BaconPlugs.hpp"

#include <algorithm>
#include <vector>

#include "BaconModule.hpp"
#include "BaconModuleWidget.h"


namespace bp = baconpaul::rackplugs;

struct PolyGnomeWidget : bp::BaconModuleWidget
{
    typedef PolyGnome<bp::BaconModule> M;
    PolyGnomeWidget(M *module);

    int clocky0 = 20;

    void drawBG(NVGcontext *vg) {}
};

PolyGnomeWidget::PolyGnomeWidget(PolyGnomeWidget::M *module)
{
    setModule(module);
    box.size = Vec(SCREW_WIDTH * 16, RACK_HEIGHT);

    BaconBackground *bg = new BaconBackground(box.size, "PolyGnome");
    addChild(bg->wrappedInFramebuffer());
    bg->addDrawFunction([this](NVGcontext *vg) { this->drawBG(vg); });

    float kPos = clocky0 + 20;
    bg->addLabel(Vec(45, kPos - 14), "BPM", 12, NVG_ALIGN_TOP | NVG_ALIGN_CENTER);
    addParam(createParam<RoundSmallBlackKnob>(Vec(7, kPos), module, M::CLOCK_PARAM));

    Vec outP = Vec(93, kPos + 5);
    bg->addPlugLabel(outP, BaconBackground::SIG_OUT, "gate");
    addOutput(createOutput<PJ301MPort>(outP, module, M::CLOCK_GATE_0));
    outP.x += 37;
    bg->addPlugLabel(outP, BaconBackground::SIG_OUT, "bpm");
    addOutput(createOutput<PJ301MPort>(outP, module, M::CLOCK_CV_LEVEL_0));

    outP.x += 37;
    bg->addPlugLabel(outP, BaconBackground::SIG_OUT, "run");
    addOutput(createOutput<PJ301MPort>(outP, module, M::RUN_OUTPUT));

    outP.x += 37;
    bg->addPlugLabel(outP, BaconBackground::SIG_OUT, "reset");
    addOutput(createOutput<PJ301MPort>(outP, module, M::RESET_OUTPUT));
    addChild(MultiDigitSevenSegmentLight<BlueLight, 2, 3>::create(Vec(7 + 30, kPos + 2), module,
                                                                  M::BPM_LIGHT));


    outP.x = 15;
    outP.y = 98;
    outP.y += 4;
    addParam(createParam<CKSS>(outP, module, M::RUN_PARAM));
    outP.y -= 4;
    auto lp = outP;
    lp.x += 5;
    lp.y -= 3;
    bg->addLabel(lp, "run", 11, NVG_ALIGN_BOTTOM | NVG_ALIGN_CENTER);
    lp.x += 12;
    lp.y -= 8;
    addChild(createLight<SmallLight<BlueLight>>(
        lp, module, M::RUNNING_LIGHT));


    outP.x += 40;
    addParam(createParam<CKD6>(outP, module, M::RESET_PARAM));
    lp = outP;
    lp.x += 15;
    lp.y -= 3;
    bg->addLabel(lp, "reset", 11, NVG_ALIGN_BOTTOM | NVG_ALIGN_CENTER);

    outP.x = 93 + 37;
    bg->addPlugLabel(outP, BaconBackground::SIG_IN, "bpm");
    addInput(createInput<PJ301MPort>(outP, module, M::BPM_INPUT));
    outP.x += 37;
    bg->addPlugLabel(outP, BaconBackground::SIG_IN, "run");
    addInput(createInput<PJ301MPort>(outP, module, M::RUN_INPUT));
    outP.x += 37;
    bg->addPlugLabel(outP, BaconBackground::SIG_IN, "reset");
    addInput(createInput<PJ301MPort>(outP, module, M::RESET_INPUT));
    outP.x += 37;

    int bh = 41;
    std::vector<int> startX{15,43,82,110, 146, 176, 206};

    auto lb = 154;
    bg->addLabel(rack::Vec(startX[0], lb), "N beats...", 11, NVG_ALIGN_LEFT | NVG_ALIGN_BOTTOM);
    bg->addLabel(rack::Vec(startX[2], lb), "per M base", 11, NVG_ALIGN_LEFT | NVG_ALIGN_BOTTOM);
    bg->addLabel(rack::Vec(startX[4], lb), "p/w", 11, NVG_ALIGN_LEFT | NVG_ALIGN_BOTTOM);
    bg->addLabel(rack::Vec(startX[5], lb), "gate", 11, NVG_ALIGN_LEFT | NVG_ALIGN_BOTTOM);
    bg->addLabel(rack::Vec(startX[6], lb), "cv", 11, NVG_ALIGN_LEFT | NVG_ALIGN_BOTTOM);

    for (size_t i = 1; i <= NUM_CLOCKS; ++i)
    {
        auto ypos = 158 + bh * (i - 1);

        Vec outP = Vec(box.size.x - 51, ypos);
        bg->addRoundedBorder(Vec(4, outP.y - 4), Vec(box.size.x - 8, bh - 5));

        auto pv = [&](int i)
        {
            return rack::Vec(startX[i], ypos + 2);
        };
        // knob light knob light port port port
        addParam(createParam<RoundSmallBlackKnob>(pv(0), module,
                                                  M::CLOCK_NUMERATOR_0 + (i - 1)));
        addChild(MultiDigitSevenSegmentLight<BlueLight, 2, 2>::create(
            pv(1), module, M::LIGHT_DENOMINATOR_1 + (i - 1)));

        addParam(createParam<RoundSmallBlackKnob>(pv(2), module,
                                                  M::CLOCK_DENOMINATOR_0 + (i - 1)));
        addChild(MultiDigitSevenSegmentLight<BlueLight, 2, 2>::create(
           pv(3), module, M::LIGHT_NUMERATOR_1 + (i - 1)));

        addParam(createParam<RoundSmallBlackKnob>(pv(4), module,
                                                  M::CLOCK_PULSE_WIDTH_0 + (i - 1)));

        addOutput(createOutput<PJ301MPort>(pv(5), module, M::CLOCK_GATE_0 + i));
        addOutput(createOutput<PJ301MPort>(pv(6), module, M::CLOCK_CV_LEVEL_0 + i));

    }
}

Model *modelPolyGnome = createModel<PolyGnomeWidget::M, PolyGnomeWidget>("PolyGnome");
