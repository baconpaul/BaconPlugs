#include "rack.hpp"

using namespace rack;

#define SCREW_WIDTH 15
#define RACK_HEIGHT 380

extern Plugin *plugin;

struct AddOneWidget : ModuleWidget {
  AddOneWidget();
};

struct GlissinatorWidget : ModuleWidget {
  GlissinatorWidget();
};

template< typename COLORBASE >
struct SevenSegmentLight : virtual COLORBASE {
  int lx, ly, ppl;
  std::vector< Rect > unscaledLoc;
  int elementsByNum[ 10 ][ 7 ] = {
    { 1, 1, 1, 1, 1, 1, 0 },
    { 0, 1, 1, 0, 0, 0, 0 },
    { 1, 1, 0, 1, 1, 0, 1 },
    { 1, 1, 1, 1, 0, 0, 1 },
    { 0, 1, 1, 0, 0, 1, 1 },
    { 1, 0, 1, 1, 0, 1, 1 },
    { 1, 0, 1, 1, 1, 1, 1 },
    { 1, 1, 1, 0, 0, 0, 0 },
    { 1, 1, 1, 1, 1, 1, 1 },
    { 1, 1, 1, 1, 0, 1, 1 }
  };
  
  SevenSegmentLight()
  {
    lx = 7;
    ly = 11;
    ppl = 4;
    this->box.size = Vec( ppl * (lx-1) + 2, ppl * (ly-1) + 2 );

    // https://en.wikipedia.org/wiki/Seven-segment_display#/media/File:7_segment_display_labeled.svg
    unscaledLoc.push_back( Rect( Vec( 2, 1 ), Vec( 3, 1 ) ) );
    unscaledLoc.push_back( Rect( Vec( 5, 2 ), Vec( 1, 3 ) ) );
    unscaledLoc.push_back( Rect( Vec( 5, 6 ), Vec( 1, 3 ) ) );
    unscaledLoc.push_back( Rect( Vec( 2, 9 ), Vec( 3, 1 ) ) );
    unscaledLoc.push_back( Rect( Vec( 1, 6 ), Vec( 1, 3 ) ) );
    unscaledLoc.push_back( Rect( Vec( 1, 2 ), Vec( 1, 3 ) ) );
    unscaledLoc.push_back( Rect( Vec( 2, 5 ), Vec( 3, 1 ) ) );
  }
  void draw( NVGcontext *vg ) override
  {
    int w = this->box.size.x;
    int h = this->box.size.y;
    
    nvgBeginPath( vg );
    nvgRect( vg, 0, 0, w, h );
    nvgFillColor( vg, nvgRGBA( 25, 35, 25, 255 ) );
    nvgFill( vg );
    
    
    int i=0;
    float fvalue = this->module->lights[ this->firstLightId ].value;
    int value = clampf( fvalue, 0, 9 );
    int *ebn = elementsByNum[ value ];

    NVGcolor oncol = this->baseColors[ 0 ];
    
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
            nvgFillColor( vg, oncol );
          }
      else
        {
          nvgFillColor( vg, nvgRGBA( 50, 70, 50, 255 ) );
        }
        nvgFill( vg );
        ++i;
      }
    
  }
};

