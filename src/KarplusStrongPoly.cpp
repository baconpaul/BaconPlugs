
#include "BaconPlugs.hpp"
#include <sstream>
#include <vector>
#include <string>
#include "dsp/digital.hpp"

#include "KSSynth.hpp"

struct KarplusStrongPoly : virtual Module {
  enum ParamIds {
    INITIAL_PACKET,
    FILTER_TYPE,
    FREQ_KNOB,
    ATTEN_KNOB,
    NUM_PARAMS
  };

  enum InputIds {
    TRIGGER_GATE,
    INITIAL_PACKET_INPUT,
    FREQ_CV,
    ATTEN_CV,
    NUM_INPUTS
  };

  enum OutputIds {
    SYNTH_OUTPUT,
    NUM_OUTPUTS
  };

  enum LightIds {
    NUM_LIGHTS
  };


  SchmittTrigger voiceTrigger;

  std::vector< KSSynth *> voices;
  const static int nVoices = 32;
  
  KarplusStrongPoly() : Module( NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS )
  {
    for( int i=0; i<nVoices; ++i ) voices.push_back( new KSSynth(-2.0f, 2.0f, engineGetSampleRate() ) );
    
    initPacketStringDirty = true;
    currentInitialPacket = KSSynth::RANDOM;
    initPacketString = voices[ 0 ]->initPacketName( currentInitialPacket );

    filterStringDirty = true;
    currentFilter = KSSynth::WEIGHTED_ONE_SAMPLE;
    filterString = voices[ 0 ]->filterTypeName( currentFilter );

  }

  virtual ~KarplusStrongPoly()
  {
    for( auto syn : voices )
      delete syn;
  }
  
  int getNumPackets() { return voices[ 0 ]->numInitPackets(); }
  KSSynth::InitPacket currentInitialPacket;

  int getNumFilters() { return voices[ 0 ]->numFilterTypes(); }
  KSSynth::FilterType currentFilter;
  
  void step() override
  {
    if( (int)( params[ INITIAL_PACKET ].value ) != currentInitialPacket )
      {
        initPacketStringDirty = true;
        currentInitialPacket = (KSSynth::InitPacket)( (int)params[ INITIAL_PACKET ].value );
        initPacketString = voices[ 0 ]->initPacketName( currentInitialPacket );
      }

    // Check a trigger here and find a voice
    bool newVoice = false;
    if( voiceTrigger.process( inputs[ TRIGGER_GATE ].value ) )
      {
        newVoice = true;
      }

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
        float pitch = params[ FREQ_KNOB ].value + 12.0f * inputs[ FREQ_CV ].value;
        float freq = 261.262f * powf( 2.0f, pitch / 12.0f );

        voice->packet = currentInitialPacket;
        voice->trigger( freq );
      }
    
    float out = 0.0f;
    for( auto syn : voices )
      if( syn->active )
        out += syn->step();

