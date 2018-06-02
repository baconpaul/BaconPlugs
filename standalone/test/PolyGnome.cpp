#include "standalone_helpers.hpp"
#include "PolyGnome.hpp"

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

typedef PolyGnome< StandaloneModule > P;

TEST_CASE( "Polygnome main clock beats regularly" )
{
  P p;
  p.step();

  REQUIRE( p.outputs[ P::CLOCK_GATE_0 ].value > 5 );
};


