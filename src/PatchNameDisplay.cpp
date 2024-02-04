#include "BaconPlugs.hpp"

#include "BaconModule.hpp"
#include "BaconModuleWidget.h"

#include "patch.hpp"

#include <ghc/filesystem.hpp>
namespace fs = ghc::filesystem;

#define SCALE_LENGTH 12

namespace bp = baconpaul::rackplugs;

struct PatchNameDisplay : virtual bp::BaconModule
{
    enum ParamIds
    {
        NUM_PARAMS
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
        NUM_LIGHTS,
    };

    PatchNameDisplay() : bp::BaconModule()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    }

    void process(const ProcessArgs &args) override {}

    enum ColorScheme
    {
        DEFAULT,
        HICONTRAST_LIGHT,
        HICONTRAST_DARK,
    };

    std::atomic<ColorScheme> colorScheme;
    json_t *dataToJson() override
    {
        auto *rootJ = json_object();
        json_object_set(rootJ, "colorScheme", json_integer(colorScheme));
        return rootJ;
    }
    void dataFromJson(json_t *rootJ) override
    {
        auto cs = json_object_get(rootJ, "colorScheme");
        colorScheme = (ColorScheme)(cs ? json_integer_value(cs) : 0);
    }
};

struct PatchNameDisplayWidget : bp::BaconModuleWidget
{
    PatchNameDisplayWidget(PatchNameDisplay *model);
    ~PatchNameDisplayWidget() {}

    std::string lastPatchName{};
    void step() override
    {
        if (module)
        {
            auto c = dynamic_cast<PatchNameDisplay *>(module);
            if (c)
            {
                std::string tpn = rack::contextGet()->patch->path;
                if (tpn != lastPatchName)
                {
                    tpn = lastPatchName;
                    bdw->dirty = true;
                }
            }
        }
        bp::BaconModuleWidget::step();
    }
    BufferedDrawFunctionWidget *bdw{nullptr};
    void drawLabel(NVGcontext *vg)
    {
        nvgSave(vg);
        nvgRotate(vg, M_PI_2);

        auto memFont =
            InternalFontMgr::get(vg, baconpaul::rackplugs::BaconStyle::get()->fontName());
        auto labelColor = baconpaul::rackplugs::BaconStyle::get()->getColor(
            baconpaul::rackplugs::BaconStyle::DEFAULT_MUTED_LABEL);

        auto fp = fs::path{rack::contextGet()->patch->path};

        nvgBeginPath(vg);
        nvgFontFaceId(vg, memFont);
        nvgFontSize(vg, 12);
        nvgFillColor(vg, labelColor);
        nvgTextAlign(vg, NVG_ALIGN_MIDDLE | NVG_ALIGN_LEFT);

        auto plabel = fp.parent_path().u8string();
        nvgText(vg, 5, -box.size.x * 0.77, plabel.c_str(), nullptr);

        nvgBeginPath(vg);
        nvgFontFaceId(vg, memFont);
        nvgFontSize(vg, 24);
        nvgFillColor(vg, labelColor);
        nvgTextAlign(vg, NVG_ALIGN_MIDDLE | NVG_ALIGN_LEFT);

        auto flabel = fp.filename().u8string();
        nvgText(vg, 5, -box.size.x * 0.30, flabel.c_str(), nullptr);
        nvgRestore(vg);
    };
};

PatchNameDisplayWidget::PatchNameDisplayWidget(PatchNameDisplay *model) : bp::BaconModuleWidget()
{
    setModule(model);
    box.size = Vec(SCREW_WIDTH * 3, RACK_HEIGHT);

    BaconBackground *bg = new BaconBackground(box.size, "Name");
    addChild(bg->wrappedInFramebuffer());

    bdw = new BufferedDrawFunctionWidget(rack::Vec(0, 25), rack::Vec(box.size.x, box.size.y - 50),
                                         [this](auto *vg) { drawLabel(vg); });
    addChild(bdw);
}

Model *modelPatchNameDisplay =
    createModel<PatchNameDisplay, PatchNameDisplayWidget>("PatchNameDisplay");
