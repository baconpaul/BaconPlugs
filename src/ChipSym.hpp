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

  public:
    NESBase( float imin, float imax )
      :
      wfMin( imin ), wfMax( imax )
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
  protected:
    float waveForm[ 32 ];
    CPUStepper cpu;


  public:
    NESTriangle( float imin, float imax, uint sampleRate )
      :
      NESBase( imin, imax ), cpu( sampleRate, NESNTSCCPURate )
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
          if( currPos >= 32 ) currPos = 0;
        }
      
      return waveForm[ currPos ] * wfMinToMax - wfMin;
    }

    void setWavelengthInSeconds( float seconds )
    {
      setDigWavelength( (uint)( seconds * NESNTSCCPURate * 1000 * 1000 / 32 ) );
    }
  };

  class NESArbitraryWaveform : public NESTriangle {
  public:
    NESArbitraryWaveform( float imin, float imax, uint sampleRate ) : NESTriangle( imin, imax, sampleRate ) { }

    void setWaveformPoint( uint pos,  // 0->31
                           uint val ) // 0->15
    {
      waveForm[ pos  ] = val;
    }
  };

  class NESPulse : public NESBase // http://wiki.nesdev.com/w/index.php/APU_Pulse
  {
  private:
    int dutyCycle;
    float **waveForms;
    int nDutyCycles;
    int wfLength;
    CPUStepper cpu;

    
  public:
    NESPulse( float imin, float imax, int sampleRate )
      :
      NESBase( imin, imax ), cpu( sampleRate, NESNTSCCPURate / 2 )
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

    void setWavelengthInSeconds( float seconds )
    {
      setDigWavelength( (uint)( seconds * NESNTSCCPURate * 1000 * 1000 / 2.0 / 8.0) );
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
          if( currPos >= wfLength  ) currPos = 0;
        }
      
      return waveForms[ dutyCycle ][ currPos ] * wfMinToMax - wfMin;
    }
  };

  class NESNoise : public NESBase
  {
  private:
    CPUStepper cpu;
    short shiftRegister;
    short currentOutput;
    short xorBit;
    
  public:

    NESNoise( float imin, float imax, int sampleRate )
      :
      NESBase( imin, imax ), cpu( sampleRate, NESNTSCCPURate / 2 )
    {
      setPeriod( 8 );
      shiftRegister = 0x07;
      currentOutput = shiftRegister & 1;
      xorBit = 1;
    }

    void setModeFlag( bool mf )
    {
      if( mf ) xorBit = 6;
      else xorBit = 1;
    }
    void setPeriod( uint c ) // 0 - 15
    {
      if( c > 15 ) c = 8;
      switch( c ) {
      case 0:
        digWavelength = 4; break;
      case 1:
        digWavelength = 8; break;
      case 2:
        digWavelength = 16; break;
      case 3:
        digWavelength = 32; break;
      case 4:
        digWavelength = 64; break;
      case 5:
        digWavelength = 96; break;
      case 6:
        digWavelength = 128; break;
      case 7:
        digWavelength = 160; break;
      case 8:
        digWavelength = 202; break;
      case 9:
        digWavelength = 254; break;
      case 10:
        digWavelength = 380; break;
      case 11:
        digWavelength = 508; break;
      case 12:
        digWavelength = 762; break;
      case 13:
        digWavelength = 1016; break;
      case 14:
        digWavelength = 2034; break;
      case 15:
        digWavelength = 4068; break;
      }
    }

    float step()
    {
      int ticks = cpu.nextStepCPUTicks();
      t -= ticks;
      if( t < 0 )
        {
          t += digWavelength;

          // Do the LFSR Shift
          short bit  = ((shiftRegister >> 0) ^ (shiftRegister >> xorBit)) & 1;
          shiftRegister =  (shiftRegister >> 1) | (bit << 13);

          currentOutput = shiftRegister & 1;
        }
      
      return currentOutput * wfMinToMax - wfMin;

    }
  };

  class LFSRGeneralImpl
  {
  public:
    typedef std::bitset< 24 > bits;
  private:
  public:
    void setActivetBits( size_t aBits ) // < 24 please
    {
    }
    void setTapsAsInt( uint taps ) // so 1 << 16 & 1 << 14 & 1 << 7 type thing
    {
    }
    
  };
};
