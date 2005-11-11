//----------------------------------------------------------------------------
// Anti-Grain Geometry - Version 2.4
// Copyright (C) 2002-2005 Maxim Shemanarev (http://www.antigrain.com)
//
// Permission to copy, use, modify, sell and distribute this software 
// is granted provided this copyright notice appears in all copies. 
// This software is provided "as is" without express or implied
// warranty, and with no claim as to its suitability for any purpose.
//
//----------------------------------------------------------------------------
// Contact: mcseem@antigrain.com
//          mcseemagg@yahoo.com
//          http://www.antigrain.com
//----------------------------------------------------------------------------
#ifndef AGG_RASTERIZER_SL_CLIP_INCLUDED
#define AGG_RASTERIZER_SL_CLIP_INCLUDED

#include "agg_clip_liang_barsky.h"
#include "agg_rasterizer_cells_aa.h"

namespace agg
{

    //------------------------------------------------------------------------
    struct ras_conv_int
    {
        typedef int coord_type;
        static AGG_INLINE int mul_div(double a, double b, double c)
        {
            return (int)(a * b / c);
        }
        static int xi(int v) { return v; }
        static int yi(int v) { return v; }
        static int upscale(double v) { return poly_coord(v); }
        static int downscale(int v)  { return v; }
    };

    //------------------------------------------------------------------------
    struct ras_conv_int_x3
    {
        typedef int coord_type;
        static AGG_INLINE int mul_div(double a, double b, double c)
        {
            return (int)(a * b / c);
        }
        static int xi(int v) { return v * 3; }
        static int yi(int v) { return v; }
        static int upscale(double v) { return poly_coord(v); }
        static int downscale(int v)  { return v; }
    };


    //------------------------------------------------------------------------
    struct ras_conv_dbl
    {
        typedef double coord_type;
        static AGG_INLINE double mul_div(double a, double b, double c)
        {
            return a * b / c;
        }
        static int xi(double v) { return poly_coord(v); }
        static int yi(double v) { return poly_coord(v); }
        static double upscale(double v) { return v; }
        static double downscale(int v)  { return poly_coord_inv(v); }
    };

    //------------------------------------------------------------------------
    struct ras_conv_dbl_x3
    {
        typedef double coord_type;
        static AGG_INLINE double mul_div(double a, double b, double c)
        {
            return a * b / c;
        }
        static int xi(double v) { return poly_coord(v * 3); }
        static int yi(double v) { return poly_coord(v); }
        static double upscale(double v) { return v; }
        static double downscale(int v)  { return poly_coord_inv(v); }
    };





    //------------------------------------------------------------------------
    template<class Rasterizer, class Conv> 
    class rasterizer_sl_clip
    {
    public:
        typedef Conv                      conv_type;
        typedef typename Conv::coord_type coord_type;
        typedef Rasterizer                rasterizer_type;
        typedef rect_base<coord_type>     rect_type;

        //--------------------------------------------------------------------
        rasterizer_sl_clip(rasterizer_type& ras) : 
            m_rasterizer(ras), 
            m_clipping(false) 
        {}

        //--------------------------------------------------------------------
        void reset_clipping();
        void clip_box(coord_type x1, coord_type y1, coord_type x2, coord_type y2);
        void move_to(coord_type x1, coord_type y1);
        void line_to(coord_type x2, coord_type y2);

    private:
        void line_to_clipped(coord_type x2, coord_type y2, unsigned f2);

        void line_clip_y(coord_type x1, coord_type y1, 
                         coord_type x2, coord_type y2, 
                         unsigned f1, unsigned f2);

        void line_clip_y2(coord_type x1, coord_type y1, 
                          coord_type x2, coord_type y2, 
                          coord_type x3, coord_type y3);

        void line_clip_y3(coord_type x1, coord_type y1, 
                          coord_type x2, coord_type y2, 
                          coord_type x3, coord_type y3, 
                          coord_type x4, coord_type y4);

        //--------------------------------------------------------------------
        // Disable copying
        rasterizer_sl_clip(const rasterizer_sl_clip<Rasterizer, Conv>&);
        const rasterizer_sl_clip<Rasterizer, Conv>& 
        operator = (const rasterizer_sl_clip<Rasterizer, Conv>&);

        rasterizer_type& m_rasterizer;
        rect_type        m_clip_box;
        coord_type       m_x1;
        coord_type       m_y1;
        unsigned         m_f1;
        bool             m_clipping;
    };







    //--------------------------------------------------------------------
    template<class Ras, class Conv>
    void rasterizer_sl_clip<Ras, Conv>::reset_clipping()
    {
        m_clipping = false;
    }

