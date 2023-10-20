
#ifndef BACONPLUGS_RACK_HACK_STYLE_H
#define BACONPLUGS_RACK_HACK_STYLE_H

#include <unordered_set>
#include <rack.hpp>

namespace baconpaul::rackplugs
{
struct StyleConstants
{
    static constexpr float rectRadius{2};
};

struct StyleParticipant;

struct BaconStyle
{
    static std::shared_ptr<BaconStyle> get()
    {
        if (!stylePtr)
        {
            stylePtr = std::make_shared<BaconStyle>();
        }
        return stylePtr;
    }

    BaconStyle();

    enum Style
    {
        DARK = 10001, // just so it's not a 0 in the JSON
        LIGHT
    };
    Style activeStyle{LIGHT};
    void setStyle(Style s)
    {
        activeStyle = s;
        notifyStyleListeners();
    }

    friend struct StyleParticipant;
    void notifyStyleListeners();

    enum Colors : uint32_t
    {
        BG,
        BG_END,
        DEFAULT_LABEL,
        DEFAULT_MUTED_LABEL,
        DEFAULT_HIGHLIGHT_LABEL,

        LABEL_BG,
        LABEL_BG_END,
        LABEL_RULE,

        MODULE_OUTLINE,

        INPUT_BG,
        INPUT_BG_END,

        HIGHLIGHT_BG,
        HIGHLIGHT_BG_END,

        SECTION_RULE_LINE,

        LIGHT_BG,

        SLIDER_NOTCH,
        SLIDER_SLOT
    };

    NVGcolor getColor(Colors c);
    std::string fontName() { return "res/Monitorica-Bd.ttf"; }
    std::string monoFontName() { return "res/FiraMono-Regular.ttf"; }

  private:
    std::unordered_set<StyleParticipant *> listeners;
    void addStyleListener(StyleParticipant *l) { listeners.insert(l); }
    void removeStyleListener(StyleParticipant *l) { listeners.erase(l); }
    static std::shared_ptr<BaconStyle> stylePtr;
};

struct StyleParticipant
{
    StyleParticipant() { BaconStyle::get()->addStyleListener(this); }
    virtual ~StyleParticipant() { BaconStyle::get()->removeStyleListener(this); }
    virtual void onStyleChanged() = 0;
};

inline void BaconStyle::notifyStyleListeners()
{
    for (auto l : listeners)
        l->onStyleChanged();
}
} // namespace baconpaul::rackplugs

#endif // BACONPLUGS_RACK_HACK_STYLE_H
