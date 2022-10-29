#include "Glissinator.hpp"
#include "BaconPlugs.hpp"
#include "BaconModule.hpp"
#include "BaconModuleWidget.h"

namespace bp = baconpaul::rackplugs;

struct GlissinatorWidget : bp::BaconModuleWidget
{
    typedef Glissinator<bp::BaconModule> G;
    GlissinatorWidget(G *model);
};

GlissinatorWidget::GlissinatorWidget(GlissinatorWidget::G *model)
{
    setModule(model);
    box.size = Vec(SCREW_WIDTH * 5, RACK_HEIGHT);
    BaconBackground *bg = new BaconBackground(box.size, "Glissinator");

    addChild(bg->wrappedInFramebuffer());
    // FIXME - spacing
    // addChild( new BaconHelpButton( "README.md#glissinator" ) );

    ParamWidget *slider =
        createParam<GraduatedFader<188>>(Vec(bg->cx(29), 23), module, G::GLISS_TIME);

    addParam(slider);

    Vec inP = Vec(7, RACK_HEIGHT - 15 - 43);
    Vec outP = Vec(box.size.x - 24 - 7, RACK_HEIGHT - 15 - 43);

    bg->addPlugLabel(inP, BaconBackground::SIG_IN, "in");
    addInput(createInput<PJ301MPort>(inP, module, G::SOURCE_INPUT));

    bg->addPlugLabel(outP, BaconBackground::SIG_OUT, "out");

    addOutput(createOutput<PJ301MPort>(outP, module, G::SLID_OUTPUT));

    auto style = baconpaul::rackplugs::BaconStyle::get();
    bg->addRoundedBorder(Vec(5, RACK_HEIGHT - 162), Vec(box.size.x - 10, 38),
                         baconpaul::rackplugs::BaconStyle::INPUT_BG);
    bg->addLabel(Vec(10, RACK_HEIGHT - 144), "gliss", 11, NVG_ALIGN_LEFT | NVG_ALIGN_BOTTOM,
                 baconpaul::rackplugs::BaconStyle::DEFAULT_LABEL);
    bg->addLabel(Vec(10, RACK_HEIGHT - 132), "cv", 11, NVG_ALIGN_LEFT | NVG_ALIGN_BOTTOM,
                 baconpaul::rackplugs::BaconStyle::DEFAULT_LABEL);
    addInput(
        createInput<PJ301MPort>(Vec(bg->cx() + 5, RACK_HEIGHT - 156), module, G::GLISS_CV_INPUT));

    bg->addRoundedBorder(Vec(5, RACK_HEIGHT - 120), Vec(box.size.x - 10, 38),
                         baconpaul::rackplugs::BaconStyle::HIGHLIGHT_BG);
    bg->addLabel(Vec(10, RACK_HEIGHT - 102), "gliss", 11, NVG_ALIGN_LEFT | NVG_ALIGN_BOTTOM,
                 baconpaul::rackplugs::BaconStyle::DEFAULT_HIGHLIGHT_LABEL);
    bg->addLabel(Vec(10, RACK_HEIGHT - 90), "gate", 11, NVG_ALIGN_LEFT | NVG_ALIGN_BOTTOM,
                 baconpaul::rackplugs::BaconStyle::DEFAULT_HIGHLIGHT_LABEL);
    addChild(createLight<SmallLight<BlueLight>>(Vec(bg->cx() - 4, RACK_HEIGHT - 120 + 38 / 2 - 3),
                                                module, G::SLIDING_LIGHT));
    addOutput(
        createOutput<PJ301MPort>(Vec(bg->cx() + 5, RACK_HEIGHT - 114), module, G::GLISSING_GATE));
}

Model *modelGlissinator = createModel<GlissinatorWidget::G, GlissinatorWidget>("Glissinator");
