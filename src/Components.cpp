#include "BaconPlugs.hpp"
#include <jansson.h>


struct BaconPlugFontMgr
{
  static std::map< std::string, int > fontMap;
  static int get( NVGcontext *vg, std::string resName )
  {
    if( fontMap.find( resName ) == fontMap.end() )
      {
        fontMap[ resName ] = nvgCreateFont( vg, resName.c_str(), assetPlugin( plugin, resName ).c_str() );
      }
    return fontMap[ resName ];
  }
};

std::map< std::string, int > BaconPlugFontMgr::fontMap;

struct BaconPlugBackground : virtual TransparentWidget
{
  int memFont = -1;
  std::string title;
  
  BaconPlugBackground( Vec size, const char* titleIn ) : title( titleIn )
  {
    box.pos = Vec( 0, 0 );
    box.size = size;
  }

  void draw( NVGcontext *vg ) override;
};

struct RoundedBorder : virtual TransparentWidget
{
  RoundedBorder( Vec pos, Vec sz )
  {
    box.pos = pos;
    box.size = sz;
  }

  void draw( NVGcontext *vg )
  {
    nvgBeginPath( vg );
    nvgRoundedRect( vg, 0, 0, box.size.x, box.size.y, 5 );
    nvgStrokeColor( vg, COLOR_BLACK );
    nvgStroke( vg );
  }
};

struct TextLabel : virtual TransparentWidget
{
  int memFont = -1;
  std::string label;
  int pxSize;
  int align;
  TextLabel( Vec pos, const char* lab, int px ) : label( lab ), pxSize( px )
  {
    align = NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE;
    box.pos = pos;
  }

  TextLabel( Vec pos, const char* lab, int px, int al ) : label( lab ), pxSize( px ), align( al )
  {
    box.pos = pos;
  }

  void draw( NVGcontext *vg ) override {
    if( memFont < 0 )
      memFont = BaconPlugFontMgr::get( vg, "res/Monitorica-Bd.ttf" );

    nvgBeginPath( vg );
    nvgFontFaceId( vg, memFont );
    nvgFontSize( vg, pxSize );
    nvgFillColor( vg, COLOR_BLACK );
    nvgTextAlign( vg, align );
    nvgText( vg, 0, 0, label.c_str(), NULL );
  }
};

struct BaconPlugLabel : virtual TransparentWidget
{
  int memFont = -1;

  BaconStyleModuleWidget::LabelStyle  st;
  BaconStyleModuleWidget::LabelAt     at;
  std::string label;

  BaconPlugLabel( Vec portPos, BaconStyleModuleWidget::LabelAt l, BaconStyleModuleWidget::LabelStyle s, const char* ilabel );
  
  void draw( NVGcontext *vg ) override;

};


void BaconPlugBackground::draw( NVGcontext *vg )
{
  if( memFont < 0 )
    memFont = BaconPlugFontMgr::get( vg, "res/Monitorica-Bd.ttf" );

  nvgBeginPath( vg );
  nvgRect( vg, 0, 0, box.size.x, box.size.y );
  nvgFillColor( vg, BaconStyleModuleWidget::bg );
  nvgFill( vg );

  nvgBeginPath( vg );
  nvgMoveTo( vg, 0, 0 );
  nvgLineTo( vg, box.size.x, 0 );
  nvgLineTo( vg, box.size.x, box.size.y );
  nvgLineTo( vg, 0, box.size.y );
  nvgLineTo( vg, 0, 0 );
  nvgStrokeColor( vg, BaconStyleModuleWidget::bgOutline );
  nvgStroke( vg );

  nvgFontFaceId( vg, memFont );
  nvgFontSize( vg, 14 );
  nvgFillColor( vg, COLOR_BLACK );
  nvgStrokeColor( vg, COLOR_BLACK );
  nvgTextAlign( vg, NVG_ALIGN_CENTER|NVG_ALIGN_BOTTOM );
  nvgText( vg, box.size.x / 2, box.size.y - 5, "BaconPlugs", NULL );

  nvgFontFaceId( vg, memFont );
  nvgFontSize( vg, 16 );
  nvgFillColor( vg, COLOR_BLACK );
  nvgStrokeColor( vg, COLOR_BLACK );
  nvgTextAlign( vg, NVG_ALIGN_CENTER|NVG_ALIGN_TOP );
  nvgText( vg, box.size.x / 2, 5, title.c_str(), NULL );


}

BaconPlugLabel::BaconPlugLabel( Vec portPos, BaconStyleModuleWidget::LabelAt l, BaconStyleModuleWidget::LabelStyle s, const char* ilabel )
  :
  st( s ), at( l ), label( ilabel )
{
  box.size.x = 24 + 5;
  box.size.y = 24 + 5 + 20;
  
  // switch on position but for now just do above
  box.pos.x = portPos.x - 2.5;
  box.pos.y = portPos.y - 2.5 - 17;
}


void BaconPlugLabel::draw( NVGcontext *vg )
{
  if( memFont < 0 )
    memFont = BaconPlugFontMgr::get( vg, "res/Monitorica-Bd.ttf" );

  NVGcolor txtCol = COLOR_BLACK;
  
  switch( st ) {
  case( BaconStyleModuleWidget::SIG_IN ) :
    {
      nvgBeginPath( vg );
      nvgRoundedRect( vg, 0, 0, box.size.x, box.size.y, 5 );
      nvgStrokeColor( vg, COLOR_BLACK );
      nvgStroke( vg );
      break;
    }
  case( BaconStyleModuleWidget::SIG_OUT ) :
    {
      nvgBeginPath( vg );
      nvgRoundedRect( vg, 0, 0, box.size.x, box.size.y, 5 );
      nvgFillColor( vg, BaconStyleModuleWidget::highlight );
      nvgFill( vg );

      nvgStrokeColor( vg, COLOR_BLACK );
      nvgStroke( vg );

      txtCol = COLOR_WHITE;
      break;
    }
  case( BaconStyleModuleWidget::OTHER ) :
    {
      nvgBeginPath( vg );
      nvgRoundedRect( vg, 0, 0, box.size.x, box.size.y, 5 );
      nvgStrokeColor( vg, COLOR_RED );
      nvgStroke( vg );
      break;
    }
  }
      
  nvgFontFaceId( vg, memFont );
  nvgFontSize( vg, 13 );
  nvgFillColor( vg, txtCol );
  nvgTextAlign( vg, NVG_ALIGN_CENTER|NVG_ALIGN_TOP );
  nvgText( vg, box.size.x / 2, 3, label.c_str(), NULL );

}


TransparentWidget *BaconStyleModuleWidget::createBaconBG( const char* lab )
{
  return new BaconPlugBackground( box.size, lab );
}

TransparentWidget *BaconStyleModuleWidget::createBaconLabel( Vec pos, const char* lab, int px, int align )
{
  return new TextLabel( pos, lab, px, align );
}

TransparentWidget *BaconStyleModuleWidget::createPlugLabel( Vec plugPos, LabelAt l, LabelStyle s, const char* ilabel )
{
  return new BaconPlugLabel( plugPos, l, s, ilabel );
}

TransparentWidget *BaconStyleModuleWidget::createRoundedBorder( Vec pos, Vec sz )
{
  return new RoundedBorder( pos, sz );
}

NVGcolor BaconStyleModuleWidget::bg = nvgRGBA( 220, 220, 210, 255 );
NVGcolor BaconStyleModuleWidget::bgOutline = nvgRGBA( 180, 180, 170, 255 );
NVGcolor BaconStyleModuleWidget::highlight = nvgRGBA( 90, 90, 60, 255 );
