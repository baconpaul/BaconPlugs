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

struct ContrastBNDEditorWidget : bp::BaconModuleWidget
{
    ContrastBNDEditorWidget(ContrastBNDEditor *model);

    int64_t warnC{0}, infoC{0};
    std::vector<std::string> deleteOnExit;
    ~ContrastBNDEditorWidget() {}

    void bndDef() { rack::ui::setTheme(nvgRGB(0x33, 0x33, 0x33), nvgRGB(0xf0, 0xf0, 0xf0)); }

    void hiLight()
    {
        auto bt = bndGetTheme();
        auto q = *bt;
        auto wl = 0;
        auto dl = 245;

        q.menuItemTheme.textColor = nvgRGB(wl, wl, wl);
        q.menuItemTheme.innerColor = nvgRGB(dl, dl, dl);
        q.menuItemTheme.textSelectedColor = nvgRGB(dl, dl, dl);
        q.menuItemTheme.innerSelectedColor = nvgRGB(wl, wl, wl);

        q.menuTheme.textColor = nvgRGB(0, 0, 20);
        q.menuTheme.innerColor = nvgRGB(dl, dl, dl);
        q.menuTheme.textSelectedColor = nvgRGB(dl * 0.9, dl * 0.9, dl);

        q.tooltipTheme.innerSelectedColor = nvgRGB(dl, dl, dl);
        q.tooltipTheme.innerColor = nvgRGB(dl, dl, dl);

        bndSetTheme(q);
    }
    void hiDark()
    {
        auto bt = bndGetTheme();
        auto q = *bt;
        auto wl = 245;
        auto dl = 5;
        q.menuItemTheme.textColor = nvgRGB(wl, wl, wl);
        q.menuItemTheme.innerColor = nvgRGB(dl, dl, dl);
        q.menuItemTheme.textSelectedColor = nvgRGB(dl, dl, dl);
        q.menuItemTheme.innerSelectedColor = nvgRGB(wl, wl, wl);

        q.menuTheme.textColor = nvgRGB(wl * 0.95, wl * 0.95, wl);
        q.menuTheme.innerColor = nvgRGB(dl, dl, dl);
        q.menuTheme.textSelectedColor = nvgRGB(dl, dl, 20);

        q.tooltipTheme.innerSelectedColor = nvgRGB(dl, dl, dl);
        q.tooltipTheme.innerColor = nvgRGB(dl, dl, dl);
        bndSetTheme(q);
    }

    void appendModuleSpecificContextMenu(Menu *menu) override
    {
        auto c = dynamic_cast<ContrastBNDEditor *>(module);

        if (c)
        {
            menu->addChild(new rack::ui::MenuSeparator);
            menu->addChild(rack::createMenuLabel("Menu Items"));
            menu->addChild(rack::createMenuItem(
                "Default", CHECKMARK(c->colorScheme == ContrastBNDEditor::DEFAULT),
                [c]() { c->colorScheme = ContrastBNDEditor::ColorScheme::DEFAULT; }));
            menu->addChild(rack::createMenuItem(
                "High Contrast Light",
                CHECKMARK(c->colorScheme == ContrastBNDEditor::HICONTRAST_LIGHT),
                [c]() { c->colorScheme = ContrastBNDEditor::ColorScheme::HICONTRAST_LIGHT; }));
            menu->addChild(rack::createMenuItem(
                "High Contrast Dark",
                CHECKMARK(c->colorScheme == ContrastBNDEditor::HICONTRAST_DARK),
                [c]() { c->colorScheme = ContrastBNDEditor::ColorScheme::HICONTRAST_DARK; }));
        }
    }

    int lastScheme{-1};
    void step() override
    {
        if (module)
        {
            auto c = dynamic_cast<ContrastBNDEditor *>(module);
            if (c)
            {
                if (lastScheme != c->colorScheme)
                {
                    lastScheme = c->colorScheme;
                    switch (c->colorScheme)
                    {
                    case ContrastBNDEditor::DEFAULT:
                        bndDef();
                        break;
                    case ContrastBNDEditor::HICONTRAST_LIGHT:
                        hiLight();
                        break;
                    case ContrastBNDEditor::HICONTRAST_DARK:
                        hiDark();
                        break;
                    }
                }
            }
        }
        Widget::step();
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
