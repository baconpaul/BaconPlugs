#include "MidifilePlayer.hpp"
#include "BaconPlugs.hpp"

struct MidifilePlayerWidget : ModuleWidget {
    typedef MidifilePlayer M;
    MidifilePlayerWidget(MidifilePlayer *model);
};

MidifilePlayerWidget::MidifilePlayerWidget(MidifilePlayer *model)
    : ModuleWidget() {
    setModule(model);
    box.size = Vec(SCREW_WIDTH * 10, RACK_HEIGHT);
    BaconBackground *bg = new BaconBackground(box.size, "MidifilePlayer");

    addChild(bg->wrappedInFramebuffer());

    addParam(createParam<RoundSmallBlackKnob>(Vec(55, 40), module, M::BPM_PARAM));
    addParam(createParam<SABROGWhite>(Vec(55, 70), module, M::GO_PARAM));

    Vec toneP = Vec(7, RACK_HEIGHT - 15 - 43);
    Vec velP  = Vec(box.size.x/2 - 12, RACK_HEIGHT - 15 - 43);
    Vec gateP = Vec(box.size.x - 24 - 7, RACK_HEIGHT - 15 - 43);

    bg->addPlugLabel(toneP, BaconBackground::SIG_OUT, "1v/oct");
    addOutput(createOutput<PJ301MPort>(toneP, module, M::TONE_CV));

    bg->addPlugLabel(velP, BaconBackground::SIG_OUT, "vel");
    addOutput(createOutput<PJ301MPort>(velP, module, M::VEL_CV));

    bg->addPlugLabel(gateP, BaconBackground::SIG_OUT, "gate");
    addOutput(createOutput<PJ301MPort>(gateP, module, M::GATE_CV));
}

Model *modelMidifilePlayer =
    createModel<MidifilePlayer, MidifilePlayerWidget>("MidifilePlayer");
