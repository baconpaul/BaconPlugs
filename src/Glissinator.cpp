#include "BaconPlugs.hpp"

struct Glissinator : Module {
  enum ParamIds {
    GLISS_TIME,

    NUM_PARAMS
  };

  enum InputIds {
    SOURCE_INPUT,
    NUM_INPUTS
  };

  enum OutputIds {
    SLID_OUTPUT,
    NUM_OUTPUTS
  };

  enum LightIds {
    SLIDING_LIGHT,
    NUM_LIGHTS
  };

  float priorIn;
  float targetIn;
  int offsetCount;

  Glissinator() : Module( NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS ) {
    offsetCount = -1;
    params[ GLISS_TIME ].value = 0.1;
  }


  
  void step()
  {
    float glist_sec = params[ GLISS_TIME ].value;
    int shift_time = engineGetSampleRate() * glist_sec;
    if( shift_time < 10 ) shift_time = 10;
    
    float thisIn = inputs[ SOURCE_INPUT ].value;
    if( offsetCount < 0 )
      {
        priorIn = thisIn;
        offsetCount = 0;
      }

    bool inGliss = offsetCount != 0;
    float thisOut = thisIn;
    if( ! inGliss )
      {
        if( thisIn != priorIn )
          {
            targetIn = thisIn;
            offsetCount = 1;
            inGliss = true;
          }
      }

    if( inGliss )
      {
        if( thisIn != targetIn )
          {
            float lastKnown = ( ( shift_time - offsetCount ) * priorIn +
                                offsetCount * targetIn) / shift_time;
            targetIn = thisIn;
            priorIn = lastKnown;
            offsetCount = 0;
          }
        thisOut = ( ( shift_time - offsetCount ) * priorIn +
                    offsetCount * thisIn ) / shift_time;
        offsetCount ++;
        
      }

    if( offsetCount == shift_time )
      {
        offsetCount = 0;
        priorIn = thisIn;
        targetIn  = thisIn;
        inGliss = false;
      }

    lights[ SLIDING_LIGHT ].value = inGliss ? 1 : 0;
    outputs[ SLID_OUTPUT ].value = thisOut;
  }
};

GlissinatorWidget::GlissinatorWidget()
{
  Glissinator *module = new Glissinator();
  setModule( module );
  box.size = Vec( SCREW_WIDTH * 5, RACK_HEIGHT );
  BaconBackground *bg = new BaconBackground( box.size, "Glissinator" );

  addChild( bg->wrappedInFramebuffer() );

  ParamWidget *slider = createParam< GraduatedFader< 255 > >( Vec( bg->cx( 29 ), 43 ),
                                                              module,
                                                              Glissinator::GLISS_TIME,
                                                              0,
                                                              1,
                                                              0.1 );
  
  addParam( slider );

  Vec inP = Vec( 7, RACK_HEIGHT - 15 - 43 );
  Vec outP = Vec( box.size.x - 24 - 7, RACK_HEIGHT - 15 - 43 );
  
  bg->addPlugLabel( inP, BaconBackground::SIG_IN, "in" );
  addInput( createInput< PJ301MPort >( inP,
                                       module,
                                       Glissinator::SOURCE_INPUT ) );

  bg->addPlugLabel( outP, BaconBackground::SIG_OUT, "out" );
  
  addOutput( createOutput< PJ301MPort >( outP,
                                         module,
                                         Glissinator::SLID_OUTPUT ) );
  addChild( createLight< MediumLight< BlueLight > >( Vec( box.size.x/2 - 4.5 , 27 ),
                                                    module, Glissinator::SLIDING_LIGHT ) );
}
