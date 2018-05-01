
#include "BaconPlugs.hpp"
#include "SampleDelay.hpp"

struct SampleDelayWidget : ModuleWidget {
  typedef SampleDelay< Module > SD;
  SampleDelayWidget( SD *module);
};

SampleDelayWidget::SampleDelayWidget( SD *module ) : ModuleWidget( module )
{
  box.size = Vec( SCREW_WIDTH * 8, RACK_HEIGHT );

  BaconBackground *bg = new BaconBackground( box.size, "SampleDelay" );
  addChild( bg->wrappedInFramebuffer());

  int outy = 30;
  int gap = 10;
  int margin = 3;

  // plug label is 29 x 49
  Vec ppos = Vec( bg->cx( SizeTable<PJ301MPort>::X ), outy + 20 );
  bg->addPlugLabel( ppos, BaconBackground::SIG_IN, "in" );
  addInput( Port::create< PJ301MPort >( ppos, Port::INPUT,
                                        module, SD::SIGNAL_IN ) );

  outy += 49 + gap + margin;
  bg->addLabel( Vec( bg->cx(), outy ), "# samples", 11, NVG_ALIGN_CENTER | NVG_ALIGN_TOP );
  outy += 14;
  addParam( ParamWidget::create< RoundBlackSnapKnob >( Vec( bg->cx( SizeTable< RoundBlackSnapKnob >::X ), outy ),
                                                       module,
                                                       SD::DELAY_KNOB,
                                                       1, 100, 1 ) );

  outy += SizeTable<RoundBlackSnapKnob>::Y + margin;
}

Model *modelSampleDelay = Model::create<SampleDelayWidget::SD, SampleDelayWidget>("Bacon Music", "SampleDelay", "SampleDelay", DELAY_TAG ); 

