/*
    MANGO Multimedia Development Platform
    Copyright (C) 2012-2018 Twilight Finland 3D Oy Ltd. All rights reserved.
*/
#include <mango/mango.hpp>

using namespace mango;
using namespace mango::filesystem;

//#define TEST_STB
//#define TEST_OCV

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
// libjpeg
// ----------------------------------------------------------------------

#include <jpeglib.h>
#include <jerror.h>

Surface load_jpeg(const char* filename)
{
    FILE *file = fopen(filename, "rb" );
    if ( file == NULL )
    {
        return Surface(0, 0, FORMAT_NONE, 0, NULL);
    }

    struct jpeg_decompress_struct info; //for our jpeg info
    struct jpeg_error_mgr err; //the error handler

    info.err = jpeg_std_error( &err );
    jpeg_create_decompress( &info ); //fills info structure

    jpeg_stdio_src( &info, file );
    jpeg_read_header( &info, TRUE );

    jpeg_start_decompress( &info );

    int w = info.output_width;
    int h = info.output_height;
    int numChannels = info.num_components; // 3 = RGB, 4 = RGBA
    unsigned long dataSize = w * h * numChannels;

    // read scanlines one at a time & put bytes in jdata[] array (assumes an RGB image)
    unsigned char *data = new u8[dataSize];;
    unsigned char *rowptr[ 1 ]; // array or pointers
    for ( ; info.output_scanline < info.output_height ; )
    {
        rowptr[ 0 ] = data + info.output_scanline * w * numChannels;
        jpeg_read_scanlines( &info, rowptr, 1 );
    }

    jpeg_finish_decompress( &info );

    fclose( file );

    Format format = FORMAT_R8G8B8;
    if (numChannels == 4)
        format = FORMAT_R8G8B8A8;

    // TODO: free data, format depends on numChannels
    return Surface(w, h, format, w * numChannels, data);
}

void save_jpeg(const char* filename, const Surface& surface)
{
    struct jpeg_compress_struct cinfo;
    jpeg_create_compress(&cinfo);

    struct jpeg_error_mgr jerr;
    cinfo.err = jpeg_std_error(&jerr);

    FILE * outfile;
    if ((outfile = fopen(filename, "wb")) == NULL)
    {
        fprintf(stderr, "can't open %s\n", filename);
        exit(1);
    }
    jpeg_stdio_dest(&cinfo, outfile);

    cinfo.image_width = surface.width;
    cinfo.image_height = surface.height;
    cinfo.input_components = surface.format.bytes();
    cinfo.in_color_space = surface.format.bytes() == 3 ? JCS_RGB : JCS_EXT_RGBA;

    int quality = 95;

    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, quality, TRUE);
    jpeg_start_compress(&cinfo, TRUE);

    JSAMPROW row_pointer[1];

    while (cinfo.next_scanline < cinfo.image_height)
    {
        row_pointer[0] = surface.image + cinfo.next_scanline * surface.stride;
        int x = jpeg_write_scanlines(&cinfo, row_pointer, 1);
        (void) x;
    }

    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);
    fclose(outfile);

    delete[] surface.image;
}

// ----------------------------------------------------------------------
// stb
// ----------------------------------------------------------------------

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

Surface stb_load_jpeg(const char* filename)
{
    int width, height, bpp;
    u8* rgb = stbi_load(filename, &width, &height, &bpp, 3);

    return Surface(width, height, FORMAT_R8G8B8, width * 3, rgb);
}

void stb_save_jpeg(const char* filename, const Surface& surface)
{
    stbi_write_jpg(filename, surface.width, surface.height, 3, surface.image, surface.width*3);
    stbi_image_free(surface.image);
}

// ----------------------------------------------------------------------
// OpenCV
// ----------------------------------------------------------------------

#ifdef TEST_OCV

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;
Surface ocv_load_jpeg(const char* filename)
{
    cv::Mat image;
    image = cv::imread(filename, CV_LOAD_IMAGE_COLOR);

    int width = image.cols;
    int height = image.rows;
    return Surface(width, height, FORMAT_B8G8R8, width * 3, image.data);
}

#endif

// ----------------------------------------------------------------------
// main()
// ----------------------------------------------------------------------

int main(int argc, const char* argv[])
{
    if (argc < 2)
    {
        printf("Too few arguments. usage: <filename.jpg>\n");
        exit(1);
    }

    warmup(argv[1]);

    Timer timer;
    u64 time0;
    u64 time1;

    // ------------------------------------------------------------------

    printf("load libjpeg: ");
    time0 = timer.ms();

    Surface s = load_jpeg(argv[1]);

    time1 = timer.ms();
    printf("%d ms\n", int(time1 - time0));

    // ------------------------------------------------------------------

#ifdef TEST_STB
    printf("load stb:     ");
    time0 = timer.ms();

    Surface s_stb = stb_load_jpeg(argv[1]);

    time1 = timer.ms();
    printf("%d ms\n", int(time1 - time0));
#endif

    // ------------------------------------------------------------------

    printf("load mango:   ");
    time0 = timer.ms();

    Bitmap bitmap(argv[1]);

    time1 = timer.ms();
    printf("%d ms\n", int(time1 - time0));

    // ------------------------------------------------------------------

#ifdef TEST_OCV
    printf("load opencv:  ");
    time0 = timer.ms();

    Surface s_ocv = ocv_load_jpeg(argv[1]);

    time1 = timer.ms();
    printf("%d ms\n", int(time1 - time0));
#endif

    // ------------------------------------------------------------------

    printf("\n");
    printf("save libjpeg: ");
    time0 = timer.ms();

    save_jpeg("output-libjpeg.jpg", s);

    time1 = timer.ms();
    printf("%d ms\n", int(time1 - time0));

    // ------------------------------------------------------------------

#ifdef TEST_STB
    printf("save stb:     ");
    time0 = timer.ms();

    stb_save_jpeg("output-stb.jpg", s_stb);

    time1 = timer.ms();
    printf("%d ms\n", int(time1 - time0));
#endif

    // ------------------------------------------------------------------

    printf("save mango:   ");
    time0 = timer.ms();

    bitmap.save("output-mango.jpg");

    time1 = timer.ms();
    printf("%d ms\n", int(time1 - time0));

    // ------------------------------------------------------------------

#ifdef TEST_OCV
    printf("save opencv:  ");

    cv::Mat ocv_image = cv::imread(argv[1], CV_LOAD_IMAGE_COLOR); // loading is not timed
    time0 = timer.ms();

    cv::imwrite("output-ocv.jpg", ocv_image);

    time1 = timer.ms();
    printf("%d ms\n", int(time1 - time0));
#endif

    // ------------------------------------------------------------------

    printf("\n");
    printf("image: %d x %d\n", bitmap.width, bitmap.height);
}
