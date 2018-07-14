#include "BaconPlugs.hpp"
#include "GenericLFSR.hpp"

struct GenericLFSRWidget : ModuleWidget {
  typedef GenericLFSR< Module > M;
  GenericLFSRWidget( M *module);
};

GenericLFSRWidget::GenericLFSRWidget( GenericLFSRWidget::M *module ) : ModuleWidget( module )
{
  box.size = Vec( SCREW_WIDTH * 14, RACK_HEIGHT );

  BaconBackground *bg = new BaconBackground( box.size, "Generic LFSR" );
  addChild( bg->wrappedInFramebuffer());

  addParam( ParamWidget::create< RoundBlackKnob >( Vec( 30, 30 ),
                                                   module,
                                                   M::SEED_LSB,
                                                   0, 15, 0 ) );

  addChild( SevenSegmentLight< BlueLight, 3 >::createHex( Vec( 30, 100 ), module, M::SEED_LSB ) );
}

Model *modelGenericLFSR = Model::create<GenericLFSRWidget::M, GenericLFSRWidget>("Bacon Music", "GenericLFSR", "GenericLFSR", RANDOM_TAG );

