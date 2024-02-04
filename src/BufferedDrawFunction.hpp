#ifndef BUFFERED_DRAW_FUNCTION_INCLUDE
#define BUFFERED_DRAW_FUNCTION_INCLUDE

#include "rack.hpp"

#include <functional>
#include <vector>
#include <string>
#include <unordered_map>
#include <iostream>

using namespace rack;

extern std::unordered_map<std::string, int> memDebugger;

struct BufferedDrawFunctionWidget : virtual FramebufferWidget /* widget::Widget*/
{
    typedef std::function<void(NVGcontext *)> drawfn_t;

    drawfn_t drawf;

    struct InternalBDW : Widget
    {
        drawfn_t drawf;
        InternalBDW(Rect box_, drawfn_t draw_) : drawf(draw_) { box = box_; }
        void draw(const DrawArgs &args) override { drawf(args.vg); }
    };

    BufferedDrawFunctionWidget(Vec pos, Vec sz, drawfn_t draw_) : drawf(draw_)
    {
        box.pos = pos;
        box.size = sz;
        auto kidBox = Rect(Vec(0, 0), box.size);
        InternalBDW *kid = new InternalBDW(kidBox, drawf);
        addChild(kid);
#if DEBUG_MEM
        memDebugger["bdw"]++;
        std::cout << "CTOR >> Outstanding BDWs = " << memDebugger["bdw"] << std::endl;
#endif
    }

#if DEBUG_MEM
    ~BufferedDrawFunctionWidget() override
    {
        memDebugger["bdw"]--;
        std::cout << "DTOR << Outstanding BDWs = " << memDebugger["bdw"] << std::endl;
    }
#endif
};

struct BufferedDrawFunctionWidgetOnLayer : BufferedDrawFunctionWidget
{
    int layer{1};
    BufferedDrawFunctionWidgetOnLayer(rack::Vec pos, rack::Vec sz, drawfn_t draw_, int ly = 1)
        : BufferedDrawFunctionWidget(pos, sz, draw_), layer(ly)
    {
    }

    void draw(const DrawArgs &args) override { return; }
    void drawLayer(const DrawArgs &args, int dl) override
    {
        if (dl == layer)
        {
            BufferedDrawFunctionWidget::draw(args);
        }
    }
};

#endif
