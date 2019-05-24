/*
    MANGO Multimedia Development Platform
    Copyright (C) 2012-2018 Twilight Finland 3D Oy Ltd. All rights reserved.
*/
#include <mango/mango.hpp>

using namespace mango;
using namespace mango::filesystem;

//#define ENABLE_SPNG

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
// libpng
// ----------------------------------------------------------------------

#include <png.h>

void libpng_load(const char* file_name)
{
    int x, y;

    int width, height;
    png_byte color_type;
    png_byte bit_depth;

    png_structp png_ptr;
    png_infop info_ptr;
    int number_of_passes;
    png_bytep * row_pointers;

        char header[8];    // 8 is the maximum size that can be checked

        /* open file and test for it being a png */
        FILE *fp = fopen(file_name, "rb");
        //if (!fp)
        //        abort_("[read_png_file] File %s could not be opened for reading", file_name);
        size_t xx = fread(header, 1, 8, fp);
        (void) xx;
        //if (png_sig_cmp(header, 0, 8))
        //        abort_("[read_png_file] File %s is not recognized as a PNG file", file_name);


        /* initialize stuff */
        png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

        //if (!png_ptr)
        //        abort_("[read_png_file] png_create_read_struct failed");

        info_ptr = png_create_info_struct(png_ptr);
        //if (!info_ptr)
        //        abort_("[read_png_file] png_create_info_struct failed");

        //if (setjmp(png_jmpbuf(png_ptr)))
        //        abort_("[read_png_file] Error during init_io");

        png_init_io(png_ptr, fp);
        png_set_sig_bytes(png_ptr, 8);

        png_read_info(png_ptr, info_ptr);

        width = png_get_image_width(png_ptr, info_ptr);
        height = png_get_image_height(png_ptr, info_ptr);
        color_type = png_get_color_type(png_ptr, info_ptr);
        bit_depth = png_get_bit_depth(png_ptr, info_ptr);

        number_of_passes = png_set_interlace_handling(png_ptr);
        png_read_update_info(png_ptr, info_ptr);


        /* read file */
        //if (setjmp(png_jmpbuf(png_ptr)))
        //        abort_("[read_png_file] Error during read_image");

        row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);
        for (y=0; y<height; y++)
                row_pointers[y] = (png_byte*) malloc(png_get_rowbytes(png_ptr,info_ptr));

        png_read_image(png_ptr, row_pointers);

        fclose(fp);
}

// ----------------------------------------------------------------------
// lodepng
// ----------------------------------------------------------------------

#include "lodepng/lodepng.h"

void lode_decodeOneStep(const char* filename)
{
  std::vector<unsigned char> image;
  unsigned width, height;
  unsigned error = lodepng::decode(image, width, height, filename);
  //if(error) std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;
}

// ----------------------------------------------------------------------
// spng
// ----------------------------------------------------------------------

// https://libspng.org/

#if defined(ENABLE_SPNG)

#include <spng.h>

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

#endif // ENABLE_SPNG

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

    const char* filename = argv[1];
    warmup(filename);

    u64 time0;
    u64 time1;

    // ------------------------------------------------------------------

    printf("load libpng:  ");
    time0 = Time::us();

    libpng_load(filename);

    time1 = Time::us();
    printf("%5d ms\n", int((time1 - time0)/1000));

    // ------------------------------------------------------------------

    printf("load lodepng: ");
    time0 = Time::us();

    lode_decodeOneStep(filename);

    time1 = Time::us();
    printf("%5d ms\n", int((time1 - time0)/1000));

    // ------------------------------------------------------------------
#if defined(ENABLE_SPNG)
    printf("load spng:    ");
    time0 = Time::us();

    Surface s = load_spng(filename);

    time1 = Time::us();
    printf("%5d ms\n", int((time1 - time0)/1000));
#endif
    // ------------------------------------------------------------------

    printf("load stb:     ");
    time0 = Time::us();

    Surface s_stb = stb_load_png(filename);

    time1 = Time::us();
    printf("%5d ms\n", int((time1 - time0)/1000));

    // ------------------------------------------------------------------

    printf("load mango:   ");
    time0 = Time::us();

    Bitmap bitmap(filename);

    time1 = Time::us();
    printf("%5d ms\n", int((time1 - time0)/1000));

    // ------------------------------------------------------------------

    printf("\n");
    printf("image: %d x %d\n", bitmap.width, bitmap.height);

    //bitmap.save("debug.png");
    //s_stb.save("debug.jpg");
}
