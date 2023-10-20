//
// Created by Paul Walker on 10/28/22.
//

#ifndef BACONPLUGS_RACK_HACK_BACONMODULEWIDGET_H
#define BACONPLUGS_RACK_HACK_BACONMODULEWIDGET_H

#include <rack.hpp>
#include "Style.hpp"

namespace baconpaul::rackplugs
{
struct BaconModuleWidget : rack::app::ModuleWidget, StyleParticipant
{
    virtual void onStyleChanged() override { dirtyFB(this); }

    void dirtyFB(rack::Widget *w)
    {
        auto f = dynamic_cast<rack::FramebufferWidget *>(w);
        if (f)
            f->dirty = true;
        for (auto c : w->children)
            dirtyFB(c);
    }

    void appendContextMenu(ui::Menu *menu) override {
        appendModuleSpecificContextMenu(menu);
    }

    virtual void appendModuleSpecificContextMenu(Menu *)
    {

    }


    void step() override
    {
#ifndef USING_CARDINAL_NOT_RACK
        auto lpd = rack::settings::preferDarkPanels;
        if (lpd)
            BaconStyle::get()->setStyle(BaconStyle::DARK);
        else
            BaconStyle::get()->setStyle(BaconStyle::LIGHT);
#endif

        ModuleWidget::step();
    }

};
}

#endif // BACONPLUGS_RACK_HACK_BACONMODULEWIDGET_H
