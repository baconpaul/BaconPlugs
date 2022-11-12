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
    std::vector<std::string> infoStrings, warningStrings;
    std::atomic<uint64_t> updateCount{1};

    void updateCurrentTarget(Module *m)
    {
        currentTarget = m;
        // FIXME i should really do this locally then lock and copy. get to that
        infoStrings.clear();
        warningStrings.clear();

        if (!m || !m->model)
        {
            currentTargetName = "Disconnected";
            updateCount ++;
            return;
        }


        currentTargetName = m->model->getFullName();

        int idx{0};

        idx = 0;
        for (auto &pq : m->paramQuantities)
        {
            std::ostringstream oss;
            oss << "PQ[" << idx++ << "] ";
            oss << "name='" << pq->name << "' label='" << pq->getLabel() << "'";
            if (pq->name.empty() || pq->getLabel()[0] == '#')
            {
                warningStrings.push_back(oss.str());
            }
            else
            {
                infoStrings.push_back(oss.str());
            }
        }

        idx = 0;
        for (auto &ii : m->inputInfos)
        {
            std::ostringstream oss;
            oss << "IN[" << idx++ << "] ";
            oss << "name='" << ii->name << "' label='" << ii->getFullName() << "'" ;
            if (ii->name.empty() || ii->getFullName()[0] == '#')
            {
                warningStrings.push_back(oss.str());
            }
            else
            {
                infoStrings.push_back(oss.str());
            }
        }
        for (auto &oo : m->outputInfos)
        {
            std::ostringstream oss;
            oss << "OUT[" << idx++ << "] ";
            oss << "name='" << oo->name << "' label='" << oo->getFullName() << "'";
            if (oo->name.empty() || oo->getFullName()[0] == '#')
            {
                warningStrings.push_back(oss.str());
            }
            else
            {
                infoStrings.push_back(oss.str());
            }
        }

        updateCount++;
    }

    void process(const ProcessArgs &args) override
    {
        if (outputs[THE_OUT_PROBE].isConnected() && !wasOutConnected)
        {
            auto cid = APP->engine->getCableIds();
            for (auto c : cid)
            {
                auto cable = APP->engine->getCable(c);
                if (cable->outputModule == this && cable->inputModule &&
                    cable->inputModule->getModel())
                {
                    auto m = cable->inputModule;
                    updateCurrentTarget(m);
                }
            }
        }
        else if (inputs[THE_IN_PROBE].isConnected() && !wasInConnected)
        {
            auto cid = APP->engine->getCableIds();
            for (auto c : cid)
            {
                auto cable = APP->engine->getCable(c);
                if (cable->inputModule == this && cable->outputModule &&
                    cable->outputModule->getModel())
                {
                    auto m = cable->outputModule;
                    updateCurrentTarget(m);
                }
            }
        }

        if (currentTarget &&
            !inputs[THE_IN_PROBE].isConnected()
            && !outputs[THE_OUT_PROBE].isConnected())
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

    int64_t warnC{0}, infoC{0};
};

LintBuddyWidget::LintBuddyWidget(LintBuddy *model) : bp::BaconModuleWidget()
{
    setModule(model);
    box.size = Vec(SCREW_WIDTH * 18, RACK_HEIGHT);

    BaconBackground *bg = new BaconBackground(box.size, "LintBuddy");
    addChild(bg->wrappedInFramebuffer());

    auto dl = new DynamicLabel(
        rack::Vec(box.size.x * 0.5, 40),
        [this]() {
            if (module)
            {
                auto m = dynamic_cast<LintBuddy *>(module);
                if (m)
                    return m->currentTargetName;
            }
            return std::string("No Module");
        },
        12, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM,
        baconpaul::rackplugs::BaconStyle::Colors::DEFAULT_LABEL);
    addChild(dl);

    auto start = 45.f;
    auto end = RACK_HEIGHT - 90;
    auto d = (end - start) * 0.5;

    auto ss = new ScrollableStringList(
        rack::Vec(5, start), rack::Vec(box.size.x - 10, d - 3),
        [this]() {
            if (!module)
                return std::vector<std::string>();
            auto m = dynamic_cast<LintBuddy *>(module);
            return m->warningStrings;
        },
        [this]() {
            if (!module)
                return false;
            auto m = dynamic_cast<LintBuddy *>(module);
            if (this->warnC != m->updateCount)
            {
                this->warnC = m->updateCount;
                return true;
            }
            return false;
        });
    addChild(ss);

    auto s2 = new ScrollableStringList(
        rack::Vec(5, start + d + 6), rack::Vec(box.size.x - 10, d - 3),
        [this]() {
            if (!module)
                return std::vector<std::string>();
            auto m = dynamic_cast<LintBuddy *>(module);
            return m->infoStrings;
        },
        [this]() {
            if (!module)
                return false;
            auto m = dynamic_cast<LintBuddy *>(module);
            if (this->infoC != m->updateCount)
            {
                this->infoC = m->updateCount;
                return true;
            }
            return false;
        });
    addChild(s2);

    int xpospl = box.size.x - 24 - 9;
    Vec outP = Vec(xpospl, RACK_HEIGHT - 60);
    bg->addPlugLabel(outP, BaconBackground::SIG_OUT, "lint");
    addOutput(createOutput<PJ301MPort>(outP, module, LintBuddy::THE_OUT_PROBE));

    outP.x -= 34;
    bg->addPlugLabel(outP, BaconBackground::SIG_IN, "lint");
    addInput(createInput<PJ301MPort>(outP, module, LintBuddy::THE_IN_PROBE));
}

Model *modelLintBuddy = createModel<LintBuddy, LintBuddyWidget>("LintBuddy");
