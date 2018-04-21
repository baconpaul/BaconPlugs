
#include "BaconPlugs.hpp"

#include <vector>
#include <algorithm>

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
    LIGHT_NUMERATOR_1,
    LIGHT_DENOMINATOR_1 = LIGHT_NUMERATOR_1 + NUM_CLOCKS,
    NUM_LIGHTS = LIGHT_DENOMINATOR_1 + NUM_CLOCKS
  };

  float phase;
  
  PolyGnome() : Module( NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS )
  {
    phase = 0.0f;
  }

  inline int numi( int i ) { return (int)params[ CLOCK_NUMERATOR_1 + i ].value; }
  inline int deni( int i ) { return (int)params[ CLOCK_DENOMINATOR_1 + i ].value; }
  void step() override
  {
    // this is all completely wrong it turns out
    int denprod = 1;
    for( int i=0; i<NUM_CLOCKS; ++i )
      if( outputs[ CLOCK_GATE_0 + i + 1 ].active )
        denprod *= deni( i );

    float clockTime = powf(2.0f, params[CLOCK_PARAM].value + inputs[CLOCK_INPUT].value);
    phase += clockTime * engineGetSampleTime();
    //    while( phase > denprod)
    // phase -= denprod;
    
    for( int i=0; i<NUM_CLOCKS+1 ; ++i )
      {
        bool gateIn = false;
        float lphase;
        if( i == 0 )
          lphase = phase;
        else
          lphase = phase / ( 1.0f * numi( i - 1 ) / deni( i - 1 ) );

        double ipart;
        float fractPhase = modf( lphase, &ipart );
        gateIn = (fractPhase < 0.5f);
        
        outputs[ CLOCK_GATE_0 + i ].value = gateIn ? 10.0f : 0.0f;
      }
    
    for( int i=0; i<NUM_CLOCKS; ++i )
      {
        lights[ LIGHT_NUMERATOR_1 + i ].value = (int)params[ CLOCK_NUMERATOR_1 + i ].value;
        lights[ LIGHT_DENOMINATOR_1 + i ].value = (int)params[ CLOCK_DENOMINATOR_1 + i ].value;
      }
  }
};

struct PolyGnomeWidget : ModuleWidget {
  PolyGnomeWidget( PolyGnome *module);
};

PolyGnomeWidget::PolyGnomeWidget( PolyGnome *module ) : ModuleWidget( module )
{
  box.size = Vec( SCREW_WIDTH * 12, RACK_HEIGHT );

  BaconBackground *bg = new BaconBackground( box.size, "PolyGnome" );
  addChild( bg->wrappedInFramebuffer());

  for( size_t i=0; i<= NUM_CLOCKS; ++i )
    {
      Vec outP = Vec( box.size.x - 45, 100 + 48 * i );
      if( i == 0 )
        {
          bg->addLabel( Vec( 17, outP.y + 21 ), "Unit (1/1) clock", 13, NVG_ALIGN_LEFT | NVG_ALIGN_BOTTOM );
        }
      else
        {
          int yoff = 2;
          // knob light knob light
          addParam( ParamWidget::create< RoundSmallBlackKnob >( Vec( 17, outP.y + yoff ),
                                                                module,
                                                                PolyGnome::CLOCK_NUMERATOR_1 + (i-1),
                                                                1, 8, 1 ) );
          addChild( ModuleLightWidget::create< SevenSegmentLight< BlueLight, 2 > >( Vec( 48, outP.y + yoff ),
                                                                                    module,
                                                                                    PolyGnome::LIGHT_NUMERATOR_1 + (i-1) ) );

          int mv = 47 + 7 + 14 - 16;
          addParam( ParamWidget::create< RoundSmallBlackKnob >( Vec( 16 + mv, outP.y + yoff ),
                                                                module,
                                                                PolyGnome::CLOCK_DENOMINATOR_1 + (i-1),
                                                                1, 16, 1 ) );
          addChild( MultiDigitSevenSegmentLight< BlueLight, 2, 2 >::create( Vec( 47 + mv, outP.y + yoff ),
                                                                            module,
                                                                            PolyGnome::LIGHT_DENOMINATOR_1 + (i-1) ) );
        }
      addOutput( Port::create< PJ301MPort >( outP,
                                             Port::OUTPUT,
                                             module,
                                             PolyGnome::CLOCK_GATE_0 + i ) );
      bg->addRoundedBorder( Vec( 12, outP.y - 4 ), Vec( box.size.x - 24, 36 ) );
    }

}

Model *modelPolyGnome = Model::create<PolyGnome, PolyGnomeWidget>("Bacon Music", "PolyGnome", "PolyGnome", CLOCK_TAG );

