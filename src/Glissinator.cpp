#include "Glissinator.hpp"
#include "BaconPlugs.hpp"

struct GlissinatorWidget : ModuleWidget
{
    typedef Glissinator<Module> G;
    GlissinatorWidget(Glissinator<Module> *model);
};

GlissinatorWidget::GlissinatorWidget(Glissinator<Module> *model) : ModuleWidget()
{
    setModule(model);
    box.size = Vec(SCREW_WIDTH * 5, RACK_HEIGHT);
    BaconBackground *bg = new BaconBackground(box.size, "Glissinator");

    addChild(bg->wrappedInFramebuffer());
    // FIXME - spacing
    // addChild( new BaconHelpButton( "README.md#glissinator" ) );

    ParamWidget *slider =
        createParam<GraduatedFader<230>>(Vec(bg->cx(29), 23), module, G::GLISS_TIME);

    addParam(slider);

    Vec inP = Vec(7, RACK_HEIGHT - 15 - 43);
    Vec outP = Vec(box.size.x - 24 - 7, RACK_HEIGHT - 15 - 43);

    bg->addPlugLabel(inP, BaconBackground::SIG_IN, "in");
    addInput(createInput<PJ301MPort>(inP, module, G::SOURCE_INPUT));

    bg->addPlugLabel(outP, BaconBackground::SIG_OUT, "out");

    addOutput(createOutput<PJ301MPort>(outP, module, G::SLID_OUTPUT));

    bg->addRoundedBorder(Vec(5, RACK_HEIGHT - 120), Vec(box.size.x - 10, 38),
                         BaconBackground::highlight);
    bg->addLabel(Vec(10, RACK_HEIGHT - 102), "gliss", 11, NVG_ALIGN_LEFT | NVG_ALIGN_BOTTOM,
                 componentlibrary::SCHEME_WHITE);
    bg->addLabel(Vec(10, RACK_HEIGHT - 90), "gate", 11, NVG_ALIGN_LEFT | NVG_ALIGN_BOTTOM,
                 componentlibrary::SCHEME_WHITE);
    addChild(createLight<SmallLight<BlueLight>>(Vec(bg->cx() - 4, RACK_HEIGHT - 120 + 38 / 2 - 3),
                                                module, G::SLIDING_LIGHT));
    addOutput(
        createOutput<PJ301MPort>(Vec(bg->cx() + 5, RACK_HEIGHT - 114), module, G::GLISSING_GATE));
}

Model *modelGlissinator = createModel<Glissinator<Module>, GlissinatorWidget>("Glissinator");
