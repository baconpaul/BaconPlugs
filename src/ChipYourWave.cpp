
#include "BaconPlugs.hpp"
#include "ChipSym.hpp"

struct ChipYourWave : virtual Module {
  enum ParamIds {
    FREQ_KNOB,

    WAVEFORM_START,
    NUM_PARAMS = WAVEFORM_START + 32
  };

  enum InputIds {
    FREQ_CV,
    NUM_INPUTS
  };

  enum OutputIds {
    WAVE_OUTPUT,
    NUM_OUTPUTS
  };

  enum LightIds {
    NUM_LIGHTS
  };

  ChipSym::NESArbitraryWaveform narb;

  ChipYourWave() : Module( NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS ),
                   narb( -2.0, 2.0, engineGetSampleRate() )
  {
    narb.setDigWavelength( 2<<8 );
  }

  float digWFInSeconds( float pitchKnob, float pitchCV )
  {
    // This is the frequency tuning used in Fundamental/VCO so lets be consistent
    float pitch = pitchKnob + pitchCV;
    float freq  = 261.626f * powf( 2.0f, pitch / 12.0f );
    // OK so now we have the frequency. We need the wavelength though. Simple
    float wl    = 1.0f / freq;
        
    return wl;
  }
  
  void step() override
  {
    float dwf = digWFInSeconds( params[ FREQ_KNOB ].value, 12.0f * inputs[ FREQ_CV ].value );

    narb.setWavelengthInSeconds( dwf );

    if( outputs[ WAVE_OUTPUT ].active )
      outputs[ WAVE_OUTPUT ].value = narb.step();
  }
};

struct ChipYourWaveWidget : ModuleWidget {
  ChipYourWaveWidget( ChipYourWave *module);
};

ChipYourWaveWidget::ChipYourWaveWidget( ChipYourWave *module ) : ModuleWidget( module )
{
  box.size = Vec( SCREW_WIDTH * 20, RACK_HEIGHT );

  BaconBackground *bg = new BaconBackground( box.size, "ChipYourWave" );
  addChild( bg->wrappedInFramebuffer());

  Vec outP = Vec( bg->cx( 24 ) + 25, RACK_HEIGHT - 15 - 43 );
  bg->addPlugLabel( outP, BaconBackground::SIG_OUT, "out" );
  addOutput( Port::create< PJ301MPort >( outP,
                                         Port::OUTPUT,
                                         module,
                                         ChipYourWave::WAVE_OUTPUT ) );

  bg->addLabel( Vec( 50, 45 ), "Freq", 14, NVG_ALIGN_LEFT | NVG_ALIGN_BOTTOM );
  addParam( ParamWidget::create< RoundHugeBlackKnob >( Vec( 10, 50 ), module,
                                                        ChipYourWave::FREQ_KNOB, -54.0f, 54.0f, 0.0f ) );
  Vec fcv = Vec( 56 + 20, 45 + 30 );
  bg->addPlugLabel( fcv, BaconBackground::SIG_IN, "v/o" );
  addInput( Port::create< PJ301MPort >( fcv,
                                        Port::INPUT,
                                        module,
                                        ChipYourWave::FREQ_CV ) );

  addParam( ParamWidget::create< NStepDraggableLEDWidget< 16, GreenFromZeroColorModel >>( Vec( 10, 120 ), module,
                                                                                          ChipYourWave::WAVEFORM_START, 0, 15, 8 ) );

    addParam( ParamWidget::create< NStepDraggableLEDWidget< 16, RedGreenFromMiddleColorModel >>( Vec( 40, 120 ), module,
                                                                                          ChipYourWave::WAVEFORM_START + 1, 0, 15, 8 ) );


}

Model *modelChipYourWave = Model::create<ChipYourWave, ChipYourWaveWidget>("Bacon Music", "ChipYourWave", "ChipYourWave", OSCILLATOR_TAG );

