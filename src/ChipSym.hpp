/*
** All my chip simulator noobits. But remember this is just me screwing around.
*/

#include <math.h>
#include <iostream>

namespace ChipSym
{
  class CPUStepper
  {
  private:
    int sampleRateInHz;
    double chipFrequencyInMHZ;

    unsigned int ticksPerSample;
    double tickFractionPerSample;
    double accruedTickFraction;
    
  public:
    CPUStepper( unsigned int _sampleRateInHz, double _chipFrequencyInMHZ )
      : sampleRateInHz( _sampleRateInHz ), chipFrequencyInMHZ( _chipFrequencyInMHZ ), accruedTickFraction( 0 )
    {
      double tpsD = chipFrequencyInMHZ * 1000000 / sampleRateInHz;
      double tpsdi;
      tickFractionPerSample = modf( tpsD, &tpsdi );
      ticksPerSample = (unsigned int)tpsdi;
    }

    /* 
    ** Take one step and tell me how many CPU ticks I would have seen if I am sampling at sampleRate.
    ** This won't be a constant of course since sometimes we get an extra to catch up
    */
    
    unsigned int nextStepCPUTicks()
    {
      accruedTickFraction += tickFractionPerSample;
      if( accruedTickFraction > 1 )
        {
          accruedTickFraction -= 1;
          return ticksPerSample + 1;
        }

      return ticksPerSample;
    }
  };

  static double NESNTSCCPURate = 1.789773;


  class NESBase
  {
  protected:
    int digWavelength; // this is the 2^11 which sets frequency time
    int t, currPos;

    float wfMin, wfMax, wfMinToMax;

    CPUStepper cpu;

  public:
    NESBase( float imin, float imax, uint sampleRate )
      :
      wfMin( imin ), wfMax( imax ), cpu( sampleRate, NESNTSCCPURate )
    {
      digWavelength = 1 << 7; // Callibrate this later
      t = digWavelength;
      currPos = 0;
      
      wfMinToMax = wfMax - wfMin;
    }

    void setDigWavelength( int df ) // 0 -> 2^11
    {
      digWavelength = df;
    }
  };
  
  class NESTriangle : public NESBase // http://wiki.nesdev.com/w/index.php/APU_Triangle
  {
  private:
    float waveForm[ 32 ];

  public:
    NESTriangle( float imin, float imax, uint sampleRate )
      :
      NESBase( imin, imax, sampleRate )
    {
      for( int i=0; i<15; ++i ) {
        waveForm[ 15 - i ] = i / 15.0f;
        waveForm[ 16 + i ] = i / 15.0f;
      }
    }

    float step()
    {
      int ticks = cpu.nextStepCPUTicks();
      t -= ticks;
      if( t < 0 )
        {
          currPos ++;
          t += digWavelength;
          if( currPos > 32 ) currPos = 0;
        }
      
      return waveForm[ currPos ] * wfMinToMax - wfMin;
    }
  };

  class NESPulse : public NESBase // http://wiki.nesdev.com/w/index.php/APU_Pulse
  {
  private:
    int dutyCycle;
    float **waveForms;
    int nDutyCycles;
    int wfLength;
    
  public:
    NESPulse( float imin, float imax, int sampleRate )
      :
      NESBase( imin, imax, sampleRate )
    {
      wfLength = 8;
      nDutyCycles = 4;
      dutyCycle = 1;
      
      waveForms = new float*[ 4 ];
      for( int i=0; i<nDutyCycles; ++i )
        {
          waveForms[ i ] = new float[ wfLength ];
          for (int j=0; j<wfLength; ++j ) waveForms[ i ][ j ] = ( i == nDutyCycles - 1 ) ? 1 : 0;

          // Really, read that website for this stuff.
          switch( i )
            {
            case 0:
              waveForms[ i ][ 1 ] = 1;
              break;
            case 1:
              waveForms[ i ][ 1 ] = 1;
              waveForms[ i ][ 2 ] = 1;
              break;
            case 2:
              waveForms[ i ][ 1 ] = 1;
              waveForms[ i ][ 2 ] = 1;
              waveForms[ i ][ 3 ] = 1;
              waveForms[ i ][ 4 ] = 1;
              break;
            case 3:
              waveForms[ i ][ 1 ] = 0;
              waveForms[ i ][ 2 ] = 0;
              break;
            }
        }

    }
    
    void setDutyCycle( int dc )
    {
      dutyCycle = dc;
    }
    
    float step()
    {
      int ticks = cpu.nextStepCPUTicks();
      t -= ticks;
      if( t < 0 )
        {
          currPos ++;
          t += digWavelength;
          if( currPos > wfLength  ) currPos = 0;
        }
      
      return waveForms[ dutyCycle ][ currPos ] * wfMinToMax - wfMin;
    }
    

  };
};
