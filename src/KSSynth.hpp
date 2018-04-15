// An implementation of the KSSYnth class in the ipython notebook in ../pynb/KarplusStrong.ipynb

#include <string>
#include <vector>
#include <iostream>

class KSSynth {
public:
  typedef enum InitPacket {
    RANDOM,
    SQUARE,
    SAW,
    NOISYSAW,
    SIN,
    SINCHIRP // if you add remember to fix the count below...
  } InitPacket;

  std::string initPacketName( InitPacket p )
  {
    switch(p)
      {
      case RANDOM: return "random";
      case SQUARE: return "square";
      case SAW: return "saw";
      case NOISYSAW: return "noisysaw";
      case SIN: return "sin";
      case SINCHIRP: return "sinchirp";
      }
  }

  int numInitPackets()
  {
    return (int)SINCHIRP + 1;
  }

  typedef enum FilterType {
    WEIGHTED_ONE_SAMPLE
  } FilterType;

  std::string filterTypeName( FilterType p )
  {
    switch(p)
      {
      case WEIGHTED_ONE_SAMPLE: return "wgt 1samp";
      }
  }

  int numFilterTypes()
  {
    return WEIGHTED_ONE_SAMPLE + 1;
  }

private:
  // here's my interior state
  InitPacket packet;
  FilterType filter;

  float filtParamA, filtParamB, filtParamC, filtAtten, filtAttenScaled;

  float freq;
  int burstLen;
  int sampleRate;

  long pos;
  std::vector< float > delay;

  float wfMin, wfMax;

public:

  KSSynth( int sampleRateIn, float minv, float maxv )
    :
    sampleRate( sampleRateIn ), wfMin( minv ), wfMax( maxv ),
    packet( RANDOM ),
    filter( WEIGHTED_ONE_SAMPLE ),
    filtAtten( 0.1f ),
    filtParamA( 0.5f ),
    filtParamB( 0.0f ),
    filtParamC( 0.0f ),
    pos( 0 )
  {
    setFreq( 220 );
  }

  void setFreq( float f )
  {
    freq = f;
    burstLen = (int)( ( sampleRate / freq + 0.5 ) * 2 );
    filtAttenScaled = filtAtten / 100 / ( freq / 440 );
    delay.resize( burstLen );
  }

  void trigger( float f )
  {
    setFreq( f );
    switch( packet )
      {
      case RANDOM:
        for( int i=0; i<burstLen; ++i )
          {
            delay[ i ] = rand() * 1.0f / RAND_MAX;
          }
        break;
      default:
        // BLOW UP (so remove this later)
        break;
      }
  }
  
  int step()
  {
    int dpos = pos % burstLen;
    int dpnext = ( pos + 1 ) % burstLen;
    int dpfill = ( pos - 1 ) % burstLen;
    pos++;

    float filtval;
    switch( filter )
      {
      case WEIGHTED_ONE_SAMPLE:
        float ftw = filtParamA;
        float fta = filtAttenScaled;
        filtval = ( ftw * delay[ dpos ] + ( 1.0 - ftw ) * delay[ dpnext ] ) * ( 1.0 - fta );
        break;
      }
    delay[ dpfill ] = filtval;
    return filtval * ( wfMax - wfMin ) - wfMin;
  }
  
};
