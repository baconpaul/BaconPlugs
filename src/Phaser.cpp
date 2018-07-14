#include "BaconPlugs.hpp"
#include "Phaser.hpp"

struct PhaserWidget : ModuleWidget {
  typedef PhaserModule< Module > M;
  PhaserWidget( M *model );
};

PhaserWidget::PhaserWidget( PhaserWidget::M *model ) : ModuleWidget( model )
{
  box.size = Vec( SCREW_WIDTH * 10, RACK_HEIGHT );
  BaconBackground *bg = new BaconBackground( box.size, "Phaser" );

  addChild( bg->wrappedInFramebuffer() );

  addInput( Port::create< PJ301MPort >( Vec( bg->cx() - 40, RACK_HEIGHT - 120 ), Port::INPUT, module, M::SIGNAL_IN ) );
  addOutput( Port::create< PJ301MPort >( Vec( bg->cx() + 40, RACK_HEIGHT - 120 ), Port::OUTPUT, module, M::PHASED ) );
}

Model *modelPhaser = Model::create<PhaserWidget::M,PhaserWidget>("Bacon Music", "Phaser", "Phaser", EFFECT_TAG); 