    //--------------------------------------------------------------------
    template<class Ras, class Conv>
    void rasterizer_sl_clip<Ras, Conv>::clip_box(coord_type x1, 
                                                 coord_type y1, 
                                                 coord_type x2, 
                                                 coord_type y2)
    {
        m_clip_box = rect_type(x1, y1, x2, y2);
        m_clip_box.normalize();
        m_clipping = true;
    }

    //------------------------------------------------------------------------
    template<class Ras, class Conv>
    AGG_INLINE
    void rasterizer_sl_clip<Ras, Conv>::line_clip_y(coord_type x1, coord_type y1, 
                                                    coord_type x2, coord_type y2, 
                                                    unsigned f1, 
                                                    unsigned f2)
    {
        f1 &= 10;
        f2 &= 10;
        if((f1 | f2) == 0)
        {
            // Fully visible
            m_rasterizer.line(Conv::xi(x1), Conv::yi(y1), 
                              Conv::xi(x2), Conv::yi(y2)); 
        }
        else
        {
            if(f1 == f2)
            {
                // Invisible by Y
                return;
            }

            coord_type tx1 = x1;
            coord_type ty1 = y1;
            coord_type tx2 = x2;
            coord_type ty2 = y2;

            if(f1 & 8) // y1 < clip.y1
            {
                tx1 = x1 + Conv::mul_div(m_clip_box.y1-y1, x2-x1, y2-y1);
                ty1 = m_clip_box.y1;
            }

            if(f1 & 2) // y1 > clip.y2
            {
                tx1 = x1 + Conv::mul_div(m_clip_box.y2-y1, x2-x1, y2-y1);
                ty1 = m_clip_box.y2;
            }

            if(f2 & 8) // y2 < clip.y1
            {
                tx2 = x1 + Conv::mul_div(m_clip_box.y1-y1, x2-x1, y2-y1);
                ty2 = m_clip_box.y1;
            }

            if(f2 & 2) // y2 > clip.y2
            {
                tx2 = x1 + Conv::mul_div(m_clip_box.y2-y1, x2-x1, y2-y1);
                ty2 = m_clip_box.y2;
            }
            m_rasterizer.line(Conv::xi(tx1), Conv::yi(ty1), 
                              Conv::xi(tx2), Conv::yi(ty2)); 
        }
    }

    //------------------------------------------------------------------------
    template<class Ras, class Conv>
    AGG_INLINE 
    void rasterizer_sl_clip<Ras, Conv>::line_clip_y2(coord_type x1, coord_type y1, 
                                                     coord_type x2, coord_type y2, 
                                                     coord_type x3, coord_type y3)
    {
        unsigned f2 = clipping_flags_y(y2, m_clip_box);
        line_clip_y(x1, y1, x2, y2, clipping_flags_y(y1, m_clip_box), f2);
        line_clip_y(x2, y2, x3, y3, f2, clipping_flags_y(y3, m_clip_box));
    }

    //------------------------------------------------------------------------
    template<class Ras, class Conv>
    AGG_INLINE 
    void rasterizer_sl_clip<Ras, Conv>::line_clip_y3(coord_type x1, coord_type y1, 
                                                     coord_type x2, coord_type y2, 
                                                     coord_type x3, coord_type y3, 
                                                     coord_type x4, coord_type y4)
    {
        unsigned f2 = clipping_flags_y(y2, m_clip_box);
        unsigned f3 = clipping_flags_y(y3, m_clip_box);
        line_clip_y(x1, y1, x2, y2, clipping_flags_y(y1, m_clip_box), f2);
        line_clip_y(x2, y2, x3, y3, f2, f3);
        line_clip_y(x3, y3, x4, y4, f3, clipping_flags_y(y4, m_clip_box));
    }

