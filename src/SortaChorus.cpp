
#include "BaconPlugs.hpp"

struct SortaChorus : virtual Module {
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

  SortaChorus() : Module( NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS )
  {
  }

  void step() override
  {
  }
};

struct SortaChorusWidget : ModuleWidget {
  SortaChorusWidget( SortaChorus *module);
};

SortaChorusWidget::SortaChorusWidget( SortaChorus *module ) : ModuleWidget( module )
{
  box.size = Vec( SCREW_WIDTH * 8, RACK_HEIGHT );

  BaconBackground *bg = new BaconBackground( box.size, "SortaChorus" );
  addChild( bg->wrappedInFramebuffer());
}

Model *modelSortaChorus = Model::create<SortaChorus, SortaChorusWidget>("Bacon Music", "SortaChorus", "SortaChorus", CHORUS_TAG );
