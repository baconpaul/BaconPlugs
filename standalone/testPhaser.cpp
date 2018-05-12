#include "standalone_helpers.hpp"
#include <iostream>
#include <math.h>
#include "../src/Phaser.hpp"

int main( int argc, char **argv )
{
  float sinb[ 100000 ];
  float sqrb[ 100000 ];
  float filb[ 100000 ];
  int ns = 100000;


  for( int i=0; i<ns; ++i )
    {
      auto freq = 220 + 220.0 * i / ns;;
      float t = i / 44100.0;
      sinb[ i ] = sin( t * 2.0 * 3.14 * freq );
      if( sinb[ i ] > 0 ) sqrb[ i ] = 1.0; else sqrb[ i ] = -1.0;
    }
  fadeIn( sinb, ns );
  fadeIn( sqrb, ns );

  AllPassFilter apf;
  apf.setRadial( 0.98, 0.04 );
  for( int i=0; i<ns; ++i )
    {
      filb[ i ] = apf.process( sinb[ i ] );
    }

  {
    BufferPlayer bp( sinb, ns );
    bp.playAudioUntilStepsDone();
  }

  {
    BufferPlayer bp( filb, ns );
    bp.playAudioUntilStepsDone();
  }

  for( int i=0; i<ns; ++i )
    {
      filb[ i ] = 0.5 * filb[ i ] + 0.5 * sinb[ i ];
    }
  
  {
    BufferPlayer bp( filb, ns );
    bp.playAudioUntilStepsDone();
  }

  return 0;
}
