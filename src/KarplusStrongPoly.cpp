
#include "BaconPlugs.hpp"
#include <sstream>
#include <vector>
#include <string>

struct KarplusStrongPoly : virtual Module {
  enum ParamIds {
    INITIAL_PACKET,
    NUM_PARAMS
  };

  enum InputIds {
    NUM_INPUTS
  };

  enum OutputIds {
    NUM_OUTPUTS
  };

  enum LightIds {
    NUM_LIGHTS
  };

  std::vector< std::string > initialPackets;
  KarplusStrongPoly() : Module( NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS )
  {
    filterString = std::string( "First time around" );
    filterStringDirty = true;
    dumbcount = 0;

    initialPackets.push_back( "random" );
    initialPackets.push_back( "square" );
    initialPackets.push_back( "saw" );
    initialPackets.push_back( "noisysaw" );
    initialPackets.push_back( "sin" );
    initialPackets.push_back( "sinchirp" );
    currentInitialPacket = -1;
  }

  int getNumPackets() { return initialPackets.size(); }
  int currentInitialPacket;
  
  void step() override
  {
    if( (int)( params[ INITIAL_PACKET ].value ) != currentInitialPacket )
      {
        filterStringDirty = true;
        currentInitialPacket = params[ INITIAL_PACKET ].value;
        filterString = initialPackets[ currentInitialPacket ];
      }
  }

  bool filterStringDirty;
  std::string filterString;
  int dumbcount;
  
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

  addParam( ParamWidget::create< RoundSmallBlackKnob >( Vec( 20, 40 ), module, KarplusStrongPoly::INITIAL_PACKET, 0, module->getNumPackets()-1, 0 ) );
  addChild( DotMatrixLightTextWidget::create( Vec( 50, 42 ), module, 8, KarplusStrongPoly::getFilterStringDirty, KarplusStrongPoly::getFilterString ) );
}

Model *modelKarplusStrongPoly = Model::create<KarplusStrongPoly, KarplusStrongPolyWidget>("Bacon Music", "KarplusStrongPoly", "KarplusStrongPoly", OSCILLATOR_TAG );

