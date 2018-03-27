
#include "RtAudio.h"

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

  int playAudioUntilEnterPressed()
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
  
    char input;
    std::cout << "\nPlaying ... press <enter> to quit.\n";
    std::cin.get( input );
    try {
      // Stop the stream
      dac.stopStream();
    }
    catch (RtAudioError& e) {
      e.printMessage();
    }
    if ( dac.isStreamOpen() ) dac.closeStream();
    return 0;
  }
};