    outputs[ SYNTH_OUTPUT ].value = out;
  }

  bool initPacketStringDirty;
  std::string initPacketString;
  
  static bool getInitialPacketStringDirty( Module *that )
  {
    return dynamic_cast<KarplusStrongPoly *>(that)->initPacketStringDirty;
  }
  static std::string getInitialPacketString( Module *that )
  {
    dynamic_cast<KarplusStrongPoly *>(that)->initPacketStringDirty = false;
    return dynamic_cast<KarplusStrongPoly *>(that)->initPacketString;
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
  box.size = Vec( SCREW_WIDTH * 15, RACK_HEIGHT );

  BaconBackground *bg = new BaconBackground( box.size, "KarplusStrongPoly" );
 
  
  addChild( bg->wrappedInFramebuffer());

  float outy;
  float yh;
  int margin = 4;
  int obuf = 10;
  
  outy = 40;

  auto brd = [&](float ys)
    {
      bg->addRoundedBorder( Vec( obuf, outy - margin ), Vec( box.size.x - 2 * obuf, ys + 2 * margin ) );
    };
  auto cl = [&](std::string lab, float ys)
    {
      bg->addLabel( Vec( obuf + margin, outy + ys / 2 ), lab.c_str(), 13, NVG_ALIGN_MIDDLE | NVG_ALIGN_LEFT );
    };

  yh = SizeTable<PJ301MPort>::Y;
  brd( yh );
  cl( "Trigger", yh );
  addInput( Port::create< PJ301MPort >( Vec( box.size.x - obuf - margin - SizeTable<PJ301MPort>::X, outy ),
                                        Port::INPUT,
                                        module,
                                        KarplusStrongPoly::TRIGGER_GATE ) );

  outy += yh + 3 * margin;
  yh = SizeTable<RoundBlackKnob >::Y;
  brd( yh );
  cl( "Freq", yh );
  int xp = box.size.x - margin - obuf - SizeTable<PJ301MPort>::X;
  addInput( Port::create< PJ301MPort >( Vec( xp, outy + diffY2c< RoundBlackKnob, PJ301MPort >() ),
                                        Port::INPUT,
                                        module,
                                        KarplusStrongPoly::FREQ_CV ) );

  xp -= SizeTable<RoundBlackKnob>::X + margin;
  addParam( ParamWidget::create< RoundBlackKnob >( Vec( xp, outy ), module,
                                                       KarplusStrongPoly::FREQ_KNOB,
                                                       -54.0f, 54.0f, 0.0f ) );



  outy += yh + 3 * margin;


  yh = SizeTable<RoundBlackSnapKnob>::Y;
  std::cout << "YH=" << yh << "\n";
  brd( yh );
  cl( "Packet", yh );

  xp = 55;

  addParam( ParamWidget::create< RoundBlackSnapKnob >( Vec( xp, outy ),
                                                       module,
                                                       KarplusStrongPoly::INITIAL_PACKET,
                                                       0,
                                                       module->getNumPackets()-1, 0 ) );


  xp += SizeTable<RoundBlackSnapKnob>::X + margin;
  addInput( Port::create<PJ301MPort>( Vec( xp, outy + diffY2c<RoundBlackSnapKnob,PJ301MPort>() ),
                                      Port::INPUT, module, KarplusStrongPoly::INITIAL_PACKET_INPUT ) );
  xp += SizeTable<PJ301MPort>::X + margin;
  addChild( DotMatrixLightTextWidget::create( Vec( xp, outy + diffY2c<RoundBlackSnapKnob,DotMatrixLightTextWidget>() ),
                                              module, 8,
                                              KarplusStrongPoly::getInitialPacketStringDirty,
                                              KarplusStrongPoly::getInitialPacketString ) );


  outy += yh + 3 * margin;

  yh = SizeTable<RoundBlackSnapKnob>::Y + SizeTable<RoundBlackKnob>::Y + margin;
  brd( yh );
  cl( "Filter", SizeTable<RoundBlackKnob>::Y );


  outy += yh + 3 * margin;
  yh = SizeTable< RoundBlackKnob >::Y;
  brd( yh );
  cl( "Atten", yh );
  xp = box.size.x - margin - obuf - SizeTable<PJ301MPort>::X;
  addInput( Port::create< PJ301MPort >( Vec( xp, outy + diffY2c< RoundBlackKnob, PJ301MPort >() ),
                                        Port::INPUT,
                                        module,
                                        KarplusStrongPoly::ATTEN_CV ) );

  xp -= SizeTable<RoundBlackKnob>::X + margin;
  addParam( ParamWidget::create< RoundBlackKnob >( Vec( xp, outy ), module,
                                                   KarplusStrongPoly::ATTEN_KNOB,
                                                   0, 1, 0.5 ) );


  outy += yh + 3 * margin;
  brd( SizeTable<PJ301MPort>::Y );
  cl( "Output", SizeTable<PJ301MPort>::Y );
  addOutput( Port::create< PJ301MPort >( Vec( box.size.x - obuf - margin - SizeTable<PJ301MPort>::X, outy ),
                                         Port::OUTPUT,
                                         module,
                                         KarplusStrongPoly::SYNTH_OUTPUT ) );

}

Model *modelKarplusStrongPoly = Model::create<KarplusStrongPoly, KarplusStrongPolyWidget>("Bacon Music", "KarplusStrongPoly", "KarplusStrongPoly", OSCILLATOR_TAG );