    //------------------------------------------------------------------------
    template<class Ras, class Conv>
    void rasterizer_sl_clip<Ras, Conv>::line_to_clipped(coord_type x2, 
                                                        coord_type y2,
                                                        unsigned f2)
    {
        if((m_f1 & 10) == (f2 & 10) && (m_f1 & 10) != 0)
        {
            // Invisible by Y
            return;
        }

        coord_type x1 = m_x1;
        coord_type y1 = m_y1;

        switch(((m_f1 & 5) << 1) | (f2 & 5))
        {
        case 0: // Visible by X
            line_clip_y(x1, y1, x2, y2, m_f1, f2);
            break;

        case 1: // x2 > clip.x2
            line_clip_y2(x1,
                         y1, 
                         m_clip_box.x2, 
                         y1 + Conv::mul_div(m_clip_box.x2-x1, y2-y1, x2-x1),
                         m_clip_box.x2, 
                         y2);
            break;

        case 2: // x1 > clip.x2
            line_clip_y2(m_clip_box.x2,
                         y1,
                         m_clip_box.x2,
                         y1 + Conv::mul_div(m_clip_box.x2-x1, y2-y1, x2-x1),
                         x2,
                         y2);
            break;

        case 3: // x1 > clip.x2 && x2 > clip.x2
            line_clip_y(m_clip_box.x2, y1, m_clip_box.x2, y2, m_f1, f2);
            break;

        case 4: // x2 < clip.x1
            line_clip_y2(x1,
                         y1,
                         m_clip_box.x1,
                         y1 + Conv::mul_div(m_clip_box.x1-x1, y2-y1, x2-x1),
                         m_clip_box.x1,
                         y2);
            break;

        case 6: // x1 > clip.x2 && x2 < clip.x1
            line_clip_y3(m_clip_box.x2,
                         y1,
                         m_clip_box.x2,
                         y1 + Conv::mul_div(m_clip_box.x2-x1, y2-y1, x2-x1),
                         m_clip_box.x1,
                         y1 + Conv::mul_div(m_clip_box.x1-x1, y2-y1, x2-x1),
                         m_clip_box.x1,
                         y2);
            break;

        case 8: // x1 < clip.x1
            line_clip_y2(m_clip_box.x1, 
                         y1,
                         m_clip_box.x1,
                         y1 + Conv::mul_div(m_clip_box.x1-x1, y2-y1, x2-x1),
                         x2,
                         y2);
            break;

        case 9:  // x1 < clip.x1 && x2 > clip.x2
            line_clip_y3(m_clip_box.x1,
                         y1,
                         m_clip_box.x1,
                         y1 + Conv::mul_div(m_clip_box.x1-x1, y2-y1, x2-x1),
                         m_clip_box.x2,
                         y1 + Conv::mul_div(m_clip_box.x2-x1, y2-y1, x2-x1),
                         m_clip_box.x2,
                         y2);
            break;

        case 12: // x1 < clip.x1 && x2 < clip.x1
            line_clip_y(m_clip_box.x1, y1, m_clip_box.x1, y2, m_f1, f2);
            break;
        }
    }

    //--------------------------------------------------------------------
    template<class Ras, class Conv>
    AGG_INLINE 
    void rasterizer_sl_clip<Ras, Conv>::move_to(coord_type x1, 
                                                coord_type y1)
    {
        m_x1 = x1;
        m_y1 = y1;
        if(m_clipping)
        {
            m_f1 = clipping_flags(x1, y1, m_clip_box);
        }
    }

    //--------------------------------------------------------------------
    template<class Ras, class Conv>
    AGG_INLINE 
    void rasterizer_sl_clip<Ras, Conv>::line_to(coord_type x2, 
                                                coord_type y2)
    {
        if(m_clipping)
        {
            unsigned f2 = clipping_flags(x2, y2, m_clip_box);
            line_to_clipped(x2, y2, f2);
            m_f1 = f2;
        }
        else
        {
            m_rasterizer.line(Conv::xi(m_x1), Conv::yi(m_y1), 
                              Conv::xi(x2),   Conv::yi(y2)); 
        }
        m_x1 = x2;
        m_y1 = y2;
    }






    //------------------------------------------------------------------------
    template<class Rasterizer>  class rasterizer_sl_no_clip
    {
    public:
        typedef ras_conv_int conv_type;
        typedef int          coord_type;
        typedef Rasterizer   rasterizer_type;

        //--------------------------------------------------------------------
        rasterizer_sl_no_clip(rasterizer_type& ras) : m_rasterizer(ras) {}

        void reset_clipping() {}
        void clip_box(coord_type x1, coord_type y1, coord_type x2, coord_type y2) {}
        void move_to(coord_type x1, coord_type y1) { m_x1 = x1; m_y1 = y1; }
        void line_to(coord_type x2, coord_type y2) 
        { 
            m_rasterizer.line(m_x1, m_y1, x2, y2); 
            m_x1 = x2; m_y1 = y2;
        }

    private:
        //--------------------------------------------------------------------
        // Disable copying
        rasterizer_sl_no_clip(const rasterizer_sl_no_clip<Rasterizer>&);
        const rasterizer_sl_no_clip<Rasterizer>& 
        operator = (const rasterizer_sl_no_clip<Rasterizer>&);

        rasterizer_type& m_rasterizer;
        int              m_x1, m_y1;
    };


}

#endif
