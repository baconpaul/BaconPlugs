#include "standalone_helpers.hpp"
#include "Glissinator.hpp"
#include <iostream>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

typedef Glissinator< StandaloneModule > G;

TEST_CASE( "Simple increase and decrease stays bounded" )
{
  for( int io = 0; io <= 1; ++io )
    {
      G g;
        
      g.params[ G::GLISS_TIME ].value = 0.1;
      g.inputs[ G::SOURCE_INPUT ].value = 1.00 + io;
      g.inputs[ G::SOURCE_INPUT ].active = true;
      g.outputs[ G::SLID_OUTPUT ].active = true;
        
      G::results_t ov;
      g.multiStep( 100, ov );
        
        
      g.inputs[ G::SOURCE_INPUT ].value = 2.00 - io;
      g.multiStep( engineGetSampleRate() * 0.15, ov );
        
      // So this should be monotonically increasing.
      auto hd = ov.begin() + 1, pv = ov.begin();
      while( hd != ov.end() )
        {
          if( io == 0 )
            REQUIRE( (*hd)[ G::SLID_OUTPUT ].value >= (*pv)[ G::SLID_OUTPUT ].value );
          else
            REQUIRE( (*hd)[ G::SLID_OUTPUT ].value <= (*pv)[ G::SLID_OUTPUT ].value );
            
          REQUIRE( (*hd)[ G::SLID_OUTPUT ].value >= 1 );
          REQUIRE( (*hd)[ G::SLID_OUTPUT ].value <= 2 );
          ++hd;
          ++pv;
        }
    }
};

TEST_CASE( "Turnaround half way through doesn't unbound us" )
{
  // The turnadound-half-way-through test
  for( int io = 0; io <= 1; ++io )
    {
      G g;
      
      g.params[ G::GLISS_TIME ].value = 0.1;
      g.inputs[ G::SOURCE_INPUT ].value = 1.00 + io;
      g.inputs[ G::SOURCE_INPUT ].active = true;
      g.outputs[ G::SLID_OUTPUT ].active = true;
      
      G::results_t ov;
      g.multiStep( 100, ov );
        
        
      g.inputs[ G::SOURCE_INPUT ].value = 2.00 - io;
      g.multiStep( engineGetSampleRate() * 0.07, ov );
        
      float maxO = g.outputs[ G::SLID_OUTPUT ].value;
      g.inputs[ G::SOURCE_INPUT ].value = 1.00 + io;
      g.multiStep( engineGetSampleRate() * 0.07, ov );
        
      // So this should no longer be monotonic strictly but should monotone up to max
      // and then down from
      bool goingUp = (io == 0)?true:false;
      bool hitMax = false;
      auto hd = ov.begin() + 1, pv = ov.begin();
      while( hd != ov.end() )
        {
          if( goingUp )
            REQUIRE( (*hd)[ G::SLID_OUTPUT ].value >= (*pv)[ G::SLID_OUTPUT ].value );
          else
            REQUIRE( (*hd)[ G::SLID_OUTPUT ].value <= (*pv)[ G::SLID_OUTPUT ].value );
            
          if( io == 0 )
            {
              REQUIRE( (*hd)[ G::SLID_OUTPUT ].value >= 1 );
              REQUIRE( (*hd)[ G::SLID_OUTPUT ].value <= maxO );
            }
          else
            {
              REQUIRE( (*hd)[ G::SLID_OUTPUT ].value <= 2 );
              REQUIRE( (*hd)[ G::SLID_OUTPUT ].value >= maxO );
            }
            
          if( (*hd)[G::SLID_OUTPUT].value == maxO && ! hitMax ) { goingUp = ! goingUp; hitMax = true; } // max can repeat
            
          ++hd;
          ++pv;
        }
    }
};

TEST_CASE( "Gliss time changes underneath us" )
{
  G g;
  // OK so now lets test that gliss time bug. If we reset the gliss time most of the way through
  // a gliss, the 0.6.1 version runs away.
  g.params[ G::GLISS_TIME ].value = 0.1;
  g.inputs[ G::SOURCE_INPUT ].value = 1.00;
  g.inputs[ G::SOURCE_INPUT ].active = true;
  g.outputs[ G::SLID_OUTPUT ].active = true;
    
  G::results_t ov;
  g.multiStep( 100, ov );
    
  g.inputs[ G::SOURCE_INPUT ].value = 2.00 ;
  g.multiStep( engineGetSampleRate() * 0.07, ov );
    
  g.params[ G::GLISS_TIME ].value = 0;
    
  g.multiStep( 1, ov );
    
  auto hd = ov.begin() + 1, pv = ov.begin();
  while( hd != ov.end() )
    {
      REQUIRE( (*hd)[ G::SLID_OUTPUT ].value >= (*pv)[ G::SLID_OUTPUT ].value );
      REQUIRE( (*hd)[ G::SLID_OUTPUT ].value >= 1 );
      REQUIRE( (*hd)[ G::SLID_OUTPUT ].value <= 2 );
      ++hd;
      ++pv;
    }
};

