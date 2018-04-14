
#include "BaconPlugs.hpp"
#include <sstream>

struct KarplusStrongPoly : virtual Module {
  enum ParamIds {
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

  KarplusStrongPoly() : Module( NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS )
  {
    filterString = std::string( "First time around" );
    filterStringDirty = true;
    dumbcount = 0;
  }

  void step() override
  {
    dumbcount ++;
    if( dumbcount % 44100 == 0 )
      {
        filterStringDirty = true;
        std::ostringstream os;
        os << "BLIMP " << dumbcount;
        filterString = os.str();
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

  addChild( DotMatrixLightTextWidget::create( Vec( 20, 40 ), module, 15, KarplusStrongPoly::getFilterStringDirty, KarplusStrongPoly::getFilterString ) );
}

Model *modelKarplusStrongPoly = Model::create<KarplusStrongPoly, KarplusStrongPolyWidget>("Bacon Music", "KarplusStrongPoly", "KarplusStrongPoly", OSCILLATOR_TAG );

