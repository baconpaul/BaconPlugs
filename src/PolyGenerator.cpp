#include "PolyGenerator.hpp"
#include "BaconPlugs.hpp"

struct PolyGeneratorWidget : baconpaul::rackplugs::BaconModuleWidget
{
    typedef PolyGenerator M;
    PolyGeneratorWidget(PolyGenerator *model);
    float y0 = 40;
    float dy = 60;
    std::vector<std::string> labels = {"Tempo (bpm)", "Voices", "Pattern", "Extra"};
    int memFont = -1;
    void drawBG(NVGcontext *vg)
    {
        memFont = InternalFontMgr::get(vg, baconpaul::rackplugs::BaconStyle::get()->fontName());

        for (int i = 0; i < 4; ++i)
        {
            nvgBeginPath(vg);
            nvgFontFaceId(vg, memFont);
            nvgFontSize(vg, 14);
            nvgFillColor(vg, baconpaul::rackplugs::BaconStyle::get()->getColor(
                                 baconpaul::rackplugs::BaconStyle::DEFAULT_LABEL));
            nvgTextAlign(vg, NVG_ALIGN_TOP | NVG_ALIGN_LEFT);
            nvgText(vg, 7, y0 + i * dy, labels[i].c_str(), NULL);
        }
    }
};

PolyGeneratorWidget::PolyGeneratorWidget(PolyGenerator *model)
{
    setModule(model);
    box.size = Vec(SCREW_WIDTH * 9, RACK_HEIGHT);
    BaconBackground *bg = new BaconBackground(box.size, "PolyGenerator");

    addChild(bg->wrappedInFramebuffer());
    bg->addDrawFunction([this](NVGcontext *vg) { this->drawBG(vg); });

    for (int i = 0; i < 4; ++i)
    {
        if (i == 1 || i == 2)
            addParam(createParam<RoundBlackSnapKnob>(Vec(7, y0 + dy * i + 16), module,
                                                     M::BPM_PARAM + i));
        else
            addParam(
                createParam<RoundBlackKnob>(Vec(7, y0 + dy * i + 16), module, M::BPM_PARAM + i));
        if (i < 2)
        {
            addChild(MultiDigitSevenSegmentLight<BlueLight, 2, 3>::create(
                Vec(box.size.x - 7 - 3 * 14, y0 + dy * i + 16 + 4), module, M::BPM_LIGHT + i));
        }
        if (i == 2)
        {
            addChild(DotMatrixLightTextWidget::create(
                Vec(41, y0 + dy * i + 16 + diffY2c<RoundBlackSnapKnob, DotMatrixLightTextWidget>()),
                module, 8, PolyGenerator::getPatternNameDirty, PolyGenerator::getPatternName));
        }
        if (i == 3)
        {
            addChild(DotMatrixLightTextWidget::create(
                Vec(41, y0 + dy * i + 16 + diffY2c<RoundBlackSnapKnob, DotMatrixLightTextWidget>()),
                module, 8, PolyGenerator::getExtraLabelDirty, PolyGenerator::getExtraLabel));
        }
        if (i == 3)
        {
            addChild(
                createLight<SmallLight<BlueLight>>(Vec(40, y0 + dy * i), module, M::EXTRA_LIGHT));
        }
    }

    Vec toneP = Vec(7, RACK_HEIGHT - 15 - 43);
    Vec velP = Vec(box.size.x / 2 - 12, RACK_HEIGHT - 15 - 43);
    Vec gateP = Vec(box.size.x - 24 - 7, RACK_HEIGHT - 15 - 43);

    bg->addPlugLabel(toneP, BaconBackground::SIG_OUT, "v/oct");
    addOutput(createOutput<PJ301MPort>(toneP, module, M::TONE_CV));

    bg->addPlugLabel(velP, BaconBackground::SIG_OUT, "vel");
    addOutput(createOutput<PJ301MPort>(velP, module, M::VEL_CV));

    bg->addPlugLabel(gateP, BaconBackground::SIG_OUT, "gate");
    addOutput(createOutput<PJ301MPort>(gateP, module, M::GATE_CV));
}

Model *modelPolyGenerator = createModel<PolyGenerator, PolyGeneratorWidget>("PolyGenerator");
