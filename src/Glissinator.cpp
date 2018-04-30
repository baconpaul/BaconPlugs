#include "BaconPlugs.hpp"
#include "Glissinator.hpp"

struct GlissinatorWidget : ModuleWidget {
  typedef Glissinator< Module > G;
  GlissinatorWidget( Glissinator<Module> *model );
};

GlissinatorWidget::GlissinatorWidget( Glissinator<Module> *model ) : ModuleWidget( model )
{
  box.size = Vec( SCREW_WIDTH * 5, RACK_HEIGHT );
  BaconBackground *bg = new BaconBackground( box.size, "Glissinator" );

  addChild( bg->wrappedInFramebuffer() );
  // FIXME - spacing
  // addChild( new BaconHelpButton( "README.md#glissinator" ) );
  
  ParamWidget *slider = ParamWidget::create< GraduatedFader< 255 > >( Vec( bg->cx( 29 ), 43 ),
                                                              module,
                                                              G::GLISS_TIME,
                                                              0,
                                                              1,
                                                              0.1 );
  
  addParam( slider );

  Vec inP = Vec( 7, RACK_HEIGHT - 15 - 43 );
  Vec outP = Vec( box.size.x - 24 - 7, RACK_HEIGHT - 15 - 43 );
  
  bg->addPlugLabel( inP, BaconBackground::SIG_IN, "in" );
  addInput( Port::create< PJ301MPort >( inP, Port::INPUT,
                                       module,
                                       G::SOURCE_INPUT ) );

  bg->addPlugLabel( outP, BaconBackground::SIG_OUT, "out" );
  
  addOutput( Port::create< PJ301MPort >( outP, Port::OUTPUT,
                                         module,
                                         G::SLID_OUTPUT ) );
  addChild( ModuleLightWidget::create< MediumLight< BlueLight > >( Vec( box.size.x/2 - 4.5 , 27 ),
                                                    module, G::SLIDING_LIGHT ) );
}

Model *modelGlissinator = Model::create<Glissinator<Module>,GlissinatorWidget>("Bacon Music", "Glissinator", "Glissinator", EFFECT_TAG); 
