#include "BaconPlugs.hpp"
#include <jansson.h>

std::map< std::string, int > BaconPlugFontMgr::fontMap;

int BaconPlugFontMgr::get( NVGcontext *vg, std::string resName )
{
  if( fontMap.find( resName ) == fontMap.end() )
    {
      fontMap[ resName ] = nvgCreateFont( vg, resName.c_str(), assetPlugin( plugin, resName ).c_str() );
    }
  return fontMap[ resName ];
}

struct DMPText // Thanks http://scruss.com/blog/tag/font/
{
  typedef std::map< char, std::vector< bool > > fontData_t;
private:
  static fontData_t fontData;
  static bool initialized;
  void init();
  
public:
  DMPText() {
    init();
  }
  
  void drawText( NVGcontext *vg, Vec pos, char c, int pxSize )
  {
    fontData_t::iterator k = fontData.find( c );
    if( k != fontData.end() ) {
      fontData_t::mapped_type blist = k->second;
      int row=0, col=0;
      for( auto v = blist.begin(); v != blist.end(); ++v )
        {
          if( *v )
            {
              float xo = (col+0.5) * pxSize + pos.x;
              float yo = (row+0.5) * pxSize + pos.y;
              nvgBeginPath( vg );
              nvgRect( vg, xo-0.1, yo-0.1, pxSize+0.2, pxSize+0.2 );
              nvgFillColor( vg, COLOR_BLACK );
              nvgFill( vg );
            }
          
          col++;
          if( col == 5 ) {
            col = 0;
            row ++;
          }
        }
    }
    else {
    }
  }
};


bool DMPText::initialized = false;
DMPText::fontData_t DMPText::fontData = DMPText::fontData_t();

void DMPText::init()
{
  if( ! initialized ) {
      initialized = true;

      json_t *json;
      json_error_t error;
      
      json = json_load_file(assetPlugin( plugin, "res/Keypunch029.json" ).c_str(), 0, &error);
      if(!json) {
        printf( "JSON FILE not loaded\n" );
      }
      const char* key;
      json_t *value;
      json_object_foreach( json, key, value ) {
        fontData_t::mapped_type valmap;
        size_t index;
        json_t *aval;
        json_array_foreach( value, index, aval ) {
          std::string s( json_string_value( aval ) );
          for( const char* c = s.c_str(); *c != 0; ++c ) {
            valmap.push_back( *c == '#' );
          }
        }
        DMPText::fontData[ key[ 0 ] ] = valmap;
      }
  }
}


void DMPTextPanel::draw( NVGcontext *vg )
{
  DMPText d;

  nvgBeginPath( vg );
  nvgRect( vg, 0, 0, box.size.x, box.size.y );
  nvgFillColor( vg, nvgRGBA( 255, 255, 200, 255 ) );
  nvgFill( vg );

  Vec cpos = Vec( 0, 0 );
  for( const char* c = txt.c_str(); *c != 0; ++c ) {
    d.drawText( vg, cpos, *c, pxper );
    cpos.x += pxper * 5.0;
  }
}

void BaconPlugBackground::draw( NVGcontext *vg )
{
  if( memFont < 0 )
    memFont = BaconPlugFontMgr::get( vg, "res/Monitorica-Bd.ttf" );

  nvgBeginPath( vg );
  nvgRect( vg, 0, 0, box.size.x, box.size.y );
  nvgFillColor( vg, nvgRGBA( 220, 220, 210, 255 ) );
  nvgFill( vg );

  nvgBeginPath( vg );
  nvgMoveTo( vg, 0, 0 );
  nvgLineTo( vg, box.size.x, 0 );
  nvgLineTo( vg, box.size.x, box.size.y );
  nvgLineTo( vg, 0, box.size.y );
  nvgLineTo( vg, 0, 0 );
  nvgStrokeColor( vg, nvgRGBA( 180, 180, 170, 255 ) );
  nvgStroke( vg );

  nvgFontFaceId( vg, memFont );
  nvgFontSize( vg, 14 );
  nvgFillColor( vg, nvgRGBA( 0, 0, 0, 255 ) );
  nvgStrokeColor( vg, nvgRGBA( 0, 0, 0, 255 ) );
  nvgTextAlign( vg, NVG_ALIGN_CENTER|NVG_ALIGN_BOTTOM );
  nvgText( vg, box.size.x / 2, box.size.y - 5, "BaconPlugs", NULL );

  nvgFontFaceId( vg, memFont );
  nvgFontSize( vg, 16 );
  nvgFillColor( vg, nvgRGBA( 0, 0, 0, 255 ) );
  nvgStrokeColor( vg, nvgRGBA( 0, 0, 0, 255 ) );
  nvgTextAlign( vg, NVG_ALIGN_CENTER|NVG_ALIGN_TOP );
  nvgText( vg, box.size.x / 2, 5, title.c_str(), NULL );


}

BaconPlugLabel::BaconPlugLabel( Vec portPos, LabelAt l, LabelStyle s, const char* ilabel )
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
  case( SIG_IN ) :
    {
      nvgBeginPath( vg );
      nvgRoundedRect( vg, 0, 0, box.size.x, box.size.y, 5 );
      nvgStrokeColor( vg, COLOR_BLACK );
      nvgStroke( vg );
      break;
    }
  case( SIG_OUT ) :
    {
      nvgBeginPath( vg );
      nvgRoundedRect( vg, 0, 0, box.size.x, box.size.y, 5 );
      nvgFillColor( vg, nvgRGBA( 90, 90, 60, 255 ) );
      nvgFill( vg );
      nvgStrokeColor( vg, COLOR_BLACK );
      nvgStroke( vg );

      txtCol = COLOR_WHITE;
      break;
    }
  case( OTHER ) :
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
