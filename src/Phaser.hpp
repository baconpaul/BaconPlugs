#include <math.h>
#include <vector>

class AllPassFilter
{
  // https://en.wikipedia.org/wiki/All-pass_filter
private:
  float inp[ 2 ], outp[ 2 ];
  float zR, zI, mz2;

public:
  AllPassFilter()
  {
    for( auto i=0; i<2; ++i )
      {
        inp[ i ] = 0;
        outp[ i ] = 0;
      }
    setRadial( 0.98, 0 );
  }

  void setRadial( float r, float th )
  {
    zR = r * cos( th );
    zI = r * sin( th );
    mz2 = r * r;
  }

  void setComplex( float _zR, float _zI )
  {
    zR = _zR;
    zI = _zI;
    mz2 = zR * zR + zI * zI;
  }
  
  float process( float ins )
  {
    float out = 0;
        
    out = ( mz2 * ( ins - outp[ 1 ] ) 
            - 2 * zR * ( inp[ 0 ] - outp[ 0 ] ) 
            + inp[ 1 ] );
    outp[ 1 ] = outp[ 0 ];
    outp[ 0 ] = out;
    inp[ 1 ] = inp[ 0 ];
    inp[ 0 ] = ins;
    
    return out;
  }
};

struct Phaser
{
  // This phaser class has an EXTERNAL LFO and processes the input and the LFO as two signals to do phasing
  float depth; // 0 -> 1
  int   nfilters;

  float r0, dr, theta0, dtheta;

  std::vector< AllPassFilter > filters;
  
  Phaser( )
    :
    nfilters( 6 )
  {
    for( auto i =0; i<nfilters; ++i )
      filters.push_back( AllPassFilter() );

    dtheta = 3.14159265 / 2.0 / ( nfilters + 1 );
    theta0 = dtheta * 0.5;
    r0 = 0.92;
    dr = 0.03;
    depth = 1.0;
  }

  float process( float input, float lfo )
  {
    float filtV = input;
    size_t i = 0;

    for( auto &f : filters )
      {
        f.setRadial( r0 + dr * lfo, ( i + lfo ) * dtheta + theta0 );
        filtV = f.process( filtV );
        ++i;
      }

    return 0.5 * ( input + depth * filtV );
  }

  struct SimpleInternalLFO
  {
    float phase = 0.0f;
    float freq = 1.0f;
    
    void setFreq( float f ) {
      freq = f;
    }
    
    float step( float dt )
    {
      phase += freq * dt;
      if( phase > 1.0f )
        phase -= 1.0f;
      return sin( 2 * M_PI * phase );
    }

  };
};

template <typename TBase>
struct PhaserModule : public TBase {
  using TBase::params;
  using TBase::inputs;
  using TBase::outputs;
  using TBase::lights;

  Phaser p;
  Phaser::SimpleInternalLFO l;

  enum ParamIds {
    NUM_PARAMS
  };

  enum InputIds {
    SIGNAL_IN,
    LFO_IN,
    NUM_INPUTS
  };

  enum OutputIds {
    PHASED,
    NUM_OUTPUTS
  };

  enum LightIds {
    NUM_LIGHTS
  };

  PhaserModule() : TBase( NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS )
  {
  }

  void step() override
  {
    float lfov = l.step( engineGetSampleTime() );
    float res = p.process( inputs[ SIGNAL_IN ].value, lfov );
    outputs[ PHASED ].value = res;
  }
};
