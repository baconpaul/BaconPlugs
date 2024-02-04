
#include "SampleDelay.hpp"
#include "BaconPlugs.hpp"

#include "BaconModule.hpp"
#include "BaconModuleWidget.h"

namespace bp = baconpaul::rackplugs;

struct SampleDelayWidget : bp::BaconModuleWidget
{
    typedef SampleDelay<bp::BaconModule> SD;
    SampleDelayWidget(SD *module);
};

SampleDelayWidget::SampleDelayWidget(SD *module)
{
    setModule(module);
    box.size = Vec(SCREW_WIDTH * 3, RACK_HEIGHT);

    BaconBackground *bg = new BaconBackground(box.size, "SmpDly");
    addChild(bg->wrappedInFramebuffer());

    int outy = 30;
    int gap = 10;
    int margin = 3;

    // plug label is 29 x 49
    Vec ppos = Vec(bg->cx(SizeTable<PJ301MPort>::X), outy + 20);
    bg->addPlugLabel(ppos, BaconBackground::SIG_IN, "in");
    addInput(createInput<PJ301MPort>(ppos, module, SD::SIGNAL_IN));

    outy += 90 + gap + margin;
    /*
    bg->addRoundedBorder(Vec(bg->cx() - 14 * 1.5 - margin, outy - margin),
                         Vec(14 * 3 + 2 * margin, 14 + SizeTable<RoundBlackSnapKnob>::Y +
                                                      2 * margin + 22 + margin + 2 * margin));
    */
    bg->addLabel(Vec(bg->cx(), outy), "# samples", 11, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);
    outy += 14;
    addParam(createParam<RoundBlackSnapKnob>(Vec(bg->cx(SizeTable<RoundBlackSnapKnob>::X), outy),
                                             module, SD::DELAY_KNOB));

    outy += SizeTable<RoundBlackSnapKnob>::Y + 2 * margin;
    addChild(MultiDigitSevenSegmentLight<BlueLight, 2, 3>::create(Vec(bg->cx() - 14 * 1.5, outy),
                                                                  module, SD::DELAY_VALUE_LIGHT));
    outy += 22 + gap + margin;

    ppos = Vec(bg->cx(SizeTable<PJ301MPort>::X), RACK_HEIGHT - 15 - 43);
    bg->addPlugLabel(ppos, BaconBackground::SIG_OUT, "out");
    addOutput(createOutput<PJ301MPort>(ppos, module, SD::SIGNAL_OUT));
}

Model *modelSampleDelay = createModel<SampleDelayWidget::SD, SampleDelayWidget>("SampleDelay");
