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
  box.size = Vec( SCREW_WIDTH * 8, RACK_HEIGHT );
  
  {
    SVGPanel *panel = new SVGPanel();
    panel->box.size = box.size;
    panel->setBackground( SVG::load( assetPlugin( plugin, "res/Bitulator.svg" ) ) );
    addChild( panel );
  }

  addInput( createInput< PJ301MPort >( Vec( 7, 50 ),
                                       module,
                                       Bitulator::SIGNAL_INPUT ) );

  addParam( createParam< Davies1900hBlackKnob >( Vec( (box.size.x - 36)/2, 100 ),
                                                 module,
                                                 Bitulator::WET_DRY_MIX,
                                                 0, 1, 1 ));

  addParam( createParam< CKSS >( Vec( 10, 140 ), module, Bitulator::BITULATE, 0, 1, 1 ) );
  addParam( createParam< Davies1900hBlackKnob >( Vec( (box.size.x - 36)/2, 140 ),
                                                 module,
                                                 Bitulator::STEP_COUNT,
                                                 2, 16, 6 ));

  addParam( createParam< CKSS >( Vec( 10, 180 ), module, Bitulator::CLIPULATE, 0, 1, 1 ) );
  addParam( createParam< Davies1900hBlackKnob >( Vec( (box.size.x - 36)/2, 180 ),
                                                 module,
                                                 Bitulator::AMP_LEVEL,
                                                 1, 10, 1 ));

  addOutput( createOutput< PJ301MPort >( Vec( (box.size.x - 24) / 2, RACK_HEIGHT - 15 - 30 ),
                                         module,
                                         Bitulator::CRUNCHED_OUTPUT ) );
}
