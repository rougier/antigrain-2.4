#include <stdio.h>
#include <string.h>
#include "agg_pixfmt_rgb24.h"
#include "agg_renderer_base.h"
#include "agg_renderer_scanline.h"
#include "agg_scanline_u.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_path_storage.h"

enum
{
    frame_width = 200,
    frame_height = 200
};

// Writing the buffer to a .PPM file, assuming it has 
// RGB-structure, one byte per color component
//--------------------------------------------------
bool write_ppm(const unsigned char* buf, 
               unsigned width, 
               unsigned height, 
               const char* file_name)
{
    FILE* fd = fopen(file_name, "wb");
    if(fd)
    {
        fprintf(fd, "P6 %d %d 255 ", width, height);
        fwrite(buf, 1, width * height * 3, fd);
        fclose(fd);
        return true;
    }
    return false;
}


int main()
{
    // Allocate the frame buffer (in this case "manually")
    // and create the rendering buffer object
    unsigned char* buffer = new unsigned char[frame_width * frame_height * 3];
    agg::rendering_buffer rbuf(buffer, 
                               frame_width, 
                               frame_height, 
                               frame_width * 3);

    // Create Pixel Format and Basic renderers
    //--------------------
    agg::pixfmt_rgb24 pixf(rbuf);
    agg::renderer_base<agg::pixfmt_rgb24> ren_base(pixf);

    // At last we do some very simple things, like clear
    //--------------------
    ren_base.clear(agg::rgba8(255, 250, 230));

    // Create Scanline Container, Scanline Rasterizer, 
    // and Scanline Renderer for solid fill.
    //--------------------
    agg::scanline_u8 sl;
    agg::rasterizer_scanline_aa<> ras;
    agg::renderer_scanline_aa_solid<
        agg::renderer_base<agg::pixfmt_rgb24> > ren_sl(ren_base);

    // Create Vertex Source (path) object, in our case it's 
    // path_storage and form the path.
    //--------------------
    agg::path_storage path;
    path.remove_all(); // Not obligatory in this case
    path.move_to(10, 10);
    path.line_to(frame_width-10, 10);
    path.line_to(frame_width-10, frame_height-10);
    path.line_to(10, frame_height-10);
    path.line_to(10, frame_height-20);
    path.curve4(frame_width-20, frame_height-20,
                frame_width-20, 20, 
                10, 20);

    // The vectorial pipeline
    //-----------------------
    ras.add_path(path);
    //-----------------------

    // Set the color and render the scanlines
    //-----------------------
    ren_sl.color(agg::rgba8(120, 60, 0));
    agg::render_scanlines(ras, sl, ren_sl);

    // Write the buffer to result.ppm and liberate memory.
    //-----------------------
    write_ppm(buffer, frame_width, frame_height, "result.ppm");
    delete [] buffer;
    return 0;
}

