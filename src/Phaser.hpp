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

class Phaser
{
};
