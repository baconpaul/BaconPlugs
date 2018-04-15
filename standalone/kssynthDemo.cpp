#include "../src/KSSynth.hpp"
#include "standalone_helpers.hpp"

#include <iostream>

struct KSGen : StepHandler
{
  KSSynth s;
  KSGen() : s( 44100, -0.9, 0.9 ) { }

  virtual int dostep( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
                      double streamTime, RtAudioStreamStatus status ) override
  {
    unsigned int i, j;
    double *buffer = (double *) outputBuffer;

    for ( i=0; i<nBufferFrames; i++ ) {
      *buffer++ = s.step();
    }
    return 0;
    
  }
};

int main( int argc, char **argv )
{
  KSGen gen;
  
  std::cout << "packets " << gen.s.numInitPackets() << "\n";

  gen.s.trigger( 440 );
  gen.playAudioUntilEnterPressed();
}
