#include "BaconPlugs.hpp"
#include <cstdio>
#include <fstream>

#include "rack.hpp"
#include "patch.hpp"
#include "BaconModule.hpp"
#include "BaconModuleWidget.h"

#define SCALE_LENGTH 12

namespace bp = baconpaul::rackplugs;

struct CPort : public rack::PJ301MPort
{
    void appendContextMenu(ui::Menu *menu) override {
        std::cout << "AppendContextMenu Called" << std::endl;
    }
};

struct LintBuddyTest
{
    virtual ~LintBuddyTest() = default;
    virtual void run(rack::Module *m, std::vector<std::string> &warnings,
                     std::vector<std::string> &info) = 0;
    virtual std::string getName() = 0;
};

struct EverythingHasAName : LintBuddyTest
{
    std::string getName() override { return "Labels Check"; }
    void run(rack::Module *m, std::vector<std::string> &warnings,
             std::vector<std::string> &info) override
    {
        if (m->paramQuantities.size() != m->params.size())
            warnings.push_back("Params and ParamQuantities differ");

        int idx{0};
        for (auto &pq : m->paramQuantities)
        {
            std::ostringstream oss;
            oss << "PQ[" << idx++ << "] ";
            oss << "name='" << pq->name << "' label='" << pq->getLabel() << "'";
            if (pq->name.empty() || pq->getLabel()[0] == '#')
            {
                warnings.push_back(oss.str());
            }
            else
            {
                info.push_back(oss.str());
            }
        }

        idx = 0;
        for (auto &ii : m->inputInfos)
        {
            std::ostringstream oss;
            oss << "IN[" << idx++ << "] ";
            oss << "name='" << ii->name << "' label='" << ii->getFullName() << "'";
            if (ii->name.empty() || ii->getFullName()[0] == '#')
            {
                warnings.push_back(oss.str());
            }
            else
            {
                info.push_back(oss.str());
            }
        }
        idx = 0;
        for (auto &oo : m->outputInfos)
        {
            std::ostringstream oss;
            oss << "OUT[" << idx++ << "] ";
            oss << "name='" << oo->name << "' label='" << oo->getFullName() << "'";
            if (oo->name.empty() || oo->getFullName()[0] == '#')
            {
                warnings.push_back(oss.str());
            }
            else
            {
                info.push_back(oss.str());
            }
        }
    }
};

struct ProbeBypass : LintBuddyTest
{
    std::string getName() override { return "Probe Bypass"; }
    void run(rack::Module *m, std::vector<std::string> &warnings,
             std::vector<std::string> &info) override
    {
        if (m->bypassRoutes.empty())
            info.push_back("No Bypass Routes in Module");
        for (const auto &br : m->bypassRoutes)
        {
            auto i = br.inputId;
            auto o = br.outputId;

            std::string in{"unnamed_input"}, on{"unnamed_output"};
            if (i >= 0 && i < (int)m->inputInfos.size())
                in = m->inputInfos[i]->name;
            if (o >= 0 && o < (int)m->outputInfos.size())
                on = m->outputInfos[o]->name;

            info.push_back("Bypass from " + std::to_string(i) + " (" + in + ") to " +
                           std::to_string(o) + " (" + on + ")");
        }
    }
};

struct JSONToInfo : LintBuddyTest
{
    std::string getName() override { return "JSON Extract"; }
    void run(rack::Module *m, std::vector<std::string> &warnings,
             std::vector<std::string> &info) override
    {
        if (!m)
            return;
        auto json = m->dataToJson();

        if (!json)
        {
            warnings.push_back("dataToJSON returned a null");
        }
        else
        {
            auto s = std::string(json_dumps(json, JSON_INDENT(2)));
            std::stringstream ss(s);
            std::string token;
            while (std::getline(ss, token, '\n'))
            {
                info.push_back(token);
            }
            json_decref(json);
        }
    }
};

