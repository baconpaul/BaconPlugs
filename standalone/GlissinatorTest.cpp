#include "standalone_helpers.hpp"
#include "../src/Glissinator.hpp"
#include <iostream>

int main( int argch, char **argv )
{
  std::cout << "Yo Gliss\n";
  typedef Glissinator< StandaloneModule > G;
  

  {
    G g;
    
    g.params[ G::GLISS_TIME ].value = 0.1;
    g.inputs[ G::SOURCE_INPUT ].value = 1.00;
    g.inputs[ G::SOURCE_INPUT ].active = true;
    g.outputs[ G::SLID_OUTPUT ].active = true;
    
    std::vector< float > ov;
    for( int i=0; i<100; ++i ) {
      g.step(); ov.push_back( g.outputs[ G::SLID_OUTPUT ].value );
    }
    
    g.inputs[ G::SOURCE_INPUT ].value = 2.00;
    for( int i=0; i<engineGetSampleRate() * 0.15; ++i ) {
      g.step(); ov.push_back( g.outputs[ G::SLID_OUTPUT ].value );
    }
    
    //for( int i=0; i<ov.size(); i += 20 )
    //std::cout << ov[ i ] << "\n";
    
  }
  
  {
    G g;
    // OK so now lets test that gliss time bug
    g.params[ G::GLISS_TIME ].value = 0.1;
    g.inputs[ G::SOURCE_INPUT ].value = 1.00;
    g.inputs[ G::SOURCE_INPUT ].active = true;
    g.outputs[ G::SLID_OUTPUT ].active = true;

    std::vector< float > ov;
    for( int i=0; i<100; ++i ) {
      g.step(); ov.push_back( g.outputs[ G::SLID_OUTPUT ].value );
    }

    g.inputs[ G::SOURCE_INPUT ].value = 2.00;
    for( int i=0; i<engineGetSampleRate() * 0.07; ++i ) {
      g.step(); ov.push_back( g.outputs[ G::SLID_OUTPUT ].value );
    }
    
    std::cout << "OT: " << g.offsetCount << "\n";
    g.params[ G::GLISS_TIME ].value = 0;

    for( int i=0; i<20; ++i )
      g.step();
    std::cout << "OT: " << g.offsetCount << "\n";
    std::cout << "I GOT 2? " <<  g.outputs[ G::SLID_OUTPUT ].value << "\n";
    

  }

}
