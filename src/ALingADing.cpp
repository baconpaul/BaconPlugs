#include "BaconPlugs.hpp"

/*
** Based heavily on http://recherche.ircam.fr/pub/dafx11/Papers/66_e.pdf 
*/

struct ALingADing : Module {
  enum ParamIds {
    NUM_PARAMS
  };

  enum InputIds {
    SIGNAL_INPUT,
    CARRIER_INPUT,

    NUM_INPUTS
  };

  enum OutputIds {
    MODULATED_OUTPUT,
    NUM_OUTPUTS
  };

  enum LightIds {
    NUM_LIGHTS
  };

  ALingADing() : Module( NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS ) {
  }


  inline float diode_sim( float in )
  {
    if( in < 0 ) return 0;
    else return 0.2 * log( 1.0 + exp( 10 * ( in - 1 ) ) );
  }
  
  void step()
  {
    float vin = inputs[ SIGNAL_INPUT ].value;
    float vc  = inputs[ CARRIER_INPUT ].value;

    float A = 0.5 * vin + vc;
    float B = vc - 0.5 * vin;

    float dPA = diode_sim( A );
    float dMA = diode_sim( -A );
    float dPB = diode_sim( B );
    float dMB = diode_sim( -B );

    float res = dPA + dMA - dPB - dMB;
    outputs[ MODULATED_OUTPUT ].value = res;
  }
};

ALingADingWidget::ALingADingWidget()
{
  ALingADing *module = new ALingADing();
  setModule( module );
  box.size = Vec( SCREW_WIDTH * 5, RACK_HEIGHT );
  
  {
    SVGPanel *panel = new SVGPanel();
    panel->box.size = box.size;
    panel->setBackground( SVG::load( assetPlugin( plugin, "res/ALingADing.svg" ) ) );
    addChild( panel );
  }

  addInput( createInput< PJ301MPort >( Vec( 10, RACK_HEIGHT - 15 - 90 ),
                                       module,
                                       ALingADing::SIGNAL_INPUT ) );
  addInput( createInput< PJ301MPort >( Vec( box.size.x - 32, RACK_HEIGHT - 15 - 90 ),
                                       module,
                                       ALingADing::CARRIER_INPUT ) );

  addOutput( createOutput< PJ301MPort >( Vec( box.size.x - 32, RACK_HEIGHT - 15 - 30 ),
                                         module,
                                         ALingADing::MODULATED_OUTPUT ) );
}
