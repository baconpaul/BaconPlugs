#include "BaconPlugs.hpp"
#include <cstdio>
#include <fstream>

#include "BaconModule.hpp"
#include "BaconModuleWidget.h"
#include "blendish.h"
#include "blendish.h"

#define SCALE_LENGTH 12

namespace bp = baconpaul::rackplugs;

struct ContrastBNDEditor : virtual bp::BaconModule
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

    ContrastBNDEditor() : bp::BaconModule()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    }

    void process(const ProcessArgs &args) override {}
};

struct ContrastBNDEditorWidget : bp::BaconModuleWidget
{
    ContrastBNDEditorWidget(ContrastBNDEditor *model);

    int64_t warnC{0}, infoC{0};
    std::vector<std::string> deleteOnExit;
    ~ContrastBNDEditorWidget() {}

    void hiLight()
    {
        auto bt = bndGetTheme();
        auto q = *bt;
        auto wl = 245;
        q.menuItemTheme.textColor = nvgRGB(0, 0, 0);
        q.menuTheme.textColor = nvgRGB(0, 0, 0);
        q.menuTheme.innerColor = nvgRGB(wl, wl, wl);
        q.tooltipTheme.innerSelectedColor = nvgRGB(wl, wl, wl);
        q.tooltipTheme.innerColor = nvgRGB(wl, wl, wl);
        bndSetTheme(q);
    }
    void hiDark()
    {
        auto bt = bndGetTheme();
        auto q = *bt;
        auto wl = 245;
        auto dl = 5;
        q.menuItemTheme.textColor = nvgRGB(wl, wl, wl);
        q.menuTheme.textColor = nvgRGB(wl, wl, wl);
        q.menuTheme.innerColor = nvgRGB(dl, dl, dl);
        q.tooltipTheme.innerSelectedColor = nvgRGB(dl, dl, dl);
        q.tooltipTheme.innerColor = nvgRGB(dl, dl, dl);
        bndSetTheme(q);
    }

    void appendModuleSpecificContextMenu(Menu *menu) override
    {
        menu->addChild(new rack::ui::MenuSeparator);
        menu->addChild(rack::createMenuLabel("Menu Items"));
        menu->addChild(rack::createMenuItem("High Contrast Light", "", [this]() { hiLight(); }));
        menu->addChild(rack::createMenuItem("High Contrast Dark", "", [this]() { hiDark(); }));
    }
};

ContrastBNDEditorWidget::ContrastBNDEditorWidget(ContrastBNDEditor *model) : bp::BaconModuleWidget()
{
    setModule(model);
    box.size = Vec(SCREW_WIDTH * 4, RACK_HEIGHT);

    BaconBackground *bg = new BaconBackground(box.size, "UI");
    addChild(bg->wrappedInFramebuffer());
}

Model *modelContrastBNDEditor =
    createModel<ContrastBNDEditor, ContrastBNDEditorWidget>("ContrastBNDEditor");
