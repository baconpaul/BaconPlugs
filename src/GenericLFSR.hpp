template< class TBase >
struct GenericLFSR : TBase
{
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
  
  using TBase::params;
  using TBase::inputs;
  using TBase::outputs;
  using TBase::lights;

  GenericLFSR() : TBase( NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS )
  {
  }

  void step() override
  {
  }

};
