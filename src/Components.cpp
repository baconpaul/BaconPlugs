#include "BaconPlugs.hpp"
#include <jansson.h>

std::map<std::string, int> InternalFontMgr::fontMap;

struct InternalRoundedBorder : virtual TransparentWidget
{
    bool doFill;
    NVGcolor fillColor;

    InternalRoundedBorder(Vec pos, Vec sz, NVGcolor fc)
    {
        box.pos = pos;
        box.size = sz;
        doFill = true;
        fillColor = fc;
    }

    InternalRoundedBorder(Vec pos, Vec sz)
    {
        box.pos = pos;
        box.size = sz;
        doFill = false;
    }

    void draw(const DrawArgs &args) override
    {
        nvgBeginPath(args.vg);
        nvgRoundedRect(args.vg, 0, 0, box.size.x, box.size.y, baconpaul::rackplugs::StyleConstants::rectRadius);
        if (doFill)
        {
            nvgFillColor(args.vg, fillColor);
            nvgFill(args.vg);
        }

        nvgStrokeColor(args.vg, componentlibrary::SCHEME_BLACK);
        nvgStroke(args.vg);
    }
};

struct InternalTextLabel : virtual TransparentWidget, baconpaul::rackplugs::StyleParticipant
{
    int memFont = -1;
    std::string label;
    int pxSize;
    int align;
    baconpaul::rackplugs::BaconStyle::Colors color;

    InternalTextLabel(Vec pos, const char *lab, int px, int al,
                      baconpaul::rackplugs::BaconStyle::Colors col)
        : label(lab), pxSize(px), align(al), color(col)
    {
        box.pos = pos;
    }

    void draw(const DrawArgs &args) override
    {
        if (memFont < 0)
            memFont = InternalFontMgr::get(args.vg, "res/Monitorica-Bd.ttf");

        auto col = baconpaul::rackplugs::BaconStyle::get()->getColor(color);
        nvgBeginPath(args.vg);
        nvgFontFaceId(args.vg, memFont);
        nvgFontSize(args.vg, pxSize);
        nvgFillColor(args.vg, col);
        nvgTextAlign(args.vg, align);
        nvgText(args.vg, 0, 0, label.c_str(), NULL);
    }

    void onStyleChanged() override
    {}
};

struct InternalPlugLabel : virtual TransparentWidget
{
    int memFont = -1;

    BaconBackground::LabelStyle st;
    BaconBackground::LabelAt at;
    std::string label;

    InternalPlugLabel(Vec portPos, BaconBackground::LabelAt l, BaconBackground::LabelStyle s,
                      const char *ilabel);

    void draw(const DrawArgs &args) override;
};

typedef std::shared_ptr<rack::Svg> svg_t;
static svg_t baconEmoji = nullptr;
static svg_t lovebaconEmoji = nullptr;

