#ifndef INCLUDE_COMPONENTS_HPP
#define INCLUDE_COMPONENTS_HPP

#include "rack.hpp"

#include <map>
#include <vector>
#include <string>

#include "GraduatedFader.hpp"

using namespace rack;


template <typename T>
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
  
  SevenSegmentLight( )
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

struct BaconBackground : virtual TransparentWidget
{
  static NVGcolor bg;
  static NVGcolor bgOutline;
  static NVGcolor highlight;

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

  void draw( NVGcontext *vg ) override;

  FramebufferWidget *wrappedInFramebuffer();
};

#endif
