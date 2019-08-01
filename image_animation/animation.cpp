/*
    MANGO Multimedia Development Platform
    Copyright (C) 2012-2019 Twilight Finland 3D Oy Ltd. All rights reserved.
*/
#include <mango/mango.hpp>
#include <mango/framebuffer/framebuffer.hpp>

using namespace mango;
using namespace mango::filesystem;
using namespace mango::framebuffer;

struct ImageAnimation
{
    File m_file;
    ImageDecoder m_decoder;
    ImageHeader m_header;
    Bitmap m_bitmap;

    ImageAnimation(const std::string& filename)
        : m_file(filename)
        , m_decoder(m_file, filename)
        , m_header(m_decoder.header())
        , m_bitmap(m_header.width, m_header.height, m_header.format)
    {
    }

    void decode()
    {
        m_decoder.decode(m_bitmap);
    }
};

class DemoWindow : public Framebuffer
{
protected:
    ImageAnimation& m_animation;
    Timer timer;
    u64 prev_time;

public:
    DemoWindow(ImageAnimation& animation)
        : Framebuffer(animation.m_bitmap.width, animation.m_bitmap.height)
        , m_animation(animation)
    {
        setVisible(true);
        setTitle("[DemoWindow]");
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
        u64 time = timer.ms();
        u64 diff = time - prev_time;
        if (diff > 60)
        {
            prev_time = time;

            Surface s = lock();

            m_animation.decode();
            s.blit(0, 0, m_animation.m_bitmap);

            unlock();
            present();
        }
    }
};

int main(int argc, const char* argv[])
{
    std::string filename = "images/dude.gif";
    if (argc > 1)
    {
        filename = argv[1];
    }

    ImageAnimation animation(filename);
    DemoWindow demo(animation);
    demo.enterEventLoop();
}
