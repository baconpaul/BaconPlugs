#include "BaconPlugs.hpp"
#include "ChipSym.hpp"

struct ChipNoise : virtual Module {
  enum ParamIds {
    NOISE_LENGTH,
    LONG_MODE,
    NUM_PARAMS
  };

  enum InputIds {
    NOISE_LENGTH_INPUT,
    NUM_INPUTS
  };

  enum OutputIds {
    NOISE_OUTPUT,
    NUM_OUTPUTS
  };

  enum LightIds {
    NOISE_FROM_INPUT,
    NOISE_FROM_KNOB,

    NOISE_LENGTH_ONES,
    NOISE_LENGTH_TENS,
    
    NUM_LIGHTS
  };

  ChipSym::NESNoise noise;
  
  ChipNoise() : Module( NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS ),
                noise( -5.0, 5.0, engineGetSampleRate() )
                
  {
    params[ LONG_MODE ].value = 1;
    params[ NOISE_LENGTH ].value = 9; 
  }

  void step() override
  {
    lights[ NOISE_FROM_KNOB ].value  = !inputs[ NOISE_LENGTH_INPUT ].active;
    lights[ NOISE_FROM_INPUT ].value =  inputs[ NOISE_LENGTH_INPUT ].active;

    unsigned int nl = (unsigned int)clamp( params[ NOISE_LENGTH ].value, 0.0f, 15.0f );
    if( inputs[ NOISE_LENGTH_INPUT ].active )
      nl = (unsigned int)clamp( inputs[ NOISE_LENGTH_INPUT ].value * 1.5, 0.0f, 15.0f );
    
    lights[ NOISE_LENGTH_ONES ].value = nl % 10;
    lights[ NOISE_LENGTH_TENS ].value = nl / 10;
    noise.setPeriod(nl);

    if( params[ LONG_MODE ].value == 1 )
      noise.setModeFlag( false );
    else
      noise.setModeFlag( true );
        
    
    outputs[ NOISE_OUTPUT ].value = noise.step();
  }
};

struct ChipNoiseWidget : ModuleWidget {
  ChipNoiseWidget( ChipNoise *module);
};

ChipNoiseWidget::ChipNoiseWidget( ChipNoise *module ) : ModuleWidget( module )
{
  box.size = Vec( SCREW_WIDTH * 6, RACK_HEIGHT );

  BaconBackground *bg = new BaconBackground( box.size, "ChipNoise" );
  addChild( bg->wrappedInFramebuffer());

  // Control the noise length
  bg->addRoundedBorder( Vec( 8, 45 ), Vec( SCREW_WIDTH * 6 - 16, 75 ) );
  bg->addLabel( Vec( bg->cx() + 7, 55 ), "wave", 11, NVG_ALIGN_LEFT | NVG_ALIGN_TOP );
  bg->addLabel( Vec( bg->cx() + 5, 66 ), "length", 11, NVG_ALIGN_LEFT | NVG_ALIGN_TOP );
  Vec inP = Vec( 16, 53 );
  addInput( Port::create< PJ301MPort >( inP,
                                        Port::INPUT,
                                        module,
                                        ChipNoise::NOISE_LENGTH_INPUT ) );
  addChild( ModuleLightWidget::create< SmallLight< BlueLight > >( inP.minus( Vec( 4, 4 ) ), module, ChipNoise::NOISE_FROM_INPUT ) );

  int ybot = 120;
  addParam( ParamWidget::create< RoundSmallBlackKnob >( Vec( 16, ybot - 3 - 28 ),
                                                        module,
                                                        ChipNoise::NOISE_LENGTH,
                                                        0, 15, 7 ) );
  addChild( ModuleLightWidget::create< SmallLight< BlueLight > >( Vec( 16-4, ybot - 3 - 28 -4 ), module, ChipNoise::NOISE_FROM_KNOB ) );
  addChild( ModuleLightWidget::create< SevenSegmentLight< BlueLight, 2 > >( Vec( 47, ybot - 5 - 24 ),
                                                                            module,
                                                                            ChipNoise::NOISE_LENGTH_TENS ) );
  addChild( ModuleLightWidget::create< SevenSegmentLight< BlueLight, 2 > >( Vec( 47 + 14, ybot - 5 - 24 ),
                                                                            module,
                                                                            ChipNoise::NOISE_LENGTH_ONES ) );


  bg->addRoundedBorder( Vec( 8, 170 ), Vec( SCREW_WIDTH * 6 - 16, 80 ) );
  bg->addLabel( Vec( 13, 205 ), "pattern", 11, NVG_ALIGN_LEFT | NVG_ALIGN_TOP );
  addParam( ParamWidget::create< NKK >( Vec( bg->cx() + 3, 190 ), module, ChipNoise::LONG_MODE, 0, 1, 1 ) );
  bg->addLabel( Vec( bg->cx() + 18, 175 ), "long", 11, NVG_ALIGN_CENTER | NVG_ALIGN_TOP );
  bg->addLabel( Vec( bg->cx() + 18, 237 ), "short", 11, NVG_ALIGN_CENTER| NVG_ALIGN_TOP );
  
  // Output port
  Vec outP = Vec( bg->cx( 24 ), RACK_HEIGHT - 15 - 43 );
  bg->addPlugLabel( outP, BaconBackground::SIG_OUT, "out" );
  addOutput( Port::create< PJ301MPort >( outP,
                                         Port::OUTPUT,
                                         module,
                                         ChipNoise::NOISE_OUTPUT ) );

}

Model *modelChipNoise = Model::create<ChipNoise, ChipNoiseWidget>("Bacon Music", "ChipNoise", "ChipNoise", NOISE_TAG );
