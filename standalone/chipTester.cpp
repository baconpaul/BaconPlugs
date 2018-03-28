#include "../src/ChipSym.hpp"
#include "standalone_helpers.hpp"

template <typename T>
struct NESGen : StepHandler
{
  T ngen;
  NESGen() : ngen( -0.2, 0.2, 44100 ) {}

  virtual int dostep( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
                      double streamTime, RtAudioStreamStatus status ) override
  {
    unsigned int i, j;
    double *buffer = (double *) outputBuffer;

    for ( i=0; i<nBufferFrames; i++ ) {
      *buffer++ = ngen.step();
    }
    return 0;
  }
};

int main( int argc, char** argv )
{
  ChipSym::CPUStepper cp( 44100, 1.789773 );

  std::cout << "Do some NES CPU Tick pulls at 44.1k\n";
  for( uint i=0; i<5; ++i )
    {
      std::cout << i * 10 << "\t";
      for( uint j=0; j<10; ++j )
        {
          std::cout << cp.nextStepCPUTicks() << "  ";
        }
      std::cout << "\n";
    }

  std::cout << "Running the NES tri gen\n";
  NESGen< ChipSym::NESTriangle > triGen;
  int df = 2<<8;
  triGen.ngen.setDigWavelength( df );
  triGen.playAudioUntilEnterPressed();

  triGen.ngen.setDigWavelength( df / 2 );
  triGen.playAudioUntilEnterPressed();

  
  NESGen< ChipSym::NESPulse > pulsGen;
  df = 2<<9;
  pulsGen.ngen.setDigWavelength( df );
  pulsGen.playAudioUntilEnterPressed();

  pulsGen.ngen.setDigWavelength( df / 2 );
  pulsGen.playAudioUntilEnterPressed();


  return 0;
}
