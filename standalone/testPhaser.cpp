#include "standalone_helpers.hpp"
#include <iostream>
#include <math.h>
#include "../src/Phaser.hpp"

int main( int argc, char **argv )
{
  float sinb[ 100000 ];
  float sqrb[ 100000 ];
  float filb[ 100000 ];
  float lfo[ 10000 ];
  int ns = 100000;


  for( int i=0; i<ns; ++i )
    {
      auto freq = 440; // + 220.0 * i / ns;;
      float t = i / 44100.0;
      sinb[ i ] = sin( t * 2.0 * 3.14 * freq );
      lfo[ i ] = sin( t * 2.0 * 3.14 * 2 );
      if( sinb[ i ] > 0 ) sqrb[ i ] = 1.0; else sqrb[ i ] = -1.0;
    }
  fadeIn( sinb, ns );
  fadeIn( sqrb, ns );

  Phaser p;
  float res[ 10000 ];
  for( int i=0; i<ns; ++i )
    {
      res[ i ] = p.process( sqrb[ i ], lfo [i ] );
    }
  {
    BufferPlayer pl( sqrb, ns );
    pl.playAudioUntilStepsDone();
  }
  {
    BufferPlayer pl( res, ns );
    pl.playAudioUntilStepsDone();
  }

  return 0;
}
