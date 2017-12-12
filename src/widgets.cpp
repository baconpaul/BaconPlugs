#include "BaconPlugs.hpp"

void SevenSegmentLight::draw( NVGcontext *vg )
{
  int w = box.size.x;
  int h = box.size.y;
  
  nvgBeginPath( vg );
  nvgRect( vg, 0, 0, w, h );
  nvgFillColor( vg, nvgRGBA( 25, 35, 25, 255 ) );
  nvgFill( vg );


  int i=0;
  float fvalue = module->lights[ firstLightId ].value;
  int value = clampf( fvalue, 0, 9 );
  int *ebn = elementsByNum[ value ];

  for( auto it = unscaledLoc.begin(); it < unscaledLoc.end(); ++it )
    {
      float y = it->pos.y - 0.5;
      float x = it->pos.x - 0.5;
      int ew = it->size.x;
      int eh = it->size.y;
      nvgBeginPath( vg );
      nvgRect( vg, x * ppl + 1, y * ppl + 1, ew * ppl, eh * ppl );
      if( ebn[ i ] > 0 )
        {
          nvgFillColor( vg, nvgRGBA( 255, 0, 0, 255 ) );
        }
      else
        {
          nvgFillColor( vg, nvgRGBA( 50, 70, 50, 255 ) );
        }
      nvgFill( vg );
      ++i;
    }

}

void SevenSegmentLight::step()
{
}

