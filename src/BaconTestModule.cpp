#include "BaconPlugs.hpp"
#include "BaconModule.hpp"
#include "BaconModuleWidget.h"

#include "sst/basic-blocks/dsp/HilbertTransform.h"
#include "sst/basic-blocks/dsp/QuadratureOscillators.h"

namespace bp = baconpaul::rackplugs;

struct BaconTest : bp::BaconModule
{
    static constexpr int nParams{8}, nPorts{4};
    enum ParamIds
    {
        PARAM_0,
        NUM_PARAMS = PARAM_0 + nParams
    };

    enum InputIds
    {
        INPUT_0,
        NUM_INPUTS = INPUT_0 + nPorts
    };

    enum OutputIds
    {
        OUTPUT_0,
        NUM_OUTPUTS = OUTPUT_0 + nPorts
    };

    enum LightIds
    {
        NUM_LIGHTS
    };

    sst::basic_blocks::dsp::HilbertTransformMonoFloat hilbertMono;
    sst::basic_blocks::dsp::HilbertTransformStereoSSE hilbert;
    sst::basic_blocks::dsp::QuadratureOscillator<float> qo;

    BaconTest()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        for (int i = 0; i < nParams; ++i)
            configParam(PARAM_0 + i, 0, 1, 0, "Param " + std::to_string(i));
        for (int i = 0; i < nPorts; ++i)
        {
            configInput(INPUT_0 + i, "Input " + std::to_string(i));
            configInput(OUTPUT_0 + i, "Output " + std::to_string(i));
        }
    }

    double theSampleRate{1};
    void onSampleRateChange(const SampleRateChangeEvent &e) override
    {
        qo.setRate(2.0 * M_PI * 50 * e.sampleTime);
        hilbert.setSampleRate(e.sampleRate);
        hilbertMono.setSampleRate(e.sampleRate);
        Module::onSampleRateChange(e);
    }
    float lrt = -1.f;
    float prior[2]{0.f, 0.f};
    void process(const ProcessArgs &args) override {
        auto iL = inputs[INPUT_0].getVoltage() / 5.0f;
        auto iR = inputs[INPUT_0 + 1].getVoltage() / 5.0f;

        qo.step();

        if (params[PARAM_0].getValue() != lrt)
        {
            lrt = params[PARAM_0].getValue();
            auto rf = lrt * 800 - 200;
            qo.setRate(2.0 * M_PI * rf * args.sampleTime);
        }
        auto fb = params[PARAM_0 + 1].getValue();
        iL = 0.8 * ( iL + fb * fb * fb * prior[0] );
        iR = 0.8 * ( iR + fb * fb * fb * prior[1] );

        auto [L, R] = hilbert.stepToPair(iL, iR);

        auto [re, im] = L;

        auto [reR, imR] = R;

        prior[0] = (re * qo.v - im * qo.u);
        prior[1] = (reR * qo.v - imR * qo.u);
        outputs[OUTPUT_0+0].setVoltage(prior[0] * 5.f);
        outputs[OUTPUT_0+1].setVoltage(prior[1] * 5.f);
    }
};

struct BaconTestWidget : bp::BaconModuleWidget
{
    BaconTestWidget(BaconTest *model);
};

struct PolyWidget : public rack::Widget
{
    int64_t dc{0};
    void draw(const DrawArgs &args) override {
        auto s = box.size;

        auto vg = args.vg;

        auto dcm = dc % (int)(s.x - 40);

        nvgBeginPath(vg);
        nvgFillColor(vg, nvgRGB(255,0,0));
        nvgRect(vg, dcm, dcm, 40 , 40);
        nvgFill(vg);

        typedef std::vector<std::pair<float, float>> poly_t;
        std::vector<poly_t> polys;

        std::map<poly_t, NVGcolor> colm;
        for (int nsides = 3; nsides < 11; ++nsides)
        {
            poly_t p;
            for (auto i = 0; i < nsides; ++i)
            {
                auto x = std::sin(-i * 2.0 * M_PI / nsides) + 1;
                auto y = std::cos(i * 2.0 * M_PI / nsides) + 1;
                p.emplace_back(x, y);
            }
            polys.push_back(p);
            auto idx = nsides - 3;
            colm[p] = nvgRGB((255 - dcm * 20) * (idx < 5), idx * 15, dcm * 20);
        }

        int idx = 0;
        for (auto &poly : polys)
        {
            auto first{true};
            nvgBeginPath(vg);
            for (const auto &[x,y] : poly)
            {
                if (first)
                {
                    nvgMoveTo(vg, 15 * x + dcm, 15 * y + dcm + 40 + idx * 18);
                }
                else
                {
                    nvgLineTo(vg, 15 * x + dcm, 15 * y + dcm + 40 + idx * 18);
                }
                first = false;
            }
            nvgClosePath(vg);
            // nvgFillColor(vg, nvgRGB((255 - dcm * 20) * (idx < 5), idx * 15, dcm * 20));
            nvgFillColor(vg, colm[poly]);
            nvgFill(vg);
            nvgStrokeColor(vg, nvgRGB(0,0,50));
            nvgStroke(vg);
            idx ++;
        }

        dc++;
    }
};