struct WidgetPositions : LintBuddyTest
{
    std::string getName() override { return "WidgetPositions"; }
    void recurseTree(rack::Widget *w, std::vector<std::string> &info, const std::string &pfx = "")
    {
        for (auto c : w->children)
        {
            std::ostringstream oss;
            auto nm = typeid(*c).name();
            oss << pfx << "| box: w=" << w->box.size.x << ", h=" << w->box.size.y
                << " x=" << w->box.size.x << " y=" << w->box.size.y << " class=[" << nm << "]";
            info.push_back(oss.str());
            if (!c->children.empty())
                recurseTree(c, info, pfx + "|--");
        }
    }
    void run(rack::Module *m, std::vector<std::string> &warnings,
             std::vector<std::string> &info) override
    {
        if (!m)
            return;
        auto w = APP->scene->rack->getModule(m->getId());
        if (!w)
            warnings.push_back("Unable to locate Widget");

        recurseTree(w, info, "");
    }
};

struct GotAnyWhiteLists : LintBuddyTest
{
    std::string getName() override { return "WhiteList"; }
    void run(rack::Module *m, std::vector<std::string> &warnings,
             std::vector<std::string> &info) override
    {
        info.clear();
        for (const auto &[k, pl] : rack::settings::moduleWhitelist)
        {
            if (pl.subscribed)
            {
                info.push_back("Subscribed: " + k);
            }
            else
            {
                warnings.push_back("Partial Sub: " + k + " to " +
                                   std::to_string(pl.moduleSlugs.size()));
            }
        }
    }
};

struct MyPatch : LintBuddyTest
{
    std::string getName() override { return "MyPatch"; }
    void run(rack::Module *m, std::vector<std::string> &warnings,
             std::vector<std::string> &info) override
    {
        info.clear();
        warnings.clear();
        auto ctx = rack::contextGet();
        rack::patch::Manager *pt = (ctx ? ctx->patch : nullptr);
        if (pt)
        {
            info.push_back("Patch Path");
            info.push_back("[" + pt->path + "]");

        }
        else
        {
            info.push_back("PT is NULL" );
        }
    }
};

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

        currentTest = std::make_unique<EverythingHasAName>();

        updateCurrentTarget(nullptr);
    }

    bool wasOutConnected{false}, wasInConnected{false};

    Module *currentTarget{nullptr};
    std::string currentTargetName{"Disconnected"};
    std::vector<std::string> info, warnings;
    std::atomic<int64_t> updateCount{1};

    std::unique_ptr<LintBuddyTest> currentTest;

    void rerun() { updateCurrentTarget(currentTarget); }

    void onExpanderChange(const ExpanderChangeEvent &e) override {
        std::cout << "Expander Change " << e.side << " "
                  << rightExpander.module << " " << leftExpander.module << std::endl;
    }

    void updateCurrentTarget(Module *m)
    {
        currentTarget = m;
        // FIXME i should really do this locally then lock and copy. get to that
        info.clear();
        warnings.clear();

        if (!m || !m->model)
        {
            currentTargetName = "Disconnected";
            warnings.push_back("LintBuddy is a Developer Tool.");
            warnings.push_back("");
            warnings.push_back("It checks module features but has no");
            warnings.push_back("musical purpose. Please don't use");
            warnings.push_back("it in performance patches. Want to add");
            warnings.push_back("a test or feature? Happy to take a PR!");
            updateCount++;
            return;
        }

        currentTargetName = m->model->getFullName();

        currentTest->run(m, warnings, info);

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

        if (currentTarget && !inputs[THE_IN_PROBE].isConnected() &&
            !outputs[THE_OUT_PROBE].isConnected())
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
    std::vector<std::string> deleteOnExit;
    ~LintBuddyWidget()
    {
        for (const auto &d : deleteOnExit)
        {
            rack::system::remove(d);
        }
    }

    template <typename T> void addTest(rack::ui::Menu *m)
    {
        auto tmp = std::make_unique<T>();
        auto lbm = dynamic_cast<LintBuddy *>(module);
        m->addChild(rack::createMenuItem(tmp->getName(), "", [lbm] {
            if (lbm)
            {
                lbm->currentTest = std::make_unique<T>();
                lbm->rerun();
            }
        }));
    }

    std::string plainTextContents()
    {
        std::ostringstream of;
        auto m = dynamic_cast<LintBuddy *>(module);
        of << "LintBuddy: module=" << m->currentTargetName << "\n";
        of << "         : test  =" << m->currentTest->getName() << "\n";
        of << "\nWARNINGS (" << m->warnings.size() << ")\n";
        for (const auto &d : m->warnings)
            of << d << "\n";

        of << "\nINFO (" << m->info.size() << ")\n";
        for (const auto &d : m->info)
            of << d << "\n";

        return of.str();
    }
    void showAsHtml()
    {
        std::string lintBuddyReports = rack::asset::user("BaconMusic/LintBuddy/");
        if (!rack::system::isDirectory(lintBuddyReports))
            rack::system::createDirectory(lintBuddyReports);

        // this is obviously lame
        auto name = lintBuddyReports + "/rpt-" + std::to_string(rand()) + ".html";
        deleteOnExit.push_back(name);
        auto of = std::ofstream(name);
        if (of.is_open())
        {
            of << "<html><body><pre>\n";
            of << plainTextContents() << "\n";
            of << "</html></body></pre>\n";

            of.close();
            if (name[0] != '/')
                name = "/" + name;
            rack::system::openBrowser("file://" + name);
        }
    }
};

