#include "rack.hpp"



template <int H>
struct GraduatedFader : SVGSlider
{
  int slider_height = 41;
  int slider_width  = 20;
  int widget_width  = 28;
  FramebufferWidget *notches;

  GraduatedFader()
  {
    background->svg = NULL;
    background->wrap(); 
    background->box.pos = Vec( 0, 0 );
    
    handle->svg = SVG::load( assetPlugin( plugin, "res/BaconSliderHandle.svg" ) );
    handle->wrap();

    maxHandlePos = Vec( (widget_width-slider_width)/2, 0 );
    minHandlePos = Vec( (widget_width-slider_width)/2, (H-slider_height) );
    box.size = Vec( widget_width, H );

    InternalSliderBG *bg = new InternalSliderBG( this );
    bg->box.pos = Vec( 0, 0 );
    bg->box.size = box.size;
    
    notches = new FramebufferWidget();
    notches->addChild( bg );
    notches->box.pos = Vec( 0, 0 );
    notches->box.size = box.size;
  }
  
  void draw( NVGcontext *vg ) override
  {
    notches->draw( vg );
    SVGSlider::draw( vg );
  }

  struct InternalSliderBG : virtual TransparentWidget
  {
    GraduatedFader *s;
    InternalSliderBG( GraduatedFader *s_ ) : s( s_ ) {}
    
    void draw( NVGcontext *vg )
    {
      int widget_width = s->widget_width;
      int nStrokes = 10;
      int slideTop = s->slider_height / 2;
      int slideHeight = H - s->slider_height;
      int slideBump = 5;
      int slotWidth = 1;
      

#ifdef DEBUG_NOTCHES
      nvgBeginPath( vg );
      nvgRect( vg, 0, 0, widget_width, H );
      nvgFillColor( vg, COLOR_RED );
      nvgFill( vg );
#endif

      float dx = (1.0 * slideHeight) / nStrokes;

      // Firest the gray highlights
      for( int i=0; i<= nStrokes; ++i )
        {
          nvgBeginPath( vg );
          nvgRect( vg, 1, slideTop + dx * i, widget_width-2, 1 );
          nvgFillColor( vg, nvgRGBA( 200, 200, 200, 255 ) );
          nvgFill( vg );
        }

      // and now the black notches
      for( int i=0; i<= nStrokes; ++i )
        {
          nvgBeginPath( vg );
          nvgRect( vg, 1, slideTop + dx * i-1, widget_width-2, 1.5 );
          nvgFillColor( vg, nvgRGBA( 100, 100, 100, 255 ));
          nvgFill( vg );
        }

      // OK so now we want to draw the vertical line
      nvgBeginPath( vg );
      nvgRect( vg, widget_width/2 - slotWidth, slideTop - slideBump, 2 * slotWidth + 1, slideHeight + 2 * slideBump );
      nvgFillColor( vg, COLOR_BLACK );
      nvgFill( vg );

    }
  };
};

