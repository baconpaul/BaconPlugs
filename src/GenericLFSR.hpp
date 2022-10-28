template <class TBase> struct GenericLFSR : TBase
{
    enum ParamIds
    {
        SEED_LSB,

        NUM_PARAMS = SEED_LSB + 4
    };

    enum InputIds
    {
        NUM_INPUTS
    };

    enum OutputIds
    {
        NUM_OUTPUTS
    };

    enum LightIds
    {
        SEED_LIGHT_LSB,

        NUM_LIGHTS = SEED_LIGHT_LSB + 4
    };

    using TBase::inputs;
    using TBase::lights;
    using TBase::outputs;
    using TBase::params;

    GenericLFSR() : TBase()
    {
        TBase::config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        TBase::configParam(SEED_LSB, 0, 15, 0, "Seed LSB");
    }

    void process(const typename TBase::ProcessArgs &args) override
    {
        lights[SEED_LIGHT_LSB].value = params[SEED_LSB].getValue();
    }
};
