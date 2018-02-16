#ifndef INCLUDE_COMPONENTS_HPP
#define INCLUDE_COMPONENTS_HPP

#include "rack.hpp"

#include <map>
#include <vector>
#include <string>
#include <tuple>

#include "GraduatedFader.hpp"
#include "BufferedDrawFunction.hpp"

using namespace rack;


template <typename T, int px = 4>
struct SevenSegmentLight : T {
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

  int pvalue;

  BufferedDrawFunctionWidget<SevenSegmentLight<T, px>> *buffer;
  
  SevenSegmentLight( )
  {
    lx = 7;
    ly = 11;
    ppl = px;
    pvalue = -1;
    this->box.size = Vec( ppl * (lx-1) + 2, ppl * (ly-1) + 2 );

    // https://en.wikipedia.org/wiki/Seven-segment_display#/media/File:7_segment_display_labeled.svg
    unscaledLoc.push_back( Rect( Vec( 2, 1 ), Vec( 3, 1 ) ) );
    unscaledLoc.push_back( Rect( Vec( 5, 2 ), Vec( 1, 3 ) ) );
    unscaledLoc.push_back( Rect( Vec( 5, 6 ), Vec( 1, 3 ) ) );
    unscaledLoc.push_back( Rect( Vec( 2, 9 ), Vec( 3, 1 ) ) );
    unscaledLoc.push_back( Rect( Vec( 1, 6 ), Vec( 1, 3 ) ) );
    unscaledLoc.push_back( Rect( Vec( 1, 2 ), Vec( 1, 3 ) ) );
    unscaledLoc.push_back( Rect( Vec( 2, 5 ), Vec( 3, 1 ) ) );

    buffer = new BufferedDrawFunctionWidget<SevenSegmentLight<T,px>>( Vec( 0, 0 ), this->box.size,
                                                                      this,
                                                                      &SevenSegmentLight<T,px>::drawSegments );
    this->addChild( buffer );
  }

  void draw( NVGcontext *vg ) override
  {
    float fvalue = this->module->lights[ this->firstLightId ].value;
    int value = clamp( fvalue, 0.0f, 9.0f );
    if( value != pvalue )
      buffer->dirty = true;

    pvalue = value;
    
    buffer->draw( vg );
  }

  void drawSegments( NVGcontext *vg )
  {
    // This is now buffered to only be called when the value has changed
    int w = this->box.size.x;
    int h = this->box.size.y;
    
    nvgBeginPath( vg );
    nvgRect( vg, 0, 0, w, h );
    nvgFillColor( vg, nvgRGBA( 25, 35, 25, 255 ) );
    nvgFill( vg );
    
    
    int i=0;
    float fvalue = this->module->lights[ this->firstLightId ].value;
    int value = clamp( fvalue, 0.0f, 9.0f );

    int *ebn = elementsByNum[ value ];

    NVGcolor oncol = this->baseColors[ 0 ];
    
    for( auto it = unscaledLoc.begin(); it < unscaledLoc.end(); ++it )
      {
        float y = it->pos.y - 0.5;
        float x = it->pos.x - 0.5;
        int ew = it->size.x;
        int eh = it->size.y;
        nvgBeginPath( vg );
        // New version with corners
        float x0 = x * ppl + 1;
        float y0 = y * ppl + 1;
        float w = ew * ppl;
        float h = eh * ppl;
        float tri = ppl / 2;
        
        if( eh == 1 )
          {
            // This is a sideways element
            nvgMoveTo( vg, x0, y0 );
            nvgLineTo( vg, x0 + w, y0 );
            nvgLineTo( vg, x0 + w + tri, y0 + tri );
            nvgLineTo( vg, x0 + w, y0 + h);
            nvgLineTo( vg, x0, y0 + h);
            nvgLineTo( vg, x0 - tri, y0 + tri );
            nvgClosePath( vg );
          }
        else
          {
            nvgMoveTo( vg, x0, y0 );
            nvgLineTo( vg, x0, y0 + h );
            nvgLineTo( vg, x0 + tri, y0 + h + tri );
            nvgLineTo( vg, x0 + w, y0 + h);
            nvgLineTo( vg, x0 + w, y0);
            nvgLineTo( vg, x0 + tri, y0 - tri );
          }


        // Old version nvgRect( vg, x * ppl + 1, y * ppl + 1, ew * ppl, eh * ppl );
        if( ebn[ i ] > 0 )
          {
            nvgFillColor( vg, oncol );
            nvgFill( vg );
          }
        else
          {
            nvgFillColor( vg, nvgRGBA( 50, 70, 50, 255 ) );
            nvgFill( vg );
                    
          }
        ++i;
      }
    
  }
};

struct BaconBackground : virtual TransparentWidget
{
  static NVGcolor bg;
  static NVGcolor bgOutline;
  static NVGcolor highlight;

  typedef std::tuple< Rect, NVGcolor, bool > col_rect_t;
  std::vector< col_rect_t > rects;

  int memFont = -1;
  std::string title;

  enum LabelAt {
    ABOVE,
    BELOW,
    LEFT,
    RIGHT
  };

  enum LabelStyle {
    SIG_IN,
    SIG_OUT,
    OTHER
  };

  int cx() { return box.size.x / 2; }
  int cx( int w ) { return (box.size.x-w) / 2; }
  
  BaconBackground( Vec size, const char* lab );
  ~BaconBackground() { }
  
  BaconBackground *addLabel( Vec pos, const char* lab, int px )
  {
    return addLabel( pos, lab, px, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM );
  }
  BaconBackground *addLabel( Vec pos, const char* lab, int px, int align );

  BaconBackground *addPlugLabel( Vec plugPos, LabelStyle s, const char* ilabel ) {
    return addPlugLabel( plugPos, LabelAt::ABOVE, s, ilabel );
  }
  BaconBackground *addPlugLabel( Vec plugPos, LabelAt l, LabelStyle s, const char* ilabel );
  BaconBackground *addRoundedBorder( Vec pos, Vec sz );

  BaconBackground *addFilledRect( Vec pos, Vec sz, NVGcolor fill )
  {
    Rect r;
    r.pos = pos; r.size = sz;
    rects.push_back( col_rect_t( r, fill, true ) );
    return this;
  }

  BaconBackground *addRect( Vec pos, Vec sz, NVGcolor fill )
  {
    Rect r;
    r.pos = pos; r.size = sz;
    rects.push_back( col_rect_t( r, fill, false ) );
    return this;
  }

  void draw( NVGcontext *vg ) override;

  FramebufferWidget *wrappedInFramebuffer();
};

#endif
