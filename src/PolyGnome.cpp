
#include "PolyGnome.hpp"
#include "BaconPlugs.hpp"

#include <algorithm>
#include <vector>

struct PolyGnomeWidget : ModuleWidget {
    typedef PolyGnome<Module> M;
    PolyGnomeWidget(M *module);

    int clocky0 = 20;
    
    void drawBG(NVGcontext *vg) {
    }
};

PolyGnomeWidget::PolyGnomeWidget(PolyGnomeWidget::M *module) : ModuleWidget() {
    setModule(module);
    box.size = Vec(SCREW_WIDTH * 11, RACK_HEIGHT);

    BaconBackground *bg = new BaconBackground(box.size, "PolyGnome");
    addChild(bg->wrappedInFramebuffer());
    bg->addDrawFunction([this](NVGcontext *vg) { this->drawBG(vg); });

    float kPos = clocky0 + 20;
    bg->addLabel(Vec( 45, kPos - 14), "Clock BPM", 12, NVG_ALIGN_TOP | NVG_ALIGN_CENTER );
    addParam(createParam<RoundSmallBlackKnob>(Vec(7, kPos), module, M::CLOCK_PARAM));

    // FIXME - label this
    Vec outP = Vec(93, kPos + 5);
    bg->addPlugLabel(outP, BaconBackground::SIG_OUT, "gate" );
    addOutput(createOutput<PJ301MPort>(outP, module, M::CLOCK_GATE_0));
    outP.x = 130;
    bg->addPlugLabel(outP, BaconBackground::SIG_OUT, "clk cv" );
    addOutput(createOutput<PJ301MPort>(outP, module, M::CLOCK_CV_LEVEL_0));


    addChild(MultiDigitSevenSegmentLight<BlueLight, 2, 3>::create(
                 Vec(7+30, kPos+2), module,
                 M::BPM_LIGHT));

    int bh = 71;
    for (size_t i = 1; i <= NUM_CLOCKS; ++i) {
        Vec outP = Vec(box.size.x - 51, 83 + bh * (i-1));
        bg->addRoundedBorder(Vec(10, outP.y - 4), Vec(box.size.x - 20, bh-5));

        int yoff = 2;
        // knob light knob light
        addParam(createParam<RoundSmallBlackKnob>(
                     Vec(15, outP.y + yoff), module,
                     M::CLOCK_NUMERATOR_1 + (i - 1)));
        addChild(MultiDigitSevenSegmentLight<BlueLight, 2, 2>::create(
                     Vec(46, outP.y + yoff), module,
                     M::LIGHT_NUMERATOR_1 + (i - 1)));
        
        int mv = 47 + 20 + 14 - 8;
        addParam(createParam<RoundSmallBlackKnob>(
                     Vec(12 + mv, outP.y + yoff), module,
                     M::CLOCK_DENOMINATOR_1 + (i - 1)));
        addChild(MultiDigitSevenSegmentLight<BlueLight, 2, 2>::create(
                     Vec(43 + mv, outP.y + yoff), module,
                     M::LIGHT_DENOMINATOR_1 + (i - 1)));
        outP.x = 15;
        outP.y += 30;

        Vec lp = outP;
        lp.y += 12;
        bg->addLabel(lp, "Gate", 14, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE );
        outP.x = 50;
        addOutput(createOutput<PJ301MPort>(outP, module, M::CLOCK_GATE_0 + i));

        lp.x = 90;
        bg->addLabel(lp, "CV", 14, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE );
        outP.x = 90 + 30;
        addOutput(createOutput<PJ301MPort>(outP, module, M::CLOCK_CV_LEVEL_0 + i));

    }
}

Model *modelPolyGnome =
    createModel<PolyGnomeWidget::M, PolyGnomeWidget>("PolyGnome");
