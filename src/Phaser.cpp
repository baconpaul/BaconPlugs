#include "Phaser.hpp"
#include "BaconPlugs.hpp"

struct PhaserWidget : ModuleWidget {
    typedef PhaserModule<Module> M;
    PhaserWidget(M *model);
};

PhaserWidget::PhaserWidget(PhaserWidget::M *model) : ModuleWidget(model) {
    box.size = Vec(SCREW_WIDTH * 10, RACK_HEIGHT);
    BaconBackground *bg = new BaconBackground(box.size, "Phaser");

    addChild(bg->wrappedInFramebuffer());

    // Internal LFO section
    int yPos = 25;
    int yMargin = 4;
    // int xMargin = 5;

    bg->addRoundedBorder(Vec(10, yPos), Vec(box.size.x - 20, yPos + 25));
    bg->addLabel(Vec(bg->cx(), yPos + 12.5), "Rate LFO", 13,
                 NVG_ALIGN_MIDDLE | NVG_ALIGN_CENTER);

    yPos += 25 + yMargin;

    addParam(createParam<RoundBlackKnob>(Vec(30, 140), module, M::DEPTH));
    addInput(createInput<PJ301MPort>(Vec(bg->cx() - 40, RACK_HEIGHT - 120),
                                     module, M::SIGNAL_IN));
    addOutput(createOutput<PJ301MPort>(Vec(bg->cx() + 40, RACK_HEIGHT - 120),
                                       module, M::PHASED));
}

Model *modelPhaser = createModel<PhaserWidget::M, PhaserWidget>("Phaser");
