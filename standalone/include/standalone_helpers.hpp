
#include "RtAudio.h"
#include <unistd.h>

float engineGetSampleRate() { return 44100; }
float engineGetSampleTime() { return 1.0f / engineGetSampleRate(); }

struct StepHandler
{
  virtual ~StepHandler() { };
  virtual int dostep( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
                      double streamTime, RtAudioStreamStatus status ) = 0;

  static int step( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
                   double streamTime, RtAudioStreamStatus status, void* userData )
  {
    StepHandler *sh = (StepHandler *)userData;
    return sh->dostep( outputBuffer, inputBuffer, nBufferFrames, streamTime, status );
  }

  RtAudio startDac()
  {
    RtAudio dac;
    if ( dac.getDeviceCount() < 1 ) {
      std::cout << "\nNo audio devices found!\n";
      exit( 0 );
    }
    
    RtAudio::StreamParameters parameters;
    parameters.deviceId = dac.getDefaultOutputDevice();
    parameters.nChannels = 1;
    parameters.firstChannel = 0;
    
    unsigned int sampleRate = 44100;
    unsigned int bufferFrames = 256; // 256 sample frames

    try {
      dac.openStream( &parameters, NULL, RTAUDIO_FLOAT64,
                      sampleRate, &bufferFrames, &StepHandler::step, (void *)this );
      dac.startStream();
    }
    catch ( RtAudioError& e ) {
      e.printMessage();
      exit( 0 );
    }
    return dac;
  }

  void stopDac( RtAudio dac )
  {
    try {
      // Stop the stream
      dac.stopStream();
    }
    catch (RtAudioError& e) {
      e.printMessage();
    }
    if ( dac.isStreamOpen() ) dac.closeStream();
  }

  int playAudioUntilStepsDone()
  {
    RtAudio dac = startDac();
    
    while( dac.isStreamRunning() )
      {
        usleep( 100 );
      }

    if ( dac.isStreamOpen() ) dac.closeStream();

    return 0;
  }
  
  int playAudioUntilEnterPressed()
  {
    RtAudio dac = startDac();
    
    char input;
    std::cout << "\nPlaying ... press <enter> to quit.\n";
    std::cin.get( input );

    stopDac( dac );
    
    return 0;
  }
};

struct BufferPlayer : StepHandler
{
  float *buf;
  int nsamps;
  int pos;

  
  BufferPlayer( float *_buf, int _nsamps )
    :
    StepHandler(),
    buf( _buf ),
    nsamps( _nsamps )
    // does not take a copy. Please dont' delete
  {
    pos = 0;
  }

  virtual int dostep( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
                      double streamTime, RtAudioStreamStatus status ) override
  {
    unsigned int i;
    double *buffer = (double *) outputBuffer;

    for ( i=0; i<nBufferFrames && pos < nsamps; i++ ) {
      *buffer++ = buf[ pos++ ];
    }
    if( pos >= nsamps )
      return 1;
    return 0;
    
  }
};

void playBuffer( float *b, size_t n )
{
  BufferPlayer bp( b, n );
  bp.playAudioUntilStepsDone();
}

void playBuffer( std::vector< float > b )
{
  playBuffer( b.data(), b.size() );
}

  
struct StandaloneModule
{
  struct thing
  {
    float value;
    bool active;
  };

  typedef std::vector< thing > values_t;
  typedef std::vector< values_t > results_t;

  void multiStep( size_t stepCount, results_t &into )
  {
    for( size_t i=0; i<stepCount; ++i )
      {
        step();
        into.push_back( outputs );
      }
  }
  
  values_t params;
  values_t lights;
  values_t inputs;
  values_t outputs;

  StandaloneModule( int nparam, int ninp, int nout, int nlight )
  {
    params.resize( nparam );
    lights.resize( nlight );
    inputs.resize( ninp );
    outputs.resize( nout );
  }

  
  virtual void step() { };
};

void fadeIn( float* samp, int ns )
{
  int n = 4000;
  for( auto i=0; i<n; ++i )
    {
      samp[ i ] *= i * 1.0 / n;
      samp[ ns - 1 - i ] *= i * 1.0 / n;
    }
}
