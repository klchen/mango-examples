/*
    MANGO Multimedia Development Platform
    Copyright (C) 2012-2019 Twilight Finland 3D Oy Ltd. All rights reserved.
*/
#include <mango/mango.hpp>

using namespace mango;
using namespace mango::filesystem;

#define ENABLE_LIBPNG
#define ENABLE_LODEPNG
//#define ENABLE_SPNG
#define ENABLE_STB
#define ENABLE_MANGO

// ----------------------------------------------------------------------
// libpng
// ----------------------------------------------------------------------

#if defined ENABLE_LIBPNG

#include <png.h>

struct png_source
{
    u8* data;
    int size;
    int offset;
};

static void png_read_callback(png_structp png_ptr, png_bytep data, png_size_t length)
{
    png_source* source = (png_source*) png_get_io_ptr(png_ptr);
    std::memcpy(data, source->data + source->offset, length);
    source->offset += length;
}

void load_libpng(Memory memory)
{
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    png_infop info_ptr = png_create_info_struct(png_ptr);

    png_source source;
    source.data = memory.address + 8;
    source.size = memory.size;
    source.offset = 0;
    png_set_read_fn(png_ptr, &source, png_read_callback);

    png_set_sig_bytes(png_ptr, 8);
    png_read_info(png_ptr, info_ptr);

    int width = png_get_image_width(png_ptr, info_ptr);
    int height = png_get_image_height(png_ptr, info_ptr);
    png_byte color_type = png_get_color_type(png_ptr, info_ptr);
    png_byte bit_depth = png_get_bit_depth(png_ptr, info_ptr);

    int number_of_passes = png_set_interlace_handling(png_ptr);
    png_read_update_info(png_ptr, info_ptr);

    int stride = png_get_rowbytes(png_ptr, info_ptr);
    u8* image = (u8*)malloc(stride * height);
    png_bytep *row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);


    for (int y=0; y<height; y++)
    {
        row_pointers[y] = image + stride * y;
    }

    png_read_image(png_ptr, row_pointers);

    free(row_pointers);
    free(image);
}

#endif

// ----------------------------------------------------------------------
// lodepng
// ----------------------------------------------------------------------

#if defined ENABLE_LODEPNG

#include "lodepng/lodepng.h"

void load_lodepng(Memory memory)
{
    u32 width, height;
    u8* image;
    int error = lodepng_decode32(&image, &width, &height, memory.address, memory.size);
    free(image);
}

#endif

// ----------------------------------------------------------------------
// spng
// ----------------------------------------------------------------------

// https://libspng.org/

#if defined(ENABLE_SPNG)

#include <spng.h>

struct read_fn_state
{
    unsigned char *data;
    size_t bytes_left;
};

int read_fn(struct spng_ctx *ctx, void *user, void *data, size_t n)
{
    struct read_fn_state *state = (struct read_fn_state *) user;
    if(n > state->bytes_left) return SPNG_IO_EOF;

    unsigned char *dst = (u8*)data;
    unsigned char *src = state->data;

#if defined(TEST_SPNG_STREAM_READ_INFO)
    printf("libspng bytes read: %lu\n", n);
#endif

    memcpy(dst, src, n);

    state->bytes_left -= n;
    state->data += n;

    return 0;
}

