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

  std::vector< float > offsets;
  
  AddOne() : Module( NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS ) {
    for( int i=0; i<OCTAVE; ++i ) offsets.push_back( 0 );

    offsets[ HALF_STEP ] = 1;
    offsets[ WHOLE_STEP ] = 2;
    offsets[ MINOR_THIRD ] = 3; 
    offsets[ MAJOR_THIRD ] = 4;
    offsets[ FIFTH ] = 7;
    offsets[ OCTAVE ] = 12;
  }

  void step() override;
};

int pct = 0;

void AddOne::step() {
  float in = inputs[ SOURCE_INPUT ].value;
  float echo = in;

  pct ++;

  float offsetI = 0;
  float uod = ( params[ UP_OR_DOWN ].value > 0 ) ? 1.0 : -1.0;
  for( int i=HALF_STEP; i <= OCTAVE; ++i )
    {
      if( params[ i ].value > 0 ) offsetI += offsets[ i ];
    }

  float increased = in +  uod * offsetI / 12.0; 

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
  addChild( createScrew<ScrewSilver>( Vec( SCREW_WIDTH, 0 ) ) );
  addChild( createScrew<ScrewSilver>( Vec( box.size.x - 2 * SCREW_WIDTH, 0 ) ) );
  addChild( createScrew<ScrewSilver>( Vec( SCREW_WIDTH, RACK_HEIGHT - SCREW_WIDTH ) ) );
  addChild( createScrew<ScrewSilver>( Vec( box.size.x - 2 * SCREW_WIDTH, RACK_HEIGHT - SCREW_WIDTH ) ) );

  addInput( createInput< PJ301MPort >( Vec( 15, 15 ), module, AddOne::SOURCE_INPUT ) );
  addOutput( createOutput<PJ301MPort>(Vec ( 10, 320 ), module, AddOne::ECHO_OUTPUT ) );
  addOutput( createOutput<PJ301MPort>(Vec ( 30, 320 ), module, AddOne::INCREASED_OUTPUT ) );

  addParam( createParam< NKK >( Vec( 15, 50 ), module, AddOne::UP_OR_DOWN, 0, 1, 1 ) );
  
  int x = 15; int y = 90; float v = -1;
  for( int i = AddOne::HALF_STEP; i <= AddOne::OCTAVE; ++i )
    {
      if( i == AddOne::OCTAVE ) { v = 1; } { v = -1; }
      addParam( createParam<NKK>( Vec( x, y ), module, i, 0, 1, v ) );
      x += 40;
      if( x + 55 > box.size.x )
        {
          x = 15;
          y += 40;
        }
    }
}