BaconTestWidget::BaconTestWidget(BaconTest *model)
{
    setModule(model);
    box.size = Vec(SCREW_WIDTH * 15, RACK_HEIGHT);

    BaconBackground *bg = new BaconBackground(box.size, "BaconTest");
    addChild(bg->wrappedInFramebuffer());

    auto layoutSize = box.size;
    layoutSize.x /= 2;

    Vec cr(5, 35);
    auto dSp = (layoutSize.x - 10) / BaconTest::nPorts;
    for (int i = 0; i < BaconTest::nPorts; ++i)
    {
        addInput(createInput<PJ301MPort>(cr, module, BaconTest::INPUT_0 + i));
        cr.x += dSp;
    }

    cr = Vec(10, 90);
    auto kSp = (layoutSize.x - 20) / 2;
    for (int i = 0; i < BaconTest::nParams; ++i)
    {
        addParam(createParam<RoundLargeBlackKnob>(cr, module, BaconTest::PARAM_0 + i));
        if (i % 2 == 0)
            cr.x += kSp;
        else
        {
            cr.x = 10;
            cr.y += kSp;
        }
    }

    cr = Vec(5, 300);
    for (int i = 0; i < BaconTest::nPorts; ++i)
    {
        addOutput(createOutput<PJ301MPort>(cr, module, BaconTest::OUTPUT_0 + i));
        cr.x += dSp;
    }

    auto p = new PolyWidget();
    p->box.pos = rack::Vec(box.size.x / 2, 20);
    p->box.size = rack::Vec(box.size.x / 2 - 10, box.size.y - 40);

    addChild(p);

#if 0
    Vec knobPos = Vec(cr.x + 12, cr.y + 25);
    Vec knobCtr = knobPos.plus(Vec(18, 18));
    addParam(createParam<RoundLargeBlackKnob>(knobPos, module, Bitulator::WET_DRY_MIX));
    bg->addLabel(knobCtr.plus(Vec(8, 21)), "dist", 10, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
    bg->addLabel(knobCtr.plus(Vec(-8, 21)), "clean", 10, NVG_ALIGN_RIGHT | NVG_ALIGN_TOP);
    knobPos.x = cr.x + rs.x - 3 - 24;
    ;
    knobPos.y += diffY2c<RoundLargeBlackKnob, PJ301MPort>();
    addInput(createInput<PJ301MPort>(knobPos, module, Bitulator::MIX_CV));

    cr.y += 88;
    bg->addRoundedBorder(cr, rs);
    bg->addLabel(Vec(cr.x + 3, cr.y + 3), "Quantize", 14, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
    addChild(createLight<SmallLight<BlueLight>>(cr.plus(Vec(rs.x - 27, 6)), module,
                                                Bitulator::BITULATING_LIGHT));

    addParam(createParam<CKSS>(cr.plus(Vec(rs.x - 17, 5)), module, Bitulator::BITULATE));

    knobPos = Vec(cr.x + 12, cr.y + 25);
    knobCtr = knobPos.plus(Vec(18, 18));
    addParam(createParam<RoundLargeBlackKnob>(knobPos, module, Bitulator::STEP_COUNT));
    bg->addLabel(knobCtr.plus(Vec(8, 21)), "smth", 10, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
    bg->addLabel(knobCtr.plus(Vec(-8, 21)), "crnch", 10, NVG_ALIGN_RIGHT | NVG_ALIGN_TOP);
    knobPos.x = cr.x + rs.x - 3 - 24;
    ;
    knobPos.y += diffY2c<RoundLargeBlackKnob, PJ301MPort>();
    addInput(createInput<PJ301MPort>(knobPos, module, Bitulator::BIT_CV));

    cr.y += 88;
    bg->addRoundedBorder(cr, rs);
    bg->addLabel(Vec(cr.x + 3, cr.y + 3), "Amp/Clip", 14, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
    addChild(createLight<SmallLight<BlueLight>>(cr.plus(Vec(rs.x - 27, 6)), module,
                                                Bitulator::CRUNCHING_LIGHT));

    addParam(createParam<CKSS>(cr.plus(Vec(rs.x - 17, 5)), module, Bitulator::CLIPULATE));

    knobPos = Vec(cr.x + 12, cr.y + 25);
    knobCtr = knobPos.plus(Vec(18, 18));
    addParam(createParam<RoundLargeBlackKnob>(knobPos, module, Bitulator::AMP_LEVEL));
    bg->addLabel(knobCtr.plus(Vec(8, 21)), "11", 10, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
    bg->addLabel(knobCtr.plus(Vec(-8, 21)), "one", 10, NVG_ALIGN_RIGHT | NVG_ALIGN_TOP);
    knobPos.x = cr.x + rs.x - 3 - 24;
    ;
    knobPos.y += diffY2c<RoundLargeBlackKnob, PJ301MPort>();
    addInput(createInput<PJ301MPort>(knobPos, module, Bitulator::AMP_CV));

    Vec inP = Vec(10, RACK_HEIGHT - 15 - 43);
    Vec outP = Vec(box.size.x - 24 - 10, RACK_HEIGHT - 15 - 43);

    bg->addPlugLabel(inP, BaconBackground::SIG_IN, "in");
    addInput(createInput<PJ301MPort>(inP, module, Bitulator::SIGNAL_INPUT));

    bg->addPlugLabel(outP, BaconBackground::SIG_OUT, "out");
    addOutput(createOutput<PJ301MPort>(outP, module, Bitulator::CRUNCHED_OUTPUT));
#endif
}

Model *modelBaconTest = createModel<BaconTest, BaconTestWidget>("BaconTest");
