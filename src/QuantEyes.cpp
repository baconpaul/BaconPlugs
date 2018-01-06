#include "BaconPlugs.hpp"

struct QuantEyes : virtual Module {
  enum ParamIds {
    NUM_PARAMS
  };

  enum InputIds {
    CV_INPUT,
    
    NUM_INPUTS
  };

  enum OutputIds {
    QUANTIZED_OUT,
    
    NUM_OUTPUTS
  };

  enum LightIds {
    NUM_LIGHTS
  };

  QuantEyes() : Module( NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS )
  {
  }

  void step() override
  {
    int scaleSize = 12;

    float in = inputs[ CV_INPUT ].value;

    float out = floor( in * scaleSize ) / scaleSize;
    outputs[ QUANTIZED_OUT ].value = out;
  }
};

QuantEyesWidget::QuantEyesWidget()
{
  QuantEyes *module = new QuantEyes();
  setModule( module );
  box.size = Vec( SCREW_WIDTH * 8, RACK_HEIGHT );

  BaconBackground *bg = new BaconBackground( box.size, "QuantEyes" );
  addChild( bg->wrappedInFramebuffer());

  Vec inP = Vec( 7, RACK_HEIGHT - 15 - 43 );
  Vec outP = Vec( box.size.x - 24 - 7, RACK_HEIGHT - 15 - 43 );
  
  bg->addPlugLabel( inP, BaconBackground::SIG_IN, "in" );
  addInput( createInput< PJ301MPort >( inP,
                                       module,
                                       QuantEyes::CV_INPUT ) );

  bg->addPlugLabel( outP, BaconBackground::SIG_OUT, "quant" );
  
  addOutput( createOutput< PJ301MPort >( outP,
                                         module,
                                         QuantEyes::QUANTIZED_OUT ) );
}
