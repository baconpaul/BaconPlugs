#include "BaconPlugs.hpp"

#include "dsp/samplerate.hpp"
#include "dsp/ringbuffer.hpp"
#include "dsp/filter.hpp"



struct AddOne : Module {
  enum ParamIds {
    UP_OR_DOWN,
    HALF_STEP,
    WHOLE_STEP,
    MINOR_THIRD,
    MAJOR_THIRD,
    FIFTH,
    OCTAVE,
    NUM_PARAMS
  };
  enum InputIds {
    SOURCE_INPUT,
    NUM_INPUTS
  };
  enum OutputIds {
    ECHO_OUTPUT,
    INCREASED_OUTPUT,
    NUM_OUTPUTS
  };
  enum LightIds {
    UP_LIGHT,
    DOWN_LIGHT,
    HALF_STEP_LIGHT,
    WHOLE_STEP_LIGHT,
    MINOR_THIRD_LIGHT,
    MAJOR_THIRD_LIGHT,
    FIFTH_LIGHT,
    OCTAVE_LIGHT,
    NUM_LIGHTS
  };
  
  std::vector< float > offsets;
  float priorOffset;
  int offsetCount;
  
  AddOne() : Module( NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS ) {
    for( int i=0; i<OCTAVE; ++i ) offsets.push_back( 0 );

    offsets[ HALF_STEP ] = 1;
    offsets[ WHOLE_STEP ] = 2;
    offsets[ MINOR_THIRD ] = 3; 
    offsets[ MAJOR_THIRD ] = 4;
    offsets[ FIFTH ] = 7;
    offsets[ OCTAVE ] = 12;
    priorOffset = 0;
    offsetCount = 0;
  }

  void step() override;
};

int pct = 0;

void AddOne::step() {
  /* TODO
     
     Fix the "fast toggle" situation by adding an "in transition" state and doing that state management
     Didsplay the shift 
  */

  float in = inputs[ SOURCE_INPUT ].value;
  float echo = in;

  pct ++;

  float offsetI = 0;
  float uod = ( params[ UP_OR_DOWN ].value > 0 ) ? 1.0 : -1.0;
  if( uod > 0 )
    {
      lights[ UP_LIGHT ].value = 1; lights[ DOWN_LIGHT ].value = 0;
    }
  else
    {
      lights[ DOWN_LIGHT ].value = 1; lights[ UP_LIGHT ].value = 0;
    }

  int ld = HALF_STEP_LIGHT - HALF_STEP;
  for( int i=HALF_STEP; i <= OCTAVE; ++i )
    {
      if( params[ i ].value > 0 )
        {
          lights[ i + ld ].value = 1.0;
          offsetI += offsets[ i ];
        }
      else
        {
          lights[ i + ld ].value = 0.0;
        }
    }
  offsetI = uod * offsetI / 12.0;

  int shift_time = 44000/20;
  if( offsetCount == 0 && offsetI != priorOffset )
    {
      offsetCount = 1;
    }
  if( offsetCount == shift_time )
    {
      priorOffset = offsetI; offsetCount = 0;
    }
  if( offsetCount > 0 )
    {
      offsetI = ( ( shift_time - offsetCount ) * priorOffset + offsetCount * offsetI ) / shift_time;
      offsetCount ++;
    }
  
  float increased = in + offsetI;

  outputs[ ECHO_OUTPUT ].value = echo;
  outputs[ INCREASED_OUTPUT ].value = increased;
}

AddOneWidget::AddOneWidget()
{
  AddOne *module = new AddOne();
  setModule( module );
  box.size = Vec( SCREW_WIDTH*8 , RACK_HEIGHT );
  {
    SVGPanel *panel = new SVGPanel();
    panel->box.size = box.size;
    panel->setBackground( SVG::load( assetPlugin( plugin, "res/AddOne.svg" ) ) );
    addChild( panel );
  }

  // These components are 32 x 32 so
  int comp = 5;
  int space = (box.size.x - comp*2)/ 3;
  int margin = ( space - 32 );
  
  addInput( createInput< PJ301MPort >( Vec( comp*2 + 0 * space + margin, 335 ), module, AddOne::SOURCE_INPUT ) );
  addOutput( createOutput<PJ301MPort>(Vec ( comp*2 + 1 * space + margin, 335 ), module, AddOne::ECHO_OUTPUT ) );
  addOutput( createOutput<PJ301MPort>(Vec ( comp*2 + 2 * space + margin, 335 ), module, AddOne::INCREASED_OUTPUT ) );

  // NKK is 32 x 44
  addParam( createParam< NKK >( Vec( 80, 16 ), module, AddOne::UP_OR_DOWN, 0, 1, 1 ) );
  addChild( createLight< MediumLight< GreenLight >>( Vec( 70, 16 + 22 - 4 - 5 ), module, AddOne::UP_LIGHT ) );
  addChild( createLight< MediumLight< RedLight >>( Vec( 70, 16 + 22 - 4 + 5 ), module, AddOne::DOWN_LIGHT ) );
  
  int x = 80; int y = 16 + 45; float v = -1;
  int ld = AddOne::HALF_STEP_LIGHT - AddOne::HALF_STEP;

  for( int i = AddOne::HALF_STEP; i <= AddOne::OCTAVE; ++i )
    {
      if( i == AddOne::OCTAVE ) { v = 1; } { v = -1; }
      addParam( createParam<NKK>( Vec( x, y ), module, i, 0, 1, v ) );
      addChild( createLight< MediumLight< BlueLight > >( Vec( 70, y + 22 - 5 ), module, i + ld ) );
      y += 45;
    }
}
