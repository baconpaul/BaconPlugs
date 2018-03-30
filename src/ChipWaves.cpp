// TODO:
// - Make it actually send output on the two outputs
// - Set up a frequency knob and input
// - Set up a knob for pulse width 1-4 with a control


#include "BaconPlugs.hpp"
#include "ChipSym.hpp"

struct ChipWaves : virtual Module {
  enum ParamIds {
    NUM_PARAMS
  };

  enum InputIds {
    NUM_INPUTS
  };

  enum OutputIds {
    PULSE_OUTPUT,
    TRI_OUTPUT,
    NUM_OUTPUTS
  };

  enum LightIds {
    NUM_LIGHTS
  };

  ChipSym::NESPulse npulse;
  ChipSym::NESTriangle ntri;

  ChipWaves() : Module( NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS ),
                npulse( -2.0, 2.0, engineGetSampleRate() ),
                ntri( -2.0, 2.0, engineGetSampleRate() )
  {
    npulse.setDigWavelength( 2<<9 );
    ntri.setDigWavelength( 2<<8 );
  }

  void step() override
  {
    if( outputs[ TRI_OUTPUT ].active )
      outputs[ TRI_OUTPUT ].value = ntri.step();
    if( outputs[ PULSE_OUTPUT ].active )
      outputs[ PULSE_OUTPUT ].value = npulse.step();
    
  }
};

struct ChipWavesWidget : ModuleWidget {
  ChipWavesWidget( ChipWaves *module);
};

ChipWavesWidget::ChipWavesWidget( ChipWaves *module ) : ModuleWidget( module )
{
  box.size = Vec( SCREW_WIDTH * 8, RACK_HEIGHT );

  BaconBackground *bg = new BaconBackground( box.size, "ChipWaves" );
  addChild( bg->wrappedInFramebuffer());

  Vec outP = Vec( bg->cx( 24 ) + 25, RACK_HEIGHT - 15 - 43 );
  bg->addPlugLabel( outP, BaconBackground::SIG_OUT, "pulse" );
  addOutput( Port::create< PJ301MPort >( outP,
                                         Port::OUTPUT,
                                         module,
                                         ChipWaves::PULSE_OUTPUT ) );

  Vec outT = Vec( bg->cx( 24 ) - 25, RACK_HEIGHT - 15 - 43 );
  bg->addPlugLabel( outT, BaconBackground::SIG_OUT, "tri" );
  addOutput( Port::create< PJ301MPort >( outT,
                                         Port::OUTPUT,
                                         module,
                                         ChipWaves::TRI_OUTPUT ) );

}

Model *modelChipWaves = Model::create<ChipWaves, ChipWavesWidget>("Bacon Music", "ChipWaves", "ChipWaves", OSCILLATOR_TAG );

