template< class TBase >
struct GenericLFSR : TBase
{
  enum ParamIds {
    SEED_LSB,
    
    NUM_PARAMS = SEED_LSB + 4
  };

  enum InputIds {
    NUM_INPUTS
  };

  enum OutputIds {
    NUM_OUTPUTS
  };

  enum LightIds {
    SEED_LIGHT_LSB,
    
    NUM_LIGHTS = SEED_LIGHT_LSB + 4
  };
  
  using TBase::params;
  using TBase::inputs;
  using TBase::outputs;
  using TBase::lights;

  GenericLFSR() : TBase( NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS )
  {
  }

  void step() override
  {
    lights[ SEED_LIGHT_LSB ].value = params[ SEED_LSB ].value;
  }

};
