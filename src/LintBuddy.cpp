#include "BaconPlugs.hpp"
#include <initializer_list>

#include "BaconModule.hpp"
#include "BaconModuleWidget.h"

#define SCALE_LENGTH 12


namespace bp = baconpaul::rackplugs;

struct LintBuddy : virtual bp::BaconModule
{
    enum ParamIds
    {
        NUM_PARAMS
    };

    enum InputIds
    {
        THE_IN_PROBE,
        NUM_INPUTS
    };

    enum OutputIds
    {
        THE_OUT_PROBE,
        NUM_OUTPUTS
    };

    enum LightIds
    {
        NUM_LIGHTS,
    };

    LintBuddy() : bp::BaconModule()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configInput(THE_IN_PROBE, "THE PROBE (IN)");
        configOutput(THE_OUT_PROBE, "THE PROBE (OUT)");
    }

    bool wasOutConnected{false}, wasInConnected{false};

    Module *currentTarget{nullptr};
    std::string currentTargetName{"Disconnected"};
    void updateCurrentTarget(Module *m)
    {
        currentTarget = m;
        if (!m || !m->model)
        {
            currentTargetName = "Disconnected";
            return;
        }

        currentTargetName = m->model->getFullName();

        for (auto &pq : m->paramQuantities)
            std::cout << "pq- " << pq->name << "/" << pq->getLabel() << std::endl;

        for (auto &ii : m->inputInfos)
            std::cout << "ii- " << ii->name << "/" << ii->getFullName() << std::endl;

        for (auto &oo : m->outputInfos)
            std::cout << "oo- " << oo->name <<  "/" << oo->getFullName() << std::endl;
    }

    void process(const ProcessArgs &args) override
    {
        if (outputs[THE_OUT_PROBE].isConnected() && !wasOutConnected)
        {
            auto cid = APP->engine->getCableIds();
            for (auto c : cid)
            {
                auto cable = APP->engine->getCable(c);
                if (cable->outputModule == this && cable->inputModule && cable->inputModule->getModel())
                {
                    auto m = cable->inputModule;
                    updateCurrentTarget(m);
                }
            }
        } else if (inputs[THE_IN_PROBE].isConnected() && !wasInConnected)
        {
            auto cid = APP->engine->getCableIds();
            for (auto c : cid)
            {
                auto cable = APP->engine->getCable(c);
                if (cable->inputModule == this && cable->outputModule && cable->outputModule->getModel())
                {
                    auto m = cable->outputModule;
                    updateCurrentTarget(m);
                }
            }
        }
        else if (wasInConnected &&
                 ! (inputs[THE_IN_PROBE].isConnected() || inputs[THE_OUT_PROBE].isConnected()))
        {
            updateCurrentTarget(nullptr);
        }

        wasOutConnected = outputs[THE_OUT_PROBE].isConnected();
        wasInConnected = inputs[THE_IN_PROBE].isConnected();
    }
};

struct LintBuddyWidget : bp::BaconModuleWidget
{
    LintBuddyWidget(LintBuddy *model);
};

LintBuddyWidget::LintBuddyWidget(LintBuddy *model) : bp::BaconModuleWidget()
{
    setModule(model);
    box.size = Vec(SCREW_WIDTH * 18, RACK_HEIGHT);

    BaconBackground *bg = new BaconBackground(box.size, "LintBuddy");
    addChild(bg->wrappedInFramebuffer());

    int xpospl = box.size.x - 24 - 9;
    Vec outP = Vec(xpospl, RACK_HEIGHT - 60);


    auto dl = new DynamicLabel(rack::Vec(box.size.x * 0.5, 40), [this]()
        {
            if (module)
            {
                auto m = dynamic_cast<LintBuddy *>(module);
                if (m)
                    return m->currentTargetName;
            }
            return std::string("No Module");
        },
        12, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM, baconpaul::rackplugs::BaconStyle::Colors::DEFAULT_LABEL);
    addChild(dl);

    bg->addPlugLabel(outP, BaconBackground::SIG_OUT, "lint");
    addOutput(createOutput<PJ301MPort>(outP, module, LintBuddy::THE_OUT_PROBE));


    outP.x -= 34;

    bg->addPlugLabel(outP, BaconBackground::SIG_IN, "lint");
    addInput(createInput<PJ301MPort>(outP, module, LintBuddy::THE_IN_PROBE));


}

Model *modelLintBuddy = createModel<LintBuddy, LintBuddyWidget>("LintBuddy");
