#include "BaconPlugs.hpp"
#include "dsp/digital.hpp"

struct QuantEyes : virtual Module {
  enum ParamIds {
    SCALE_PARAM,
    NUM_PARAMS = SCALE_PARAM + 12
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
    SCALE_LIGHTS,
    NUM_LIGHTS = SCALE_LIGHTS + 12
  };

  int scaleState[ 12 ];
  SchmittTrigger scaleTriggers[ 12 ];

  QuantEyes() : Module( NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS )
  {
    for( int i=0; i<12; ++i ) scaleState[ i ] = 1;
  }

  void step() override
  {
    int scaleSize = 12;
    for( int i=0; i<12; ++i )
      {
        if( scaleTriggers[ i ].process( params[ SCALE_PARAM + i ].value )  )
          {
            scaleState[ i ] = ! scaleState[ i ];
          }
        lights[ SCALE_LIGHTS + i ].value = scaleState[ i ];
      }
    

    float in = inputs[ CV_INPUT ].value;
    double octave, note;
    note = modf( in, &octave );
    int noteI = floor( note * scaleSize );

    while( scaleState[ noteI ] == 0 && noteI > 0 ) noteI --;

    float out = 1.0 * noteI / scaleSize + octave;
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

  // FIXME - should be 11
  int rx = 20, ry = 30, sp = 22;
  for( int i=0; i<11; ++i )
    {
      char d[ 24 ];
      sprintf( d, "%d", i+1 );
      if( i==0 ) d[ 0 ] = 'R';
      bg->addLabel( Vec( rx - 3, i * sp + ry + sp / 2), d, 12, NVG_ALIGN_RIGHT | NVG_ALIGN_MIDDLE );
      addChild( createParam< LEDButton >( Vec( rx, i * sp + ry ), module, QuantEyes::SCALE_PARAM + i, 0, 1, 0 ) );
      addChild( createLight< MediumLight< BlueLight > >( Vec( rx + 4, i * sp + ry + 4 ), module, QuantEyes::SCALE_LIGHTS + i ) );
    }
  addOutput( createOutput< PJ301MPort >( outP,
                                         module,
                                         QuantEyes::QUANTIZED_OUT ) );
}
