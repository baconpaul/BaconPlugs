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
}

Model *modelGenericLFSR = Model::create<GenericLFSRWidget::M, GenericLFSRWidget>("Bacon Music", "GenericLFSR", "GenericLFSR", RANDOM_TAG );