unsigned char *getimage_libspng(unsigned char *buf, size_t size, size_t *out_size, int fmt, int flags, struct spng_ihdr *info)
{
    int r;
    size_t siz;
    unsigned char *out = NULL;
    struct spng_ihdr ihdr;

    spng_ctx *ctx = spng_ctx_new(0);

    if(ctx==NULL)
    {
        printf("spng_ctx_new() failed\n");
        return NULL;
    }

    /*struct read_fn_state state;
    state.data = buf;
    state.bytes_left = size;
    r = spng_set_png_stream(ctx, read_fn, &state);
    if(r)
    {
        printf("spng_set_png_stream() error: %s\n", spng_strerror(r));
        goto err;
    }*/


    spng_set_crc_action(ctx, SPNG_CRC_USE, SPNG_CRC_USE);

    r = spng_set_png_buffer(ctx, buf, size);

    if(r)
    {
        printf("spng_set_png_buffer() error: %s\n", spng_strerror(r));
        goto err;
    }

    r = spng_get_ihdr(ctx, &ihdr);

    if(r)
    {
        printf("spng_get_ihdr() error: %s\n", spng_strerror(r));
        goto err;
    }

    memcpy(info, &ihdr, sizeof(struct spng_ihdr));

    r = spng_decoded_image_size(ctx, fmt, &siz);
    if(r) goto err;

    *out_size = siz;

    out = (unsigned char*)malloc(siz);
    if(out==NULL) goto err;

    r = spng_decode_image(ctx, out, siz,  fmt, flags);

    if(r)
    {
        printf("spng_decode_image() error: %s\n", spng_strerror(r));
        goto err;
    }

    spng_ctx_free(ctx);

goto skip_err;

err:
    spng_ctx_free(ctx);
    if(out !=NULL) free(out);
    return NULL;

skip_err:

    return out;
}

void load_spng(Memory memory)
{
    struct spng_ihdr ihdr;
    size_t img_spng_size;

    u8* image = getimage_libspng(memory.address, memory.size, &img_spng_size, SPNG_FMT_RGBA8, 0, &ihdr);
    free(image);
}

#endif

// ----------------------------------------------------------------------
// stb
// ----------------------------------------------------------------------

#if defined ENABLE_STB

#define STB_IMAGE_IMPLEMENTATION
#include "../jpeg_benchmark/stb_image.h"

void load_stb(Memory memory)
{
    int width, height, bpp;
    u8* image = stbi_load_from_memory(memory.address, memory.size, &width, &height, &bpp, 4);
    free(image);
}

#endif

// ----------------------------------------------------------------------
// mango
// ----------------------------------------------------------------------

#if defined(ENABLE_MANGO)

void load_mango(Memory memory)
{
    ImageDecoder decoder(memory, ".png");

    ImageHeader header = decoder.header();
    Bitmap bitmap(header.width, header.height, header.format);

    ImageDecodeOptions options;
    decoder.decode(bitmap, options);
}

#endif

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

    const char* filename = argv[1];

    File file(filename);
    Buffer buffer(file);

    u64 time0;
    u64 time1;

    // ------------------------------------------------------------------

#if defined ENABLE_LIBPNG

    printf("load libpng:  ");
    time0 = Time::us();

    load_libpng(buffer);

    time1 = Time::us();
    printf("%5d.%d ms\n", int((time1 - time0)/1000), int((time1 - time0)%1000));

#endif

    // ------------------------------------------------------------------

#if defined ENABLE_LODEPNG

    printf("load lodepng: ");
    time0 = Time::us();

    load_lodepng(buffer);

    time1 = Time::us();
    printf("%5d.%d ms\n", int((time1 - time0)/1000), int((time1 - time0)%1000));

#endif

    // ------------------------------------------------------------------

#if defined(ENABLE_SPNG)

    printf("load spng:    ");
    time0 = Time::us();

    load_spng(buffer);

    time1 = Time::us();
    printf("%5d.%d ms\n", int((time1 - time0)/1000), int((time1 - time0)%1000));

#endif

    // ------------------------------------------------------------------

#if defined(ENABLE_STB)

    printf("load stb:     ");
    time0 = Time::us();

    load_stb(buffer);

    time1 = Time::us();
    printf("%5d.%d ms\n", int((time1 - time0)/1000), int((time1 - time0)%1000));

#endif

    // ------------------------------------------------------------------

#if defined(ENABLE_MANGO)

    printf("load mango:   ");
    time0 = Time::us();

    load_mango(buffer);

    time1 = Time::us();
    printf("%5d.%d ms\n", int((time1 - time0)/1000), int((time1 - time0)%1000));

#endif

    // ------------------------------------------------------------------

    //printf("\n");
    //printf("image: %d x %d\n", bitmap.width, bitmap.height);

    //bitmap.save("/test/debug.png");
    //s_stb.save("debug.jpg");
}
