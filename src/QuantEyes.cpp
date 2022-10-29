#include "BaconPlugs.hpp"
#include <initializer_list>

#include "BaconModule.hpp"
#include "BaconModuleWidget.h"

#define SCALE_LENGTH 12


namespace bp = baconpaul::rackplugs;

struct QuantEyes : virtual bp::BaconModule
{
    enum ParamIds
    {
        ROOT_STEP,
        SCALE_PARAM,
        NUM_PARAMS = SCALE_PARAM + SCALE_LENGTH
    };

    enum InputIds
    {
        CV_INPUT,
        NUM_INPUTS
    };

    enum OutputIds
    {
        QUANTIZED_OUT,
        CHANGE_TRIG_OUT,
        NUM_OUTPUTS
    };

    enum LightIds
    {
        ROOT_LIGHT,
        ACTIVE_NOTE_LIGHTS,
        SCALE_LIGHTS = ACTIVE_NOTE_LIGHTS + 16 * SCALE_LENGTH,
        NUM_LIGHTS = SCALE_LIGHTS + SCALE_LENGTH
    };

    int scaleState[SCALE_LENGTH];
    dsp::SchmittTrigger scaleTriggers[SCALE_LENGTH];
    float priorOut[16]; // max-poly
    int trigOut[16];

    QuantEyes() : bp::BaconModule()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(ROOT_STEP, 0, 12, 0, "The root in 1/12 of a volt");
        for (int i = 0; i < SCALE_LENGTH; ++i)
            configParam(SCALE_PARAM + i, 0, 1, 0);
        for (int i = 0; i < SCALE_LENGTH; ++i)
            scaleState[i] = 1;

        configInput(CV_INPUT, "V/Oct Input");
        configOutput(QUANTIZED_OUT, "Quantized Output");
        configOutput(CHANGE_TRIG_OUT, "Trigger on Change");
        for (int i=0; i<16; ++i)
        {
            priorOut[i] = 0;
            trigOut[i] = 0;
        }
    }

    void process(const ProcessArgs &args) override
    {
        int root = clamp(params[ROOT_STEP].getValue(), 0.0f, 12.0f);
        lights[ROOT_LIGHT].value = root;

        for (int i = 0; i < SCALE_LENGTH; ++i)
        {
            if (scaleTriggers[i].process(params[SCALE_PARAM + i].getValue()))
            {
                scaleState[i] = !scaleState[i];
            }
            lights[SCALE_LIGHTS + i].value = scaleState[i];
            for (int j = 0; j < 16; ++j)
            {
                lights[ACTIVE_NOTE_LIGHTS + i + j * SCALE_LENGTH].value = 0;
            }
        }

        int nChan = inputs[CV_INPUT].getChannels();
        outputs[QUANTIZED_OUT].setChannels(nChan);
        outputs[CHANGE_TRIG_OUT].setChannels(nChan);
        for (int i = 0; i < nChan; ++i)
        {
            if (inputs[CV_INPUT].isConnected())
            {
                float in = inputs[CV_INPUT].getVoltage(i);

                double octave, note;
                note = modf(in, &octave);

                // In the event we get negative input, modf has the behavior of
                // making both outputs negative. So 1.23 gives .23 in note and 1
                // in octave -1.23 gives -.23 in note and -1 in octave which is
                // the same as .77 in note and -2 in octave
                if (in < 0)
                {
                    note += 1;
                    octave -= 1;
                }

                // assert( note >= 0 );

                // We need to re-mod note since the root can make our range of
                // the integer note somewhere other than (0,1) so first push by
                // root. This 1e-5 increase makes sure that QuantEyes is an
                // identity for tuned inputs (which can sometimes end up just below
                // the floor).
                float noteF = (floor(note * SCALE_LENGTH + 1e-5) + root);
                // Then find the new integer part of the pushed stuff
                int noteI = (int)noteF % SCALE_LENGTH;
                // and bump the octave if root pushes us up. Note if root==0
                // this never activates.
                if (noteF > SCALE_LENGTH - 1)
                    octave += 1.0;

                // Then find the activated note searching from above
                while (scaleState[noteI] == 0 && noteI > 0)
                    noteI--;

                // turn on the light
                lights[ACTIVE_NOTE_LIGHTS + i * SCALE_LENGTH + noteI].value = 1;

                // and figure out the CV
                float out = 1.0 * noteI / SCALE_LENGTH + octave;

                if (out != priorOut[i])
                {
                    priorOut[i] = out;
                    trigOut[i] = (int)(APP->engine->getSampleRate() * 0.015);
                }
                outputs[QUANTIZED_OUT].setVoltage(out, i);

                if (trigOut[i])
                {
                    outputs[CHANGE_TRIG_OUT].setVoltage(10.0, i);
                    trigOut[i] --;
                }
                else
                {
                    outputs[CHANGE_TRIG_OUT].setVoltage(0.0, i);
                }
            }
        }
    }

    json_t *dataToJson() override
    {
        json_t *rootJ = json_object();
        json_t *scaleJ = json_array();
        for (int i = 0; i < SCALE_LENGTH; ++i)
        {
            json_t *noteJ = json_integer(scaleState[i]);
            json_array_append_new(scaleJ, noteJ);
        }
        json_object_set_new(rootJ, "scaleState", scaleJ);

        return rootJ;
    }

    void dataFromJson(json_t *rootJ) override
    {
        json_t *scaleJ = json_object_get(rootJ, "scaleState");
        if (scaleJ)
            for (int i = 0; i < SCALE_LENGTH; ++i)
            {
                json_t *noteJ = json_array_get(scaleJ, i);
                if (noteJ)
                    scaleState[i] = json_integer_value(noteJ);
            }
    }
};

