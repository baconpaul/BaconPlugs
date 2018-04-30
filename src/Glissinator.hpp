
template <typename TBase>
struct Glissinator : public TBase {
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

  Glissinator() : TBase( NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS ) {
    offsetCount = -1;
    this->params[ GLISS_TIME ].value = 0.1;
  }


  
  void step() override
  {
    float glist_sec = this->params[ GLISS_TIME ].value;
    int shift_time = engineGetSampleRate() * glist_sec;
    if( shift_time < 10 ) shift_time = 10;
    
    float thisIn = this->inputs[ SOURCE_INPUT ].value;
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

    if( offsetCount >= shift_time )
      {
        offsetCount = 0;
        priorIn = thisIn;
        targetIn  = thisIn;
        inGliss = false;
      }

    this->lights[ SLIDING_LIGHT ].value = inGliss ? 1 : 0;
    this->outputs[ SLID_OUTPUT ].value = thisOut;
  }
};
