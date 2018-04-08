
#include "BaconPlugs.hpp"

#define NUM_CLOCKS 4

struct PolyGnome : virtual Module {
  enum ParamIds {
    CLOCK_PARAM,
    CLOCK_NUMERATOR_1,
    CLOCK_DENOMINATOR_1 = CLOCK_NUMERATOR_1 + NUM_CLOCKS,
    NUM_PARAMS = CLOCK_DENOMINATOR_1 + NUM_CLOCKS,
  };

  enum InputIds {
    CLOCK_INPUT,
    NUM_INPUTS,
  };

  enum OutputIds {
    CLOCK_GATE_0,
    
    NUM_OUTPUTS = CLOCK_GATE_0 + NUM_CLOCKS + 1 // the "1" is for the 1/4 note clock which isn't parameterized
  };

  enum LightIds {
    
    NUM_LIGHTS
  };

  float phase;
  long nticks;
  
  PolyGnome() : Module( NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS )
  {
    phase = 0.f;
    nticks = 0;
  }

  void step() override
  {
    bool gateIn = false;
    float clockTime = powf(2.0f, params[CLOCK_PARAM].value + inputs[CLOCK_INPUT].value);
    
    phase += clockTime * engineGetSampleTime();
    if (phase >= 1.0f) {
      phase -= 1.0f;
      nticks ++;
    }
    gateIn = (phase < 0.5f);
    outputs[ CLOCK_GATE_0 ].value = gateIn ? 10.0f : 0.0f;
  }
};

struct PolyGnomeWidget : ModuleWidget {
  PolyGnomeWidget( PolyGnome *module);
};

PolyGnomeWidget::PolyGnomeWidget( PolyGnome *module ) : ModuleWidget( module )
{
  box.size = Vec( SCREW_WIDTH * 13, RACK_HEIGHT );

  BaconBackground *bg = new BaconBackground( box.size, "PolyGnome" );
  addChild( bg->wrappedInFramebuffer());

  for( size_t i=0; i<= NUM_CLOCKS; ++i )
    {
      Vec outP = Vec( box.size.x - 50, 100 + 55 * i );
      bg->addPlugLabel( outP, BaconBackground::SIG_OUT, "g" );
      addOutput( Port::create< PJ301MPort >( outP,
                                             Port::OUTPUT,
                                             module,
                                             PolyGnome::CLOCK_GATE_0 + i ) );
    }

}

Model *modelPolyGnome = Model::create<PolyGnome, PolyGnomeWidget>("Bacon Music", "PolyGnome", "PolyGnome", CLOCK_TAG );