void BaconBackground::draw(const DrawArgs &args)
{
    auto style = baconpaul::rackplugs::BaconStyle::get();
    if (memFont < 0)
        memFont = InternalFontMgr::get(args.vg, "res/Monitorica-Bd.ttf");

    if (baconEmoji == nullptr)
    {
        baconEmoji = APP->window->loadSvg(rack::asset::plugin(pluginInstance, "res/1f953.svg"));
    }
    if (lovebaconEmoji == nullptr)
    {
        lovebaconEmoji = APP->window->loadSvg(rack::asset::plugin(pluginInstance, "res/1f60d.svg"));
    }

    auto bg = style->getColor(baconpaul::rackplugs::BaconStyle::BG);
    auto bgEnd = style->getColor(baconpaul::rackplugs::BaconStyle::BG_END);
    nvgBeginPath(args.vg);
    nvgRect(args.vg, 0, 0, box.size.x, box.size.y);
    NVGpaint vgr = nvgLinearGradient(args.vg, 0, 0, 0, box.size.y, bg, bgEnd);
    nvgFillPaint(args.vg, vgr);
    nvgFill(args.vg);

    // Standard Footer
    float rulePos = 360;
    nvgBeginPath(args.vg);
    nvgRect(args.vg, 0, rulePos, box.size.x, box.size.y - rulePos);
    vgr = nvgLinearGradient(args.vg, 0, rulePos, 0, box.size.y, labelBgEnd, labelBg);
    nvgFillPaint(args.vg, vgr);
    nvgFill(args.vg);

    nvgBeginPath(args.vg);
    nvgMoveTo(args.vg, 0, rulePos);
    nvgLineTo(args.vg, box.size.x, rulePos);
    nvgStrokeColor(args.vg, BaconBackground::labelRule);
    nvgStroke(args.vg);

    float endB = box.size.x / 2, startM = box.size.x / 2;
    svg_t svgu = baconEmoji;
    if (showSmiles)
        svgu = lovebaconEmoji;
    if (svgu != nullptr && svgu->handle)
    {
        float scaleFactor = 1.0 * (box.size.y - rulePos - 4) / svgu->handle->height;
        float x0 = box.size.x / 2 - svgu->handle->width * scaleFactor / 2;

        if (box.size.x < 5.5 * SCREW_WIDTH)
        {
            x0 = box.size.x - 2 - svgu->handle->width * scaleFactor;
        }

        endB = x0 - 2;
        startM = x0 + svgu->handle->width * scaleFactor + 2;
        nvgSave(args.vg);
        nvgTranslate(args.vg, x0, rulePos + 2);
        nvgScale(args.vg, scaleFactor, scaleFactor);
        rack::svgDraw(args.vg, svgu->handle);
        nvgRestore(args.vg);

        baconBox.pos = rack::Vec(x0, rulePos + 2);
        baconBox.size =
            rack::Vec(svgu->handle->width * scaleFactor, svgu->handle->height * scaleFactor);
    }

    if (box.size.x > 5 * SCREW_WIDTH)
    {
        nvgBeginPath(args.vg);
        nvgFontFaceId(args.vg, memFont);
        nvgFontSize(args.vg, 14);
        nvgFillColor(args.vg, componentlibrary::SCHEME_BLACK);
        nvgStrokeColor(args.vg, componentlibrary::SCHEME_BLACK);
        nvgTextAlign(args.vg, NVG_ALIGN_RIGHT | NVG_ALIGN_BOTTOM);
        nvgText(args.vg, endB, box.size.y - 3, "Bacon", NULL);
        nvgTextAlign(args.vg, NVG_ALIGN_LEFT | NVG_ALIGN_BOTTOM);
        nvgText(args.vg, startM, box.size.y - 3, "Music", NULL);
    }
    else
    {
        nvgBeginPath(args.vg);
        nvgFontFaceId(args.vg, memFont);
        nvgFontSize(args.vg, 14);
        nvgFillColor(args.vg, componentlibrary::SCHEME_BLACK);
        nvgStrokeColor(args.vg, componentlibrary::SCHEME_BLACK);
        nvgTextAlign(args.vg, NVG_ALIGN_LEFT | NVG_ALIGN_BOTTOM);
        nvgText(args.vg, 3, box.size.y - 3, "Bacon", NULL);
    }

    // Header BG
    rulePos = 22;
    nvgBeginPath(args.vg);
    nvgRect(args.vg, 0, 0, box.size.x, rulePos);
    vgr = nvgLinearGradient(args.vg, 0, 0, 0, rulePos, labelBgEnd, labelBg);

    nvgFillPaint(args.vg, vgr);
    nvgFill(args.vg);

    nvgBeginPath(args.vg);
    nvgMoveTo(args.vg, 1, rulePos);
    nvgLineTo(args.vg, box.size.x, rulePos);
    nvgStrokeColor(args.vg, BaconBackground::labelRule);
    nvgStroke(args.vg);

    // Header label
    nvgBeginPath(args.vg);
    nvgFontFaceId(args.vg, memFont);
    nvgFontSize(args.vg, 16);
    nvgFillColor(args.vg, componentlibrary::SCHEME_BLACK);
    nvgStrokeColor(args.vg, componentlibrary::SCHEME_BLACK);
    nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);
    nvgText(args.vg, box.size.x / 2, 5, title.c_str(), NULL);

    nvgBeginPath(args.vg);
    nvgMoveTo(args.vg, 0, 0);
    nvgLineTo(args.vg, box.size.x, 0);
    nvgLineTo(args.vg, box.size.x, box.size.y);
    nvgLineTo(args.vg, 0, box.size.y);
    nvgLineTo(args.vg, 0, 0);
    nvgStrokeColor(args.vg, BaconBackground::bgOutline);
    nvgStroke(args.vg);

    if (extraDrawFunction != nullptr)
        extraDrawFunction(args.vg);

    for (auto w : children)
    {
        nvgTranslate(args.vg, w->box.pos.x, w->box.pos.y);
        w->draw(args);
        nvgTranslate(args.vg, -w->box.pos.x, -w->box.pos.y);
    }

    for (auto it = rects.begin(); it != rects.end(); ++it)
    {
        col_rect_t tu = *it;
        Rect r = std::get<0>(tu);
        NVGcolor c = std::get<1>(tu);
        bool f = std::get<2>(tu);
        nvgBeginPath(args.vg);
        nvgRect(args.vg, r.pos.x, r.pos.y, r.size.x, r.size.y);
        if (f)
        {
            nvgFillColor(args.vg, c);
            nvgFill(args.vg);
        }
        else
        {
            nvgStrokeColor(args.vg, c);
            nvgStroke(args.vg);
        }
    }
}

