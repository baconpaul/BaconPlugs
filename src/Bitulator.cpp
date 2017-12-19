#include "BaconPlugs.hpp"

/*
** ToDo:
**   Add lights for on/off
**   Add a 7 segment display for step count
*/

struct Bitulator : Module {
  enum ParamIds {
    WET_DRY_MIX,
    STEP_COUNT,
    AMP_LEVEL,
    BITULATE,
    CLIPULATE,
    NUM_PARAMS
  };

  enum InputIds {
    SIGNAL_INPUT,
    NUM_INPUTS
  };

  enum OutputIds {
    CRUNCHED_OUTPUT,
    NUM_OUTPUTS
  };

  enum LightIds {
    NUM_LIGHTS
  };

  Bitulator() : Module( NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS ) {
    params[ WET_DRY_MIX ].value = 1.0;
    params[ STEP_COUNT ].value = 6;
    params[ AMP_LEVEL ].value = 1;
    params[ BITULATE ].value = 1;
    params[ CLIPULATE ].value = 1;
  }


  void step()
  {
    float vin = inputs[ SIGNAL_INPUT ].value;
    float wd = params[ WET_DRY_MIX ].value;

    // Signals are +/-5V signals of course. So

    float res = 0;
    if( params[ BITULATE ].value > 0 ) {
      float qi = params[ STEP_COUNT ].value / 2;
      float crunch = (int)( (vin/5.0) * qi ) / qi * 5.0;

      res = crunch;
    }
    else
    {
      res = vin;
    }

    if( params[ CLIPULATE ].value > 0 ) {
      float al = params[ AMP_LEVEL ].value;
      res = clampf( res * al, -5.0, 5.0 );
    }
        

    outputs[ CRUNCHED_OUTPUT ].value = wd * res + ( 1.0 - wd ) * vin;
  }
};

BitulatorWidget::BitulatorWidget()
{
  Bitulator *module = new Bitulator();
  setModule( module );
  box.size = Vec( SCREW_WIDTH * 6, RACK_HEIGHT );

  addChild( createBaconBG( "Bitulator" ) );

  int wdpos = 40;
  addChild( createBaconLabel( Vec( cx(), wdpos ), "Mix", 14, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE ) );
  addChild( createBaconLabel( Vec( cx() + 10, wdpos + 60 ), "Wet", 13, NVG_ALIGN_LEFT | NVG_ALIGN_TOP ) );
  addChild( createBaconLabel( Vec( cx() - 10, wdpos + 60 ), "Dry", 13, NVG_ALIGN_RIGHT | NVG_ALIGN_TOP ) );

  addParam( createParam< RoundLargeBlackKnob >( Vec( cx( 46 ), wdpos + 10 ),
                                                module,
                                                Bitulator::WET_DRY_MIX,
                                                0, 1, 1 ));

  addChild( createRoundedBorder( Vec( 5, 140 ), Vec( box.size.x-10, 70 ) ) );
  addChild( createBaconLabel( Vec( box.size.x / 2, 143 ), "Quantize", 14, NVG_ALIGN_CENTER|NVG_ALIGN_TOP ) );
  addParam( createParam< CKSS >( Vec( 10, 160 ), module, Bitulator::BITULATE, 0, 1, 1 ) );
  addParam( createParam< RoundBlackKnob >( Vec( (box.size.x - 15 - 36), 165 ),
                                                 module,
                                                 Bitulator::STEP_COUNT,
                                                 2, 16, 6 ));

  addChild( createRoundedBorder( Vec( 5, 215 ), Vec( box.size.x-10, 70 ) ) );
  addChild( createBaconLabel( Vec( box.size.x / 2, 218 ), "Amp'n'Clip", 14, NVG_ALIGN_CENTER|NVG_ALIGN_TOP ) );

  addParam( createParam< CKSS >( Vec( 10, 235 ), module, Bitulator::CLIPULATE, 0, 1, 1 ) );
  addParam( createParam< RoundBlackKnob >( Vec( (box.size.x - 15 - 36 ), 240 ),
                                                 module,
                                                 Bitulator::AMP_LEVEL,
                                                 1, 10, 1 ));

  Vec inP = Vec( 10, RACK_HEIGHT - 15 - 43 );
  Vec outP = Vec( box.size.x - 24 - 10, RACK_HEIGHT - 15 - 43 );
  
  addChild( createPlugLabel( inP, SIG_IN, "in" ) );
  addInput( createInput< PJ301MPort >( inP,
                                       module,
                                       Bitulator::SIGNAL_INPUT ) );

  addChild( createPlugLabel( outP, SIG_OUT, "out" ) );
  addOutput( createOutput< PJ301MPort >( outP,
                                         module,
                                         Bitulator::CRUNCHED_OUTPUT ) );
}
