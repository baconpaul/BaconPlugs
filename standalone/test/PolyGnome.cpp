#include "standalone_helpers.hpp"
#include "PolyGnome.hpp"

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

typedef PolyGnome< StandaloneModule > P;

TEST_CASE( "Polygnome main clock beats regularly" )
{
  P p;
  p.inputs[ P::CLOCK_INPUT ].value = 0;
  p.params[ P::CLOCK_PARAM ].value = 6.0f;
  
  p.step();

  CHECK( p.outputs[ P::CLOCK_GATE_0 ].value > 5 );
  std::vector< int > maxCounts;
  for( int i=0; i<20; ++i )
    {
      int countOn = 0;

      while( p.outputs[ P::CLOCK_GATE_0 ].value > 5 )
        {
          countOn ++;
          p.step();
        }

      maxCounts.push_back( countOn );
      
      while( p.outputs[ P::CLOCK_GATE_0 ].value < 5 )
        {
          countOn --;
          p.step();
        }

      CHECK( countOn <= 1);
      CHECK( countOn >= -1 );
    }

  for( auto i=1; i<maxCounts.size(); ++i )
    CHECK( abs( maxCounts[ i ] - maxCounts[ i - 1 ] ) <= 1 );
};


