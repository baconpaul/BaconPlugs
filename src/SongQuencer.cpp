#include "BaconPlugs.hpp"
#include <initializer_list>

#include "BaconModule.hpp"
#include "BaconModuleWidget.h"

#include <random>

namespace bp = baconpaul::rackplugs;

struct SongQuencer : virtual bp::BaconModule
{
    enum ParamIds
    {
        NUM_PARAMS
    };

    enum InputIds
    {
        CLOCK,
        RESET,
        NUM_INPUTS
    };

    enum OutputIds
    {
        ROOT,
        FULL,
        SHELL,
        NUM_OUTPUTS
    };

    enum LightIds
    {
        NUM_LIGHTS
    };

    SongQuencer() : bp::BaconModule()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        configInput(CLOCK, "Clock (1/4 note) in");

        for (int i = 0; i < 12; ++i)
        {
            chords[0][i] = (i < 4 || i == 6 || i == 7 || i == 10) ? "C"
                         : (i == 4 || i == 5 || i == 9)         ? "F"
                                                                : "G";
        }
    }

    rack::dsp::SchmittTrigger clockTrigger;
    rack::dsp::SchmittTrigger resetTrigger;
    void process(const ProcessArgs &args) override
    {
        if (markStepsDirty)
        {
            std::cout << "Updating Steps" << std::endl;
            markStepsDirty = false;
        }

        if (clockTrigger.process(inputs[CLOCK].getVoltage()))
        {
            intStep ++;
            if (intStep > numPhrases * stepsPerPhrase)
                intStep = 0;
        }

        if (resetTrigger.process(inputs[RESET].getVoltage()))
        {
            intStep = 0;
        }
    }

    json_t* dataToJson() override {
        json_t* rootJ = json_object();
        json_t* songJ = json_array();
        for (auto i=0; i<stepsPerPhrase; ++i)
            json_array_append_new(songJ, json_string(song[i].c_str()));
        json_object_set_new(rootJ, "song", songJ);


        json_t *chordJ = json_array();
        for (auto p=0; p<numPhrases; ++p)
        {
            json_t *phraseJ = json_array();
            for (auto i=0; i<stepsPerPhrase; ++i)
            {
                json_array_append_new(phraseJ, json_string(chords[p][i].c_str()));
            }
            json_array_append_new(chordJ, phraseJ);
        }
        json_object_set_new(rootJ, "chords", chordJ);

        return rootJ;
    }

    static constexpr int stepsPerPhrase{16}, numPhrases{6};
    std::array<std::array<std::string, stepsPerPhrase>, numPhrases> chords;
    std::array<std::string, stepsPerPhrase> song;
    std::atomic<bool> markStepsDirty{false};

    int intStep{0};
    static std::pair<int, int> phraseStepFromIntStep(int intStep) {
        return { intStep / stepsPerPhrase, intStep % stepsPerPhrase};
    }
    static int intStepFrompPhraseSteo(int phrase, int pstep)
    {
        return phrase * stepsPerPhrase + pstep;
    }
};

struct SQTextField : LedDisplayTextField
{
    SongQuencer *module;
    int phrase{0}, pstep{0}, intStep{0};

    SQTextField()
    {
        textOffset = rack::Vec(1, 1);
        multiline = false;
    }

    void draw(const DrawArgs &args) override
    {
        auto style = baconpaul::rackplugs::BaconStyle::get();
        nvgBeginPath(args.vg);
        nvgRoundedRect(args.vg, 0, 0, box.size.x, box.size.y,
                       baconpaul::rackplugs::StyleConstants::rectRadius);
        nvgFillColor(args.vg,
                     style->getColor(baconpaul::rackplugs::BaconStyle::LED_BACKGROUND_COLOR));
        nvgFill(args.vg);

        nvgStrokeColor(args.vg,
                       style->getColor(baconpaul::rackplugs::BaconStyle::SECTION_RULE_LINE));
        if (module && module->intStep == this->intStep)
        {
            nvgStrokeColor(args.vg, nvgRGB(255, 0, 0));
        }
        nvgStroke(args.vg);

        color = style->getColor(baconpaul::rackplugs::BaconStyle::LED_TEXT_COLOR);

        LedDisplayTextField::draw(args);
    }

    void init()
    {
        if (phrase >= 0)
            intStep = SongQuencer::intStepFrompPhraseSteo(phrase, pstep);
    }

    void step() override
    {
        LedDisplayTextField::step();
        if (module)
        {
            if (phrase >= 0)
                setText(module->chords[phrase][pstep]);
            else
                setText(module->song[pstep]);
        }
    }

