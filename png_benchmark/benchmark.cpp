/*
    MANGO Multimedia Development Platform
    Copyright (C) 2012-2018 Twilight Finland 3D Oy Ltd. All rights reserved.
*/
#include <mango/mango.hpp>

using namespace mango;

// ----------------------------------------------------------------------
// warmup()
// ----------------------------------------------------------------------

void warmup(const char* filename)
{
    File file(filename);
    Memory memory = file;
    std::vector<char> buffer(memory.size);
    std::memcpy(buffer.data(), memory.address, memory.size);
}

// ----------------------------------------------------------------------
// spng
// ----------------------------------------------------------------------

// https://libspng.org/

#include "spng.h"

Surface load_spng(const char* filename)
{
    Surface s(0, 0, Format(), 0, nullptr);

    FILE *png;
    char *pngbuf;
    png = fopen(filename, "r");

    if(png==NULL)
    {
        printf("error opening input file %s\n", filename);
        return s;
    }

    fseek(png, 0, SEEK_END);

    long siz_pngbuf = ftell(png);
    rewind(png);

    if(siz_pngbuf < 1)
        return s;

    pngbuf = (char*)malloc(siz_pngbuf);
    if(pngbuf==NULL)
    {
        printf("malloc() failed\n");
        return s;
    }

    if(fread(pngbuf, siz_pngbuf, 1, png) != 1)
    {
        printf("fread() failed\n");
        return s;
    }

    spng_ctx *ctx = spng_ctx_new(0);
    if(ctx == NULL)
    {
        printf("spng_ctx_new() failed\n");
        return s;
    }

    int r;
    r = spng_set_png_buffer(ctx, pngbuf, siz_pngbuf);

    if(r)
    {
        printf("spng_set_png_buffer() error: %s\n", spng_strerror(r));
        return s;
    }

    struct spng_ihdr ihdr;
    r = spng_get_ihdr(ctx, &ihdr);

    if(r)
    {
        printf("spng_get_ihdr() error: %s\n", spng_strerror(r));
        return s;
    }

#if 0
    const char *clr_type_str;
    if(ihdr.color_type == SPNG_COLOR_TYPE_GRAYSCALE)
        clr_type_str = "grayscale";
    else if(ihdr.color_type == SPNG_COLOR_TYPE_TRUECOLOR)
        clr_type_str = "truecolor";
    else if(ihdr.color_type == SPNG_COLOR_TYPE_INDEXED)
        clr_type_str = "indexed color";
    else if(ihdr.color_type == SPNG_COLOR_TYPE_GRAYSCALE_ALPHA)
        clr_type_str = "grayscale with alpha";
    else
        clr_type_str = "truecolor with alpha";

    printf("width: %" PRIu32 "\nheight: %" PRIu32 "\n"
           "bit depth: %" PRIu8 "\ncolor type: %" PRIu8 " - %s\n",
           ihdr.width, ihdr.height,
           ihdr.bit_depth, ihdr.color_type, clr_type_str);
    printf("compression method: %" PRIu8 "\nfilter method: %" PRIu8 "\n"
           "interlace method: %" PRIu8 "\n",
           ihdr.compression_method, ihdr.filter_method,
           ihdr.interlace_method);
#endif

    size_t out_size;

    r = spng_decoded_image_size(ctx, SPNG_FMT_RGBA8, &out_size);

    if(r)
        return s;

    unsigned char *out = (u8*) malloc(out_size);
    if(out==NULL)
        return s;

    r = spng_decode_image(ctx, out, out_size, SPNG_FMT_RGBA8, 0);

    if(r)
    {
        printf("spng_decode_image() error: %s\n", spng_strerror(r));
        return s;
    }

    spng_ctx_free(ctx);

    /* write raw pixels to file */
    //fwrite(out, out_size, 1, raw);

    free(out);
    free(pngbuf);
    return s;
}

// ----------------------------------------------------------------------
// stb
// ----------------------------------------------------------------------

#define STB_IMAGE_IMPLEMENTATION
#include "../jpeg_benchmark/stb_image.h"

Surface stb_load_png(const char* filename)
{
    int width, height, bpp;
    u8* rgb = stbi_load(filename, &width, &height, &bpp, 3);
 
    return Surface(width, height, FORMAT_R8G8B8, width * 3, rgb);
}

// ----------------------------------------------------------------------
// main()
// ----------------------------------------------------------------------

int main(int argc, const char* argv[])
{
    if (argc < 2)
    {
        printf("Too few arguments. usage: <filename.png>\n");
        exit(1);
    }

    warmup(argv[1]);

    Timer timer;
    uint64 time0;
    uint64 time1;

    // ------------------------------------------------------------------

    printf("load spng:    ");
    time0 = timer.us();

    Surface s = load_spng(argv[1]);

    time1 = timer.us();
    printf("%d us\n", int(time1 - time0));

    // ------------------------------------------------------------------

    printf("load stb:     ");
    time0 = timer.us();

    Surface s_stb = stb_load_png(argv[1]);

    time1 = timer.us();
    printf("%d us\n", int(time1 - time0));

    // ------------------------------------------------------------------

    printf("load mango:   ");
    time0 = timer.us();

    Bitmap bitmap(argv[1]);

    time1 = timer.us();
    printf("%d us\n", int(time1 - time0));

    // ------------------------------------------------------------------

    printf("\n");
    printf("image: %d x %d\n", bitmap.width, bitmap.height);
}
