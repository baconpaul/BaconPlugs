#include "BaconPlugs.hpp"

struct Bitulator : Module {
  enum ParamIds {
    WET_DRY_MIX,
    STEP_COUNT,
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
  }


  void step()
  {
    float vin = inputs[ SIGNAL_INPUT ].value;
    float wd = params[ WET_DRY_MIX ].value;

    // Signals are +/-5V signals of course. So
    float qi = params[ STEP_COUNT ].value / 2;
    float crunch = (int)( (vin/5.0) * qi ) / qi * 5.0;

    outputs[ CRUNCHED_OUTPUT ].value = wd * crunch + ( 1.0 - wd ) * vin;
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

  addParam( createParam< Davies1900hBlackKnob >( Vec( (box.size.x - 36)/2, 140 ),
                                                 module,
                                                 Bitulator::STEP_COUNT,
                                                 2, 16, 6 ));

  addOutput( createOutput< PJ301MPort >( Vec( (box.size.x - 24) / 2, RACK_HEIGHT - 15 - 30 ),
                                         module,
                                         Bitulator::CRUNCHED_OUTPUT ) );
}
