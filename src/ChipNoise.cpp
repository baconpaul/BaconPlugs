#include "BaconPlugs.hpp"
#include "ChipSym.hpp"

struct ChipNoise : virtual Module {
  enum ParamIds {
    NOISE_LENGTH,
    LONG_MODE,
    NUM_PARAMS
  };

  enum InputIds {
    NOISE_LENGTH,
    NUM_INPUTS
  };

  enum OutputIds {
    NOISE_OUTPUT,
    NUM_OUTPUTS
  };

  enum LightIds {
    NUM_LIGHTS
  };

  ChipSym::NESNoise noise;
  
  ChipNoise() : Module( NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS ),
                noise( -2.0, 2.0, engineGetSampleRate() )
                
  {
    params[ LONG_MODE ].value = 1;
    params[ NOISE_LENGTH ].value = 9;
  }

  void step() override
  {
    outputs[ NOISE_OUTPUT ].value = noise.step();
  }
};

struct ChipNoiseWidget : ModuleWidget {
  ChipNoiseWidget( ChipNoise *module);
};

ChipNoiseWidget::ChipNoiseWidget( ChipNoise *module ) : ModuleWidget( module )
{
  box.size = Vec( SCREW_WIDTH * 8, RACK_HEIGHT );

  BaconBackground *bg = new BaconBackground( box.size, "ChipNoise" );
  addChild( bg->wrappedInFramebuffer());

  Vec outP = Vec( bg->cx( 24 ), RACK_HEIGHT - 15 - 43 );
  bg->addPlugLabel( outP, BaconBackground::SIG_OUT, "out" );
  addOutput( Port::create< PJ301MPort >( outP,
                                         Port::OUTPUT,
                                         module,
                                         ChipNoise::NOISE_OUTPUT ) );

}

Model *modelChipNoise = Model::create<ChipNoise, ChipNoiseWidget>("Bacon Music", "ChipNoise", "ChipNoise", NOISE_TAG );
