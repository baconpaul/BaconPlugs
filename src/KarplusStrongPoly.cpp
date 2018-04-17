
#include "BaconPlugs.hpp"
#include <sstream>
#include <vector>
#include <string>

#include "KSSynth.hpp"

struct KarplusStrongPoly : virtual Module {
  enum ParamIds {
    INITIAL_PACKET,
    NUM_PARAMS
  };

  enum InputIds {
    TRIGGER_GATE,
    NUM_INPUTS
  };

  enum OutputIds {
    SYNTH_OUTPUT,
    NUM_OUTPUTS
  };

  enum LightIds {
    NUM_LIGHTS
  };


  std::vector< KSSynth *> voices;
  const static int nVoices = 32;
  
  KarplusStrongPoly() : Module( NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS )
  {
    for( int i=0; i<nVoices; ++i ) voices.push_back( new KSSynth(-2.0f, 2.0f, engineGetSampleRate() ) );
    
    filterStringDirty = true;
    currentInitialPacket = KSSynth::RANDOM;
    filterString = voices[ 0 ]->initPacketName( currentInitialPacket );

    pgate = 0;
  }

  virtual ~KarplusStrongPoly()
  {
    for( auto syn : voices )
      delete syn;
  }
  
  int getNumPackets() { return voices[ 0 ]->numInitPackets(); }
  KSSynth::InitPacket currentInitialPacket;
  float pgate;
  
  void step() override
  {
    if( (int)( params[ INITIAL_PACKET ].value ) != currentInitialPacket )
      {
        filterStringDirty = true;
        currentInitialPacket = (KSSynth::InitPacket)( (int)params[ INITIAL_PACKET ].value );
        filterString = voices[ 0 ]->initPacketName( currentInitialPacket );
      }

    // Check a trigger here and find a voice
    bool newVoice = false;
    if( pgate == 0 && inputs[ TRIGGER_GATE ].value > 1 )
      {
        pgate = 10;
        newVoice = true;
      }
    if( pgate == 10 && inputs[ TRIGGER_GATE ].value < 1 ) pgate = 0;

    if( newVoice )
      {
        // find voice
        KSSynth *voice = NULL;
        for( auto syn: voices )
          if( ! syn->active )
            {
              voice = syn;
              break;
            }

        if( voice == NULL )
          {
            // info( "All voices are active: Running voice steal" );
            voice = voices[ 0 ];
            float ds = voice->sumDelaySquared;
            for( auto syn: voices )
              {
                if( syn->sumDelaySquared < ds )
                  {
                    ds = syn->sumDelaySquared;
                    voice = syn;
                  }
              }
          }
        
        // Capture parameters onto this voice and trigger it
        voice->packet = currentInitialPacket;
        voice->trigger( 440 );
      }
    
    float out = 0.0f;
    for( auto syn : voices )
      if( syn->active )
        out += syn->step();

    outputs[ SYNTH_OUTPUT ].value = out;
  }

  bool filterStringDirty;
  std::string filterString;
  
  static bool getFilterStringDirty( Module *that )
  {
    return dynamic_cast<KarplusStrongPoly *>(that)->filterStringDirty;
  }
  static std::string getFilterString( Module *that )
  {
    dynamic_cast<KarplusStrongPoly *>(that)->filterStringDirty = false;
    return dynamic_cast<KarplusStrongPoly *>(that)->filterString;
  }
  
};

struct KarplusStrongPolyWidget : ModuleWidget {
  KarplusStrongPolyWidget( KarplusStrongPoly *module);
};

KarplusStrongPolyWidget::KarplusStrongPolyWidget( KarplusStrongPoly *module ) : ModuleWidget( module )
{
  box.size = Vec( SCREW_WIDTH * 20, RACK_HEIGHT );

  BaconBackground *bg = new BaconBackground( box.size, "KarplusStrongPoly" );
 
  
  addChild( bg->wrappedInFramebuffer());
  
  addInput( Port::create< PJ301MPort >( Vec( 20, 200 ),
                                        Port::INPUT,
                                        module,
                                        KarplusStrongPoly::TRIGGER_GATE ) );

  addOutput( Port::create< PJ301MPort >( Vec( 100, 200 ),
                                         Port::OUTPUT,
                                         module,
                                         KarplusStrongPoly::SYNTH_OUTPUT ) );

  
  addParam( ParamWidget::create< RoundSmallBlackKnob >( Vec( 20, 40 ), module, KarplusStrongPoly::INITIAL_PACKET, 0, module->getNumPackets()-1, 0 ) );
  addChild( DotMatrixLightTextWidget::create( Vec( 50, 42 ), module, 8, KarplusStrongPoly::getFilterStringDirty, KarplusStrongPoly::getFilterString ) );
}

Model *modelKarplusStrongPoly = Model::create<KarplusStrongPoly, KarplusStrongPolyWidget>("Bacon Music", "KarplusStrongPoly", "KarplusStrongPoly", OSCILLATOR_TAG );