InternalPlugLabel::InternalPlugLabel(Vec portPos, BaconBackground::LabelAt l,
                                     BaconBackground::LabelStyle s, const char *ilabel)
    : st(s), at(l), label(ilabel)
{
    box.size.x = 24 + 5;
    box.size.y = 24 + 5 + 20;

    // switch on position but for now just do above
    box.pos.x = portPos.x - 2.5;
    box.pos.y = portPos.y - 2.5 - 17;
}

void BaconBackground::onButton(const event::Button &e)
{
    showSmiles = false;
    if (e.action == GLFW_PRESS && baconBox.isContaining(e.pos))
        showSmiles = true;
    auto *r = dynamic_cast<FramebufferWidget *>(parent);
    if (r != nullptr)
    {
        r->dirty = true;
    }
}

void InternalPlugLabel::draw(const DrawArgs &args)
{
    if (memFont < 0)
        memFont = InternalFontMgr::get(args.vg, "res/Monitorica-Bd.ttf");

    NVGcolor txtCol = componentlibrary::SCHEME_BLACK;

    switch (st)
    {
    case (BaconBackground::SIG_IN):
    {
        nvgBeginPath(args.vg);
        nvgRoundedRect(args.vg, 0, 0, box.size.x, box.size.y, baconpaul::rackplugs::StyleConstants::rectRadius);
        NVGpaint vgr = nvgLinearGradient(args.vg, 0, 0, 0, box.size.y, BaconBackground::inputStart,
                                         BaconBackground::inputEnd);
        nvgFillPaint(args.vg, vgr);
        nvgFill(args.vg);
        nvgStrokeColor(args.vg, componentlibrary::SCHEME_BLACK);
        nvgStroke(args.vg);
        break;
    }
    case (BaconBackground::SIG_OUT):
    {
        nvgBeginPath(args.vg);
        nvgRoundedRect(args.vg, 0, 0, box.size.x, box.size.y, baconpaul::rackplugs::StyleConstants::rectRadius);
        NVGpaint vgr = nvgLinearGradient(args.vg, 0, 0, 0, box.size.y, BaconBackground::highlight,
                                         BaconBackground::highlightEnd);
        nvgFillPaint(args.vg, vgr);
        nvgFill(args.vg);

        nvgStrokeColor(args.vg, componentlibrary::SCHEME_BLACK);
        nvgStroke(args.vg);

        txtCol = componentlibrary::SCHEME_WHITE;
        break;
    }
    case (BaconBackground::OTHER):
    {
        nvgBeginPath(args.vg);
        nvgRoundedRect(args.vg, 0, 0, box.size.x, box.size.y, baconpaul::rackplugs::StyleConstants::rectRadius);
        nvgStrokeColor(args.vg, componentlibrary::SCHEME_RED);
        nvgStroke(args.vg);
        break;
    }
    }

    nvgFontFaceId(args.vg, memFont);
    nvgFontSize(args.vg, 13);
    nvgFillColor(args.vg, txtCol);
    nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);
    nvgText(args.vg, box.size.x / 2, 3, label.c_str(), NULL);
}

