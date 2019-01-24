/*
    MANGO Multimedia Development Platform
    Copyright (C) 2012-2019 Twilight Finland 3D Oy Ltd. All rights reserved.
*/
#include <mango/mango.hpp>
#include <mango/framebuffer/framebuffer.hpp>

using namespace mango;
using namespace mango::framebuffer;

class DemoWindow : public Framebuffer
{
protected:
    Timer timer;
    u64 prev_time;
    u64 frames = 0;
    u8 color = 0;

public:
    DemoWindow(int width, int height)
        : Framebuffer(width, height)
    {
        setVisible(true);
        setTitle("[DemoWindow] Initializing...");
        prev_time = timer.us();
    }

    ~DemoWindow()
    {
    }

    void onKeyPress(Keycode key, u32 mask) override
    {
        if (key == KEYCODE_ESC)
            breakEventLoop();
    }

    void onIdle() override
    {
        onDraw();
    }

    void onDraw() override
    {
        u64 time = timer.us();
        u64 diff = time - prev_time;
        ++frames;
        if (diff > 1000000 / 4)
        {
            diff = diff / frames;
            frames = 0;
            prev_time = time;
            std::string text = makeString("[DemoWindow]  time: %.2f ms (%d Hz)", diff / 1000.0f, diff ? 1000000 / diff : 0);
            setTitle(text);
        }

        Surface s = lock();

        float c = float(color) / 255.0f;
        color++;
        s.clear(c, c, c, 1.0f);

        // draw debugging rectangles
        // should be: red, green, blue and white box at top-left corner
        Surface(s,  0, 0, 32, 32).clear(1.0f, 0.0f, 0.0f, 1.0);
        Surface(s, 32, 0, 32, 32).clear(0.0f, 1.0f, 0.0f, 1.0);
        Surface(s, 64, 0, 32, 32).clear(0.0f, 0.0f, 1.0f, 1.0);
        Surface(s, 96, 0, 32, 32).clear(1.0f, 1.0f, 1.0f, 1.0);

        unlock();
        present();
    }
};

int main(int argc, const char* argv[])
{
    DemoWindow demo(640, 480);
    demo.enterEventLoop();
}