    void onChange(const ChangeEvent &e) override
    {
        if (module)
        {
            if( phrase >= 0)
                module->chords[phrase][pstep] = getText();
            else
                module->song[pstep] = getText();
            module->markStepsDirty = true;
        }
    }
};

struct SongQuencerWidget : bp::BaconModuleWidget
{
    typedef SongQuencer M;
    SongQuencerWidget(SongQuencer *model);
};

SongQuencerWidget::SongQuencerWidget(SongQuencer *m) : bp::BaconModuleWidget()
{
    setModule(m);
    box.size = Vec(SCREW_WIDTH * 52, RACK_HEIGHT);

    BaconBackground *bg = new BaconBackground(box.size, "SongQuencer");
    addChild(bg->wrappedInFramebuffer());

    {
        int yp = 30;
        auto w = 43.0, h = 25.0, margin = 4.0;

        for (int i = 0; i < SongQuencer::numPhrases + 1; ++i)
        {
            int xp = 5;
            std::string sec = "A";
            sec[0] = (char)('A' + i);
            if (i == SongQuencer::numPhrases)
                sec = "SNG";
            bg->addLabel(Vec(xp, yp + h / 2), sec.c_str(), 11, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE,
                         baconpaul::rackplugs::BaconStyle::DEFAULT_LABEL);
            xp += 20;
            for (int j = 0; j < SongQuencer::stepsPerPhrase; ++j)
            {
                auto sqw = createWidget<SQTextField>(rack::Vec(0, 0));
                sqw->box.pos = rack::Vec(xp, yp);
                sqw->box.size = rack::Vec(w, h);
                sqw->phrase = i == SongQuencer::numPhrases ? -1 : i;
                sqw->pstep = j;
                sqw->multiline = false;
                sqw->module = m;
                sqw->init();
                addChild(sqw);
                xp += w + margin;
            }
            yp += h + margin;
            if (i == SongQuencer::numPhrases - 1)
                yp += 2 * margin;
        }
    }
    auto xp = 5;
    {
        auto h = 30;
        bg->addRoundedBorder(Vec(xp, RACK_HEIGHT - (h + 30)), Vec(70, 35),
                             baconpaul::rackplugs::BaconStyle::INPUT_BG);
        bg->addLabel(Vec(20, RACK_HEIGHT - (h + 12)), "Clock", 11,
                     NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM,
                     baconpaul::rackplugs::BaconStyle::DEFAULT_LABEL);
        bg->addLabel(Vec(20, RACK_HEIGHT - h), "in", 11, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM,
                     baconpaul::rackplugs::BaconStyle::DEFAULT_LABEL);
        addInput(createInput<PJ301MPort>(Vec(43, RACK_HEIGHT - (h + 24)), module, M::CLOCK));

        xp += 75;
    }

    {
        auto h = 30;
        bg->addRoundedBorder(Vec(xp, RACK_HEIGHT - (h + 30)), Vec(70, 35),
                             baconpaul::rackplugs::BaconStyle::INPUT_BG);
        bg->addLabel(Vec(xp + 15, RACK_HEIGHT - (h + 12)), "Reset", 11,
                     NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM,
                     baconpaul::rackplugs::BaconStyle::DEFAULT_LABEL);
        bg->addLabel(Vec(xp + 15, RACK_HEIGHT - h), "Song", 11, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM,
                     baconpaul::rackplugs::BaconStyle::DEFAULT_LABEL);
        addInput(createInput<PJ301MPort>(Vec(xp + 38, RACK_HEIGHT - (h + 24)), module, M::RESET));

        xp += 75;
    }

    auto mko = [this, bg, &xp](auto s1, auto s2, auto ov) {
        auto h = 30;
        bg->addRoundedBorder(Vec(xp, RACK_HEIGHT - (h + 30)), Vec(70, 35),
                             baconpaul::rackplugs::BaconStyle::HIGHLIGHT_BG);
        bg->addLabel(Vec(xp + 15, RACK_HEIGHT - (h + 12)), s1, 11,
                     NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM,
                     baconpaul::rackplugs::BaconStyle::DEFAULT_HIGHLIGHT_LABEL);
        bg->addLabel(Vec(xp + 15, RACK_HEIGHT - h), s2, 11, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM,
                     baconpaul::rackplugs::BaconStyle::DEFAULT_HIGHLIGHT_LABEL);
        addOutput(createOutput<PJ301MPort>(Vec(xp + 38, RACK_HEIGHT - (h + 24)), module, ov));

        xp += 75;
    };

    mko("Root", "tone", M::ROOT);
    mko("Full", "chord", M::FULL);
    mko("Shell", "chord", M::SHELL);
}

Model *modelSongQuencer = createModel<SongQuencer, SongQuencerWidget>("SongQuencer");
