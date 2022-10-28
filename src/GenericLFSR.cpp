#include "GenericLFSR.hpp"
#include "BaconPlugs.hpp"

struct GenericLFSRWidget : ModuleWidget
{
    typedef GenericLFSR<Module> M;
    GenericLFSRWidget(M *module);
};

GenericLFSRWidget::GenericLFSRWidget(GenericLFSRWidget::M *module) : ModuleWidget()
{
    setModule(module);
    box.size = Vec(SCREW_WIDTH * 14, RACK_HEIGHT);

    BaconBackground *bg = new BaconBackground(box.size, "Generic LFSR");
    addChild(bg->wrappedInFramebuffer());

    addParam(createParam<RoundBlackKnob>(Vec(30, 30), module, M::SEED_LSB));

    addChild(SevenSegmentLight<BlueLight, 3>::createHex(Vec(30, 100), module, M::SEED_LSB));
}

Model *modelGenericLFSR = createModel<GenericLFSRWidget::M, GenericLFSRWidget>("GenericLFSR");
