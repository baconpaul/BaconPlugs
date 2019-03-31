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

  // Internal LFO section
  int yPos = 25;
  int yMargin = 4;
  //int xMargin = 5;
  
  bg->addRoundedBorder( Vec( 10, yPos ), Vec( box.size.x - 20, yPos + 25 ) );
  bg->addLabel( Vec( bg->cx(), yPos + 12.5 ), "Rate LFO", 13, NVG_ALIGN_MIDDLE | NVG_ALIGN_CENTER );
  
  yPos += 25 + yMargin;
  

  addParam( ParamWidget::create< RoundBlackKnob >( Vec( 30, 140 ), module, M::DEPTH, 0.0f, 1.0f, 1.0f ) );
  addInput( Port::create< PJ301MPort >( Vec( bg->cx() - 40, RACK_HEIGHT - 120 ), Port::INPUT, module, M::SIGNAL_IN ) );
  addOutput( Port::create< PJ301MPort >( Vec( bg->cx() + 40, RACK_HEIGHT - 120 ), Port::OUTPUT, module, M::PHASED ) );
}

Model *modelPhaser = Model::create<PhaserWidget::M,PhaserWidget>("Bacon Music", "Phaser", "Phaser", EFFECT_TAG); 
