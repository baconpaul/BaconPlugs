
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
        updateJSON();
    }

    friend struct StyleParticipant;
    void notifyStyleListeners();

    enum Colors : uint32_t {
        BG,
        BG_END,
        DEFAULT_LABEL,
        DEFAULT_HIGHLIGHT_LABEL
    };

    NVGcolor getColor(Colors c);

  private:
    std::unordered_set<StyleParticipant *> listeners;
    void addStyleListener(StyleParticipant *l) { listeners.insert(l); }
    void removeStyleListener(StyleParticipant *l) { listeners.erase(l); }
    void updateJSON();
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
}

#endif // BACONPLUGS_RACK_HACK_STYLE_H
