
#include "PleaseQuit.hpp"
#include "BaconPlugs.hpp"

#include "BaconModule.hpp"
#include "BaconModuleWidget.h"

namespace bp = baconpaul::rackplugs;

struct PleaseQuitWidget : bp::BaconModuleWidget
{
    typedef PleaseQuit<bp::BaconModule> SD;
    PleaseQuitWidget(SD *module);

    BufferedDrawFunctionWidget *bdw{nullptr};

    void drawLabel(NVGcontext *vg)
    {
        nvgSave(vg);
        nvgRotate(vg, M_PI_2);

        auto memFont =
            InternalFontMgr::get(vg, baconpaul::rackplugs::BaconStyle::get()->fontName());
        auto labelColor = baconpaul::rackplugs::BaconStyle::get()->getColor(
            baconpaul::rackplugs::BaconStyle::DEFAULT_MUTED_LABEL);

        nvgBeginPath(vg);
        nvgFontFaceId(vg, memFont);
        nvgFontSize(vg, 12);
        nvgFillColor(vg, labelColor);
        nvgTextAlign(vg, NVG_ALIGN_MIDDLE | NVG_ALIGN_LEFT);

        auto plabel = std::string("PleaseQuit is a utility module to shut down rack");
        nvgText(vg, 5, -box.size.x * 0.77, plabel.c_str(), nullptr);

        nvgBeginPath(vg);
        nvgFontFaceId(vg, memFont);
        nvgFontSize(vg, 24);
        nvgFillColor(vg, labelColor);
        nvgTextAlign(vg, NVG_ALIGN_MIDDLE | NVG_ALIGN_LEFT);

        auto flabel = std::string("Rack Quits on Trigger");
        nvgText(vg, 5, -box.size.x * 0.30, flabel.c_str(), nullptr);
        nvgRestore(vg);
    };
};

PleaseQuitWidget::PleaseQuitWidget(SD *module)
{
    setModule(module);
    box.size = Vec(SCREW_WIDTH * 3, RACK_HEIGHT);

    BaconBackground *bg = new BaconBackground(box.size, "Quit");
    addChild(bg->wrappedInFramebuffer());

    int outy = 30;
    //int gap = 10;
    //int margin = 3;

    // plug label is 29 x 49
    Vec ppos = Vec(bg->cx(SizeTable<PJ301MPort>::X), outy + 20);
    bg->addPlugLabel(ppos, BaconBackground::SIG_IN, "bye!");
    addInput(createInput<PJ301MPort>(ppos, module, SD::PLEASE_QUIT));

    bdw = new BufferedDrawFunctionWidget(rack::Vec(0, outy + 70), rack::Vec(box.size.x, box.size.y - 70 - 25),
                                         [this](auto *vg) { drawLabel(vg); });
    addChild(bdw);
}

Model *modelPleaseQuit = createModel<PleaseQuitWidget::SD, PleaseQuitWidget>("PleaseQuit");
