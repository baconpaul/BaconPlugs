
#include "PolyGnome.hpp"
#include "BaconPlugs.hpp"

#include <algorithm>
#include <vector>

struct PolyGnomeWidget : ModuleWidget {
    typedef PolyGnome<Module> M;
    PolyGnomeWidget(M *module);
};

PolyGnomeWidget::PolyGnomeWidget(PolyGnomeWidget::M *module) : ModuleWidget() {
    setModule(module);
    box.size = Vec(SCREW_WIDTH * 14, RACK_HEIGHT);

    BaconBackground *bg = new BaconBackground(box.size, "PolyGnome");
    addChild(bg->wrappedInFramebuffer());

    bg->addLabel(Vec(17, 45), "Clock", 13, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
    addParam(
        createParam<RoundSmallBlackKnob>(Vec(55, 40), module, M::CLOCK_PARAM));
    addInput(createInput<PJ301MPort>(Vec(85, 40), module, M::CLOCK_INPUT));
    // FIXME - label this
    addOutput(createOutput<PJ301MPort>(Vec( 120, 40), module, M::CLOCK_CV_LEVEL));
    
    for (size_t i = 0; i <= NUM_CLOCKS; ++i) {
        Vec outP = Vec(box.size.x - 45, 100 + 48 * i);
        if (i == 0) {
            bg->addLabel(Vec(17, outP.y + 21), "Unit (1/1) clock", 13,
                         NVG_ALIGN_LEFT | NVG_ALIGN_BOTTOM);
        } else {
            int yoff = 2;
            // knob light knob light
            addParam(createParam<RoundSmallBlackKnob>(
                Vec(17, outP.y + yoff), module,
                M::CLOCK_NUMERATOR_1 + (i - 1)));
            addChild(MultiDigitSevenSegmentLight<BlueLight, 2, 2>::create(
                Vec(48, outP.y + yoff), module,
                M::LIGHT_NUMERATOR_1 + (i - 1)));

            int mv = 47 + 20 + 14 - 16;
            addParam(createParam<RoundSmallBlackKnob>(
                Vec(16 + mv, outP.y + yoff), module,
                M::CLOCK_DENOMINATOR_1 + (i - 1)));
            addChild(MultiDigitSevenSegmentLight<BlueLight, 2, 2>::create(
                Vec(47 + mv, outP.y + yoff), module,
                M::LIGHT_DENOMINATOR_1 + (i - 1)));
        }
        addOutput(createOutput<PJ301MPort>(outP, module, M::CLOCK_GATE_0 + i));
        bg->addRoundedBorder(Vec(12, outP.y - 4), Vec(box.size.x - 24, 36));
    }
}

Model *modelPolyGnome =
    createModel<PolyGnomeWidget::M, PolyGnomeWidget>("PolyGnome");
