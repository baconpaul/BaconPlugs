#include "standalone_helpers.hpp"
#include "../src/SampleDelay.hpp"
#include <iostream>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"


typedef SampleDelay< StandaloneModule > SD;

TEST_CASE( "Sample Delay delays by given amount" )
{
  SD sd;
  int dk = 37;
  sd.params[ SD::DELAY_KNOB ].value = dk;
  sd.inputs[ SD::SIGNAL_IN ].active = 1;
  
  SD::results_t ov;
  SD::results_t iv;
  
  int ns = 1000;
  for( int i=0; i<ns; ++i )
    {
      sd.inputs[ SD::SIGNAL_IN ].value = i * 1.0f / ns;
      sd.step();

      iv.push_back( sd.inputs );
      ov.push_back( sd.outputs );
    }
  for( int i=dk+1; i<ns; ++i )
    // this "i-1" vs "i" is because every rack adds a one sample diff. So if dk == 0 then we are off-by-one
    REQUIRE( ov[ i-1 ][ SD::SIGNAL_OUT ].value == iv[ i-dk ][ SD::SIGNAL_IN ].value );
  
}