BaconBackground *BaconBackground::addLabel(Vec pos, const char *lab, int px, int align,
                                           baconpaul::rackplugs::BaconStyle::Colors col)
{
    addChild(new InternalTextLabel(pos, lab, px, align, col));
    return this;
}

BaconBackground *BaconBackground::addPlugLabel(Vec plugPos, LabelAt l, LabelStyle s,
                                               const char *ilabel)
{
    addChild(new InternalPlugLabel(plugPos, l, s, ilabel));
    return this;
}

BaconBackground *BaconBackground::addRoundedBorder(Vec pos, Vec sz)
{
    addChild(new InternalRoundedBorder(pos, sz));
    return this;
}

BaconBackground *BaconBackground::addRoundedBorder(Vec pos, Vec sz, NVGcolor fill)
{
    addChild(new InternalRoundedBorder(pos, sz, fill));
    return this;
}

NVGcolor BaconBackground::bgOutline = nvgRGBA(120, 120, 160, 255);
NVGcolor BaconBackground::highlight = nvgRGBA(70, 70, 100, 255);
NVGcolor BaconBackground::highlightEnd = nvgRGBA(60, 60, 120, 255);

NVGcolor BaconBackground::inputStart = nvgRGBA(225, 225, 255, 255);
NVGcolor BaconBackground::inputEnd = nvgRGBA(225, 225, 255, 255);

NVGcolor BaconBackground::labelBg = nvgRGBA(170, 170, 190, 255);
NVGcolor BaconBackground::labelBgEnd = nvgRGBA(220, 220, 240, 255);
NVGcolor BaconBackground::labelRule = nvgRGBA(120, 120, 190, 255);

BaconBackground::BaconBackground(Vec size, const char *lab) : title(lab)
{
    box.pos = Vec(0, 0);
    box.size = size;
    baconBox.pos = Vec(0, 0);
    baconBox.size = Vec(0, 0);
}

FramebufferWidget *BaconBackground::wrappedInFramebuffer()
{
    FramebufferWidget *fb = new FramebufferWidget();
    fb->box = box;
    fb->addChild(this);
    return fb;
}

BaconBackground *BaconBackground::addLabelsForHugeKnob(Vec topLabelPos, const char *knobLabel,
                                                       const char *zeroLabel, const char *oneLabel,
                                                       Vec &putKnobHere)
{
    addLabel(topLabelPos, knobLabel, 14, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
    addLabel(Vec(topLabelPos.x + 10, topLabelPos.y + 72), oneLabel, 13,
             NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
    addLabel(Vec(topLabelPos.x - 10, topLabelPos.y + 72), zeroLabel, 13,
             NVG_ALIGN_RIGHT | NVG_ALIGN_TOP);
    putKnobHere.y = topLabelPos.y + 10;
    putKnobHere.x = topLabelPos.x - 28;
    return this;
}

BaconBackground *BaconBackground::addLabelsForLargeKnob(Vec topLabelPos, const char *knobLabel,
                                                        const char *zeroLabel, const char *oneLabel,
                                                        Vec &putKnobHere)
{
    addLabel(topLabelPos, knobLabel, 14, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
    addLabel(Vec(topLabelPos.x + 10, topLabelPos.y + 48), oneLabel, 13,
             NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
    addLabel(Vec(topLabelPos.x - 10, topLabelPos.y + 48), zeroLabel, 13,
             NVG_ALIGN_RIGHT | NVG_ALIGN_TOP);
    putKnobHere.y = topLabelPos.y + 10;
    putKnobHere.x = topLabelPos.x - 18;
    return this;
}
