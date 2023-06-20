//
// Created by Paul Walker on 10/28/22.
//

#include "Style.hpp"
#include "rack.hpp"

namespace baconpaul::rackplugs
{
std::shared_ptr<BaconStyle> BaconStyle::stylePtr{nullptr};

BaconStyle::BaconStyle()
{
    std::string defaultsDir = rack::asset::user("BaconMusic/");
    if (!rack::system::isDirectory(defaultsDir))
        rack::system::createDirectory(defaultsDir);
    std::string defaultsFile = rack::asset::user("BaconMusic/default-skin.json");

    json_error_t error;
    json_t *fd = json_load_file(defaultsFile.c_str(), 0, &error);
    if (!fd)
    {
        setStyle(LIGHT);
    }
    else
    {
        auto as = json_object_get(fd, "activeStyle");
        if (!as)
        {
            setStyle(LIGHT);
        }
        else
        {
            auto iv = json_integer_value(as);
            if (iv == LIGHT || iv == DARK)
            {
                setStyle((Style)iv);
            }
            else
            {
                setStyle(LIGHT);
            }
        }
    }
}

NVGcolor getColorLight(baconpaul::rackplugs::BaconStyle::Colors c)
{
    switch (c)
    {
    case BaconStyle::BG:
        return nvgRGB(0xF6, 0xF0, 0xED);
    case BaconStyle::DEFAULT_HIGHLIGHT_LABEL:
    case BaconStyle::BG_END:
        return nvgRGB(0xF6, 0XF5, 0xF3);
    case BaconStyle::DEFAULT_LABEL:
        return nvgRGB(10, 10, 15);
    case BaconStyle::DEFAULT_MUTED_LABEL:
        return nvgRGB(70, 70, 75);

    case BaconStyle::INPUT_BG:
        return nvgRGB(0xE6, 0xE0, 0xDD);
    case BaconStyle::INPUT_BG_END:
        return nvgRGB(0xE0, 0xD3, 0xD0);

    case BaconStyle::LABEL_BG:
        return nvgRGB(0xE6, 0xE0, 0XDD);
    case BaconStyle::LABEL_BG_END:
        return nvgRGB(0xE6, 0xE6, 0xE3);
    case BaconStyle::MODULE_OUTLINE:
    case BaconStyle::LABEL_RULE:
        return nvgRGB(0xC0, 0xC0, 0xD0);
    case BaconStyle::SECTION_RULE_LINE:
        return nvgRGB(0x10, 0x10, 0x15);

        // fixme - make more wheaty
    case BaconStyle::HIGHLIGHT_BG:
        return nvgRGB(70, 70, 100);

    case BaconStyle::HIGHLIGHT_BG_END:
        return nvgRGB(60, 60, 120);

    case BaconStyle::SLIDER_NOTCH:
    case BaconStyle::SLIDER_SLOT:
        return nvgRGB(0, 0, 40);

    case BaconStyle::LIGHT_BG:
        return nvgRGB(200, 200, 220);


    case BaconStyle::LED_TEXT_COLOR:
        return nvgRGB(0x10, 0x10, 0x20);
    case BaconStyle::LED_BACKGROUND_COLOR:
        return nvgRGB(0xD0, 0xD0, 0xE0);
    }
    return nvgRGBA(255, 0, 0, 255);
}

NVGcolor getColorDark(baconpaul::rackplugs::BaconStyle::Colors c)
{
    switch (c)
    {
    case BaconStyle::BG:
        return nvgRGB(50, 50, 55);
    case BaconStyle::DEFAULT_HIGHLIGHT_LABEL:
        return nvgRGB(0xF0, 0xF0, 0xFF);
    case BaconStyle::BG_END:
        return nvgRGB(60, 60, 75);
    case BaconStyle::DEFAULT_LABEL:
        return nvgRGB(0xF0, 0xF0, 0xFF);
    case BaconStyle::DEFAULT_MUTED_LABEL:
        return nvgRGB(0xC0, 0xC0, 0xCF);

    case BaconStyle::INPUT_BG:
        return nvgRGB(45, 45, 55);
    case BaconStyle::INPUT_BG_END:
        return nvgRGB(55, 55, 70);

    case BaconStyle::LABEL_BG:
        return nvgRGB(20, 20, 25);
    case BaconStyle::LABEL_BG_END:
        return nvgRGB(25, 25, 30);
    case BaconStyle::LABEL_RULE:
    case BaconStyle::MODULE_OUTLINE:
        return nvgRGB(80, 80, 90);
    case BaconStyle::SECTION_RULE_LINE:
        return nvgRGB(0xA0, 0xA0, 0xA5);

    case BaconStyle::HIGHLIGHT_BG:
        return nvgRGB(90, 90, 90);

    case BaconStyle::HIGHLIGHT_BG_END:
        return nvgRGB(90, 90, 100);

    case BaconStyle::SLIDER_NOTCH:
        return nvgRGB(120, 120, 140);
    case BaconStyle::SLIDER_SLOT:
        return nvgRGB(0, 0, 40);

    case BaconStyle::LIGHT_BG:
        return nvgRGB(20, 20, 30);

    case BaconStyle::LED_TEXT_COLOR:
        return nvgRGB(0xD0, 0xD0, 0xFF);
    case BaconStyle::LED_BACKGROUND_COLOR:
        return nvgRGB(20,20,30);
    }
    return nvgRGBA(255, 0, 0, 255);
}

NVGcolor BaconStyle::getColor(baconpaul::rackplugs::BaconStyle::Colors c)
{
    if (activeStyle == LIGHT)
        return getColorLight(c);
    return getColorDark(c);
}

void BaconStyle::updateJSON()
{
    std::string defaultsDir = rack::asset::user("BaconMusic/");
    if (!rack::system::isDirectory(defaultsDir))
        rack::system::createDirectory(defaultsDir);
    std::string defaultsFile = rack::asset::user("BaconMusic/default-skin.json");

    json_t *rootJ = json_object();
    json_t *stJ = json_integer(activeStyle);
    json_object_set_new(rootJ, "activeStyle", stJ);
    FILE *f = std::fopen(defaultsFile.c_str(), "w");
    if (f)
    {
        json_dumpf(rootJ, f, JSON_INDENT(2));
        std::fclose(f);
    }
    json_decref(rootJ);
}

} // namespace baconpaul::rackplugs