LintBuddyWidget::LintBuddyWidget(LintBuddy *m) : bp::BaconModuleWidget()
{
    setModule(m);
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
            return m->warnings;
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
            return m->info;
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
    addOutput(createOutput<CPort>(outP, module, LintBuddy::THE_OUT_PROBE));

    outP.x -= 34;
    bg->addPlugLabel(outP, BaconBackground::SIG_IN, "lint");
    addInput(createInput<CPort>(outP, module, LintBuddy::THE_IN_PROBE));

    rack::Rect butB;
    butB.pos.x = 10;
    butB.pos.y = outP.y - 2.5 - 17;
    butB.size.x = 140;
    butB.size.y = 22;

    auto cb = new CBButton(butB.pos, butB.size);
    cb->getLabel = [m]() {
        if (m)
            return m->currentTest->getName();
        else
            return std::string("Test Selector");
    };
    cb->onPressed = [this, m]() {
        if (!m)
            return;
        auto men = rack::createMenu();
        men->addChild(rack::createMenuLabel("Select Test"));
        addTest<EverythingHasAName>(men);
        addTest<ProbeBypass>(men);
        addTest<JSONToInfo>(men);
        addTest<WidgetPositions>(men);
        addTest<GotAnyWhiteLists>(men);
        addTest<MyPatch>(men);
    };
    addChild(cb);

    butB.pos.y += 26;
    butB.size.x /= 2;
    butB.size.x -= 1;
    cb = new CBButton(butB.pos, butB.size);
    cb->getLabel = []() { return "Output To..."; };
    cb->onPressed = [this, m]() {
        if (!m)
            return;
        auto men = rack::createMenu();
        men->addChild(rack::createMenuLabel("Output To"));
        men->addChild(rack::createMenuItem("STDOUT (if attached)", "", [this]() {
            std::cout << plainTextContents() << std::endl;
        }));
        men->addChild(rack::createMenuItem("HTML", "", [this]() { showAsHtml(); }));
        men->addChild(rack::createMenuItem("RACK Log", "", [this]() {
            INFO("%s", ("LintBuddy Log Output\n" + plainTextContents()).c_str());
        }));
    };
    addChild(cb);

    butB.pos.x += butB.size.x + 1;
    auto rb = new CBButton(butB.pos, butB.size);
    rb->getLabel = []() { return "Run 100 times"; };
    rb->onPressed = [this]() {
        auto lbm = dynamic_cast<LintBuddy *>(module);
        if (lbm)
        {
            std::cout << "Running 100x" << std::endl;
            for (int i=0; i<100; ++i)
                lbm->rerun();
        }
    };

    addChild(rb);

    std::cout << "Spaceship " << ((1 <=> 2) < 0) << " is cool" << std::endl;
}

Model *modelLintBuddy = createModel<LintBuddy, LintBuddyWidget>("LintBuddy");
