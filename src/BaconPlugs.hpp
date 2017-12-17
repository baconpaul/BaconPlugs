#include "rack.hpp"

#include <map>
#include <vector>
#include <string>

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

struct ALingADingWidget : ModuleWidget {
  ALingADingWidget();
};

struct BitulatorWidget : ModuleWidget {
  BitulatorWidget();
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

template <int H>
struct BaconSlider : SVGSlider
{
  int slider_height = 41;
  int slider_width = 20;
  int widget_width = 32;
  BaconSlider()
  {
    int margin = (widget_width-slider_width)/2;
    maxHandlePos = Vec( (widget_width-slider_width)/2, margin );
    minHandlePos = Vec( (widget_width-slider_width)/2, (H-slider_height - margin) );
    box.size = Vec( widget_width, H );

    background->svg = NULL;
    background->wrap(); 
    background->box.pos = Vec( 0, 0 );

    handle->svg = SVG::load( assetPlugin( plugin, "res/BaconSliderHandle.svg" ) );
    handle->wrap();

  }

  void draw( NVGcontext *vg ) override
  {
    int margin = (widget_width-slider_width)/2;
    int nStrokes = 10;
    int yTop = slider_height / 2;
    int yHeight = H - slider_height;

#if 0
    nvgBeginPath( vg );
    nvgRect( vg, 0, 0, widget_width, H );
    nvgFillColor( vg, COLOR_RED );
    nvgFill( vg );
#endif
    
    float dx = yHeight / nStrokes;


    for( int i=0; i<= nStrokes; ++i )
      {
        nvgBeginPath( vg );
        nvgMoveTo( vg, 1, dx * i + yTop + margin + 1 );
        nvgLineTo( vg, widget_width-1, dx * i + yTop + margin + 1 );
        nvgStrokeColor( vg, nvgRGBA( 200, 200, 200, 255 ) );
        nvgStroke( vg );
      }

    nvgBeginPath( vg );
    nvgRect( vg,
             widget_width/2 - 2, yTop,
             5, yHeight+1 );
    nvgFillColor( vg, COLOR_BLACK );
    nvgFill( vg );

    nvgBeginPath( vg );
    nvgMoveTo( vg, widget_width/2 - 2, yTop );
    nvgLineTo( vg, widget_width/2 - 2, yTop+yHeight+1 );
    nvgLineTo( vg, widget_width/2 - 2+5, yTop+yHeight+1 );
    nvgStrokeColor( vg, nvgRGBA( 120, 120, 120, 255 ) );
    nvgStroke( vg );
    
    for( int i=0; i<= nStrokes; ++i )
      {
        nvgBeginPath( vg );
        nvgMoveTo( vg, 1, dx * i + yTop + margin );
        nvgLineTo( vg, widget_width-1, dx * i + yTop + margin );
        nvgStrokeColor( vg, nvgRGBA( 10, 10, 10, 255 ) );
        nvgStroke( vg );
      }

    SVGSlider::draw( vg );
  }
};

/*
then something like this
*/
struct DMPTextPanel : virtual TransparentWidget
{
  std::string txt;
  float pxper;
  
  DMPTextPanel( Vec pos, const char* txtc, float pxperdot ) : txt( txtc ), pxper( pxperdot )
  {
    box.pos = pos;
    box.size.y = pxperdot * 7;
    box.size.x = 5 * txt.length() * pxperdot;
  }

  void draw( NVGcontext *vg ) override;
};

