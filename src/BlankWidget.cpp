#if 0

#include "BaconPlugs.hpp"

struct MODULE_NAME : virtual Module {
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

  MODULE_NAME() : Module( NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS )
  {
  }

  void step() override
  {
  }
};

WIDGET_NAME::WIDGET_NAME()
{
  MODULE_NAME *module = new MODULE_NAME();
  setModule( module );
  box.size = Vec( SCREW_WIDTH * 8, RACK_HEIGHT );

  BaconBackground *bg = new BaconBackground( box.size, "MODULE_NAME" );
  addChild( bg->wrappedInFramebuffer());
}
#endif
