#ifndef BUFFERED_DRAW_FUNCTION_INCLUDE
#define BUFFERED_DRAW_FUNCTION_INCLUDE

#include "rack.hpp"

#include <functional>
#include <vector>

using namespace rack;

template <typename T>
struct BufferedDrawFunctionWidget : virtual FramebufferWidget /* widget::Widget*/
{
    typedef std::function<void(T *, NVGcontext *)> drawfn_t;

    // Put this in when I change baseclass to compile at least
    // bool dirty = true;

    T *that;
    drawfn_t drawf;

    struct InternalBDW : Widget
    {
        T *that;
        drawfn_t drawf;
        InternalBDW(Rect box_, T *that_, drawfn_t draw_) : that(that_), drawf(draw_) { box = box_; }
        void draw(const DrawArgs &args) override { drawf(that, args.vg); }
    };

    BufferedDrawFunctionWidget(Vec pos, Vec sz, T *that_, drawfn_t draw_)
        : that(that_), drawf(draw_)
    {
        box.pos = pos;
        box.size = sz;
        auto kidBox = Rect(Vec(0, 0), box.size);
        InternalBDW *kid = new InternalBDW(kidBox, that, drawf);
        addChild(kid);
    }
};

struct BufferedDrawLambdaWidget : virtual FramebufferWidget
{
    typedef std::function<void(NVGcontext *)> drawfn_t;
    drawfn_t drawf;

    struct InternalBDW : TransparentWidget
    {
        drawfn_t drawf;
        InternalBDW(Rect box_, drawfn_t draw_) : drawf(draw_) { box = box_; }
        void draw(const DrawArgs &args) override { drawf(args.vg); }
    };

    BufferedDrawLambdaWidget(Vec pos, Vec sz, drawfn_t draw_) : drawf(draw_)
    {
        box.pos = pos;
        box.size = sz;
        auto kidBox = Rect(Vec(0, 0), box.size);
        InternalBDW *kid = new InternalBDW(kidBox, drawf);
        addChild(kid);
    }
};

#endif
