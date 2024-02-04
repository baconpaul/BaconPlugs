#include "rack.hpp"

#include "BufferedDrawFunction.hpp"

#include "Style.hpp"

template <int H> struct GraduatedFader : app::SliderKnob, baconpaul::rackplugs::StyleParticipant
{
    int slider_height = 41;
    int slider_width = 20;
    int widget_width = 28;
    widget::Widget *notches{nullptr}, *handle{nullptr}, *shadow{nullptr};
    widget::FramebufferWidget *fb{nullptr};

    math::Vec minHandlePos, maxHandlePos;

    GraduatedFader()
    {
        maxHandlePos = Vec((widget_width - slider_width) / 2, 0);
        minHandlePos = Vec((widget_width - slider_width) / 2, (H - slider_height));
        box.size = Vec(widget_width, H);

        fb = new widget::FramebufferWidget();
        addChild(fb);

        notches = new BufferedDrawFunctionWidget(Vec(0, 0), box.size,
                                                 [this](auto vg) { drawBackground(vg); });
        fb->addChild(notches);

        shadow = new BufferedDrawFunctionWidget(Vec(0, 0), Vec(20, 41),
                                                [this](auto vg) { drawHandleShadow(vg); });
        fb->addChild(shadow);

        handle = new BufferedDrawFunctionWidget(Vec(0, 0), Vec(20, 41),
                                                [this](auto vg) { drawHandle(vg); });
        fb->addChild(handle);
    }

    void onChange(const event::Change &e) override
    {
        if (getParamQuantity())
        {
            // Interpolate handle position
            float v = getParamQuantity()->getScaledValue();
            handle->box.pos = math::Vec(math::rescale(v, 0.f, 1.f, minHandlePos.x, maxHandlePos.x),
                                        math::rescale(v, 0.f, 1.f, minHandlePos.y, maxHandlePos.y));
            shadow->box.pos.x = handle->box.pos.x + 1.5;
            shadow->box.pos.y = handle->box.pos.y + 3;
            fb->dirty = true;
        }
        ParamWidget::onChange(e);
    }

    void drawHandleShadow(NVGcontext *vg)
    {
        nvgBeginPath(vg);
        nvgRoundedRect(vg, 0, 0, 20, 41, 3);
        nvgFillColor(vg, nvgRGBA(40, 40, 90, 80));
        nvgFill(vg);
    }

    void drawHandle(NVGcontext *vg)
    {
        nvgBeginPath(vg);
        nvgRoundedRect(vg, 0, 0, 20, 21, 3);
        NVGpaint vgr = nvgLinearGradient(vg, 0, 0, 0, 19, nvgRGBA(170, 170, 190, 255),
                                         nvgRGBA(220, 200, 240, 255));
        nvgFillPaint(vg, vgr);
        nvgFill(vg);

        nvgBeginPath(vg);
        nvgRoundedRect(vg, 0, 19, 20, 21, 3);
        vgr = nvgLinearGradient(vg, 0, 41, 0, 21, nvgRGBA(170, 170, 190, 255),
                                nvgRGBA(220, 200, 240, 255));
        nvgFillPaint(vg, vgr);
        nvgFill(vg);

        nvgBeginPath(vg);
        nvgRoundedRect(vg, 0, 0, 20, 41, 3);
        nvgStrokeColor(vg, componentlibrary::SCHEME_BLACK);
        nvgStrokeWidth(vg, 0.5);
        nvgStroke(vg);

        nvgBeginPath(vg);
        nvgRect(vg, 0, 19, 20, 3);
        nvgFillColor(vg, nvgRGB(40, 40, 90));
        nvgFill(vg);
        nvgStrokeColor(vg, componentlibrary::SCHEME_BLACK);
        nvgStrokeWidth(vg, 0.5);
        nvgStroke(vg);
    }

    void drawBackground(NVGcontext *vg)
    {
        auto style = baconpaul::rackplugs::BaconStyle::get();
        int nStrokes = 10;
        int slideTop = slider_height / 2;
        int slideHeight = H - slider_height;
        int slideBump = 5;
        int slotWidth = 1;

        float dx = (1.0 * slideHeight) / nStrokes;

        // and now the black notches
        for (int i = 0; i <= nStrokes; ++i)
        {
            nvgBeginPath(vg);
            nvgMoveTo(vg, 1, slideTop + dx * i);
            nvgLineTo(vg, widget_width - 2, slideTop + dx * i);
            nvgStrokeColor(vg, style->getColor(baconpaul::rackplugs::BaconStyle::SLIDER_NOTCH));
            nvgStrokeWidth(vg, 0.5);
            nvgStroke(vg);
        }

        // OK so now we want to draw the vertical line
        nvgBeginPath(vg);
        nvgRect(vg, widget_width / 2 - slotWidth, slideTop - slideBump, 2 * slotWidth + 1,
                slideHeight + 2 * slideBump);
        nvgFillColor(vg, style->getColor(baconpaul::rackplugs::BaconStyle::SLIDER_SLOT));
        nvgFill(vg);
    }

    void onStyleChanged() override { fb->dirty = true; }
};