struct QuantEyesWidget : bp::BaconModuleWidget
{
    QuantEyesWidget(QuantEyes *model);
    void appendModuleSpecificContextMenu(Menu *menu) override;
};

QuantEyesWidget::QuantEyesWidget(QuantEyes *model) : bp::BaconModuleWidget()
{
    setModule(model);
    box.size = Vec(SCREW_WIDTH * 12, RACK_HEIGHT);

    BaconBackground *bg = new BaconBackground(box.size, "QuantEyes");
    addChild(bg->wrappedInFramebuffer());

    float rx = 15, ry = 30, sp = 22, slope = 8.5;
    for (int i = 0; i < SCALE_LENGTH; ++i)
    {
        char d[24];
        sprintf(d, "%d", i + 1);
        if (i == 0)
            d[0] = 'R';
        int x0 = rx + (i + 0.5) * slope;
        int yp0 = (SCALE_LENGTH - i - 1) * sp;
        bg->addLabel(Vec(rx - 3, yp0 + ry + sp / 2), d, 12, NVG_ALIGN_RIGHT | NVG_ALIGN_MIDDLE);
        addParam(createParam<LEDButton>(Vec(x0, yp0 + ry), module, QuantEyes::SCALE_PARAM + i));
        addChild(createLight<MediumLight<BlueLight>>(Vec(x0 + 4, yp0 + ry + 4), module,
                                                     QuantEyes::SCALE_LIGHTS + i));

        for (int j = 0; j < 16; ++j)
        {
            float yOff = (j < 8 ? 0 : 4.5);
            addChild(createLight<TinyLight<BlueLight>>(
                Vec(x0 + 20.6 + 4.5 * (j % 8), yp0 + ry + 6 + yOff), module,
                QuantEyes::ACTIVE_NOTE_LIGHTS + i + j * SCALE_LENGTH));
        }

        auto c = nvgRGBA(225, 225, 225, 255);
        if (i == 1 || i == 3 || i == 6 || i == 8 || i == 10)
            c = nvgRGBA(110, 110, 110, 255);

        bg->addFilledRect(Vec(rx, yp0 + ry + 7), Vec(x0 - rx, 4.5), c);
        bg->addRect(Vec(rx, yp0 + ry + 7), Vec(x0 - rx, 4.5), nvgRGBA(70, 70, 70, 255));
    }

    int xpospl = box.size.x - 24 - 9;
    Vec inP = Vec(xpospl - 64, RACK_HEIGHT - 60);
    Vec trigP = Vec(xpospl - 32, RACK_HEIGHT - 60);
    Vec outP = Vec(xpospl, RACK_HEIGHT - 60);

    bg->addPlugLabel(inP, BaconBackground::SIG_IN, "in");
    addInput(createInput<PJ301MPort>(inP, module, QuantEyes::CV_INPUT));

    bg->addPlugLabel(trigP, BaconBackground::SIG_OUT, "trig");
    addOutput(createOutput<PJ301MPort>(trigP, module, QuantEyes::CHANGE_TRIG_OUT));

    bg->addPlugLabel(outP, BaconBackground::SIG_OUT, "out");
    addOutput(createOutput<PJ301MPort>(outP, module, QuantEyes::QUANTIZED_OUT));

    bg->addRoundedBorder(Vec(10, box.size.y - 79.5), Vec(65, 49));
    bg->addLabel(Vec(45, box.size.y - 79.5 + 2), "Root CV", 12, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);
    int ybot = box.size.y - 79.5 + 24 + 5 + 20;
    addParam(
        createParam<RoundSmallBlackKnob>(Vec(16, ybot - 3 - 28), module, QuantEyes::ROOT_STEP));
    addChild(MultiDigitSevenSegmentLight<BlueLight, 2, 2>::create(Vec(42, ybot - 5 - 24), module,
                                                                  QuantEyes::ROOT_LIGHT));
}

struct QuantEyesScaleItem : MenuItem
{
    QuantEyes *quanteyes;
    typedef std::vector<int> scale_t;

    scale_t scale;

    void onAction(const event::Action &e) override
    {
        INFO("Selecting pre-canned scale %s", text.c_str());
        quanteyes->scaleState[0] = 10;
        for (auto i = 1; i < SCALE_LENGTH; ++i)
            quanteyes->scaleState[i] = 0;

        int pos = 0;
        for (auto p : scale)
        {
            pos += p;
            if (pos < SCALE_LENGTH)
                quanteyes->scaleState[pos] = 10;
        }
    }
    void setScale(scale_t scaleData) { scale = scaleData; }
};

void QuantEyesWidget::appendModuleSpecificContextMenu(Menu *menu)
{
    // TODO: Fix me for 1.0
    menu->addChild(new rack::ui::MenuSeparator);
    menu->addChild(createMenuLabel("Scales:"));
    QuantEyes *qe = dynamic_cast<QuantEyes *>(module);

    auto addScale = [menu, qe](const char *name, QuantEyesScaleItem::scale_t scale) {
        QuantEyesScaleItem *scaleItem =
            createMenuItem<QuantEyesScaleItem>(name);
        scaleItem->quanteyes = qe;
        scaleItem->setScale(scale);
        menu->addChild(scaleItem);
    };

    addScale("Major", {2, 2, 1, 2, 2, 2, 1});
    addScale("Natural Minor", {2, 1, 2, 2, 1, 2, 2});
    addScale("Harmonic Minor", {2, 1, 2, 2, 1, 3, 1});
    addScale("Whole Tone", {2, 2, 2, 2, 2, 2});
}

Model *modelQuantEyes = createModel<QuantEyes, QuantEyesWidget>("QuantEyes");
