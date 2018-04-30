#include "standalone_helpers.hpp"
#include "../src/Glissinator.hpp"
#include <iostream>

int main( int argch, char **argv )
{
  std::cout << "Yo Gliss\n";
  typedef Glissinator< StandaloneModule > G;
  
  G g;

  g.inputs[ G::SOURCE_INPUT ].value = 1.23;
  g.inputs[ G::SOURCE_INPUT ].active = true;
  g.outputs[ G::SLID_OUTPUT ].active = true;
  
  for( int i=0; i<1000; ++i ) {
    g.step();
    std::cout << g.outputs[ G::SLID_OUTPUT ].value << "\n";
  }
}
