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
//
// The Stack Blur Algorithm was invented by Mario Klingemann, 
// mario@quasimondo.com and described here:
// http://incubator.quasimondo.com/processing/fast_blur_deluxe.php
// (search phrase "Stackblur: Fast But Goodlooking"). 
// The major improvement is that there's no more division table
// that was very expensive to create for large blur radii. Insted, 
// for 8-bit per channel and radius not exceeding 254 the division is 
// replaced by multiplication and shift. 
//
//----------------------------------------------------------------------------

#ifndef AGG_BLUR_INCLUDED
#define AGG_BLUR_INCLUDED

#include "agg_array.h"
#include "agg_pixfmt_transposer.h"

namespace agg
{

    //==============================================================stack_blur
    template<class ColorT, class AccumulatorT> class stack_blur
    {
    public:
        typedef ColorT       color_type;
        typedef AccumulatorT accumulator_type;

        //--------------------------------------------------------------------
        template<class Img> void blur_x(Img& img, unsigned radius)
        {
            unsigned x, y, xp, i;
            unsigned stack_ptr;
            unsigned stack_start;

            color_type       pix;
            color_type*      stack_pix;
            accumulator_type sum;
            accumulator_type sum_in;
            accumulator_type sum_out;

            unsigned w   = img.width();
            unsigned h   = img.height();
            unsigned wm  = w - 1;
            unsigned div = radius * 2 + 1;

            unsigned div_sum = (radius + 1) * (radius + 1);
            unsigned mul_sum = 0;
            unsigned shr_sum = 0;
            unsigned max_val = color_type::base_mask;

            if(max_val <= 255 && radius < 255)
            {
                mul_sum = g_stack_blur8_mul[radius];
                shr_sum = g_stack_blur8_shr[radius];
            }

            m_buf.allocate(w, 128);
            m_stack.allocate(div, 32);

            for(y = 0; y < h; y++)
            {
                sum.clear();
                sum_in.clear();
                sum_out.clear();

                pix = img.pixel(0, y);
                for(i = 0; i <= radius; i++)
                {
                    m_stack[i] = pix;
                    sum.add(pix, i + 1);
                    sum_out.add(m_stack[i]);
                }
                for(i = 1; i <= radius; i++)
                {
                    pix = img.pixel((i > wm) ? wm : i, y);
                    m_stack[i + radius] = pix;
                    sum.add(pix, radius + 1 - i);
                    sum_in.add(pix);
                }

                stack_ptr = radius;
                for(x = 0; x < w; x++)
                {
                    if(mul_sum) sum.calc_pix(m_buf[x], mul_sum, shr_sum);
                    else        sum.calc_pix(m_buf[x], div_sum);

                    sum.sub(sum_out);
           
                    stack_start = stack_ptr + div - radius;
                    if(stack_start >= div) stack_start -= div;
                    stack_pix = &m_stack[stack_start];

                    sum_out.sub(*stack_pix);

                    xp = x + radius + 1;
                    if(xp > wm) xp = wm;
                    pix = img.pixel(xp, y);
            
                    *stack_pix = pix;
            
                    sum_in.add(pix);
                    sum.add(sum_in);
            
                    ++stack_ptr;
                    if(stack_ptr >= div) stack_ptr = 0;
                    stack_pix = &m_stack[stack_ptr];

                    sum_out.add(*stack_pix);
                    sum_in.sub(*stack_pix);
                }
                img.copy_color_hspan(0, y, w, &m_buf[0]);
            }
        }

        //--------------------------------------------------------------------
        template<class Img> void blur_y(Img& img, unsigned radius)
        {
            pixfmt_transposer<Img> img2(img);
            blur_x(img2, radius);
        }

        //--------------------------------------------------------------------
        template<class Img> void blur(Img& img, unsigned radius)
        {
            blur_x(img, radius);
            pixfmt_transposer<Img> img2(img);
            blur_x(img2, radius);
        }

    private:
        static int16u g_stack_blur8_mul[255];
        static int8u  g_stack_blur8_shr[255];

        pod_vector<color_type> m_buf;
        pod_vector<color_type> m_stack;
    };

    //------------------------------------------------------------------------
    template<class ColorT, class AccumulatorT> 
    int16u stack_blur<ColorT, AccumulatorT>::g_stack_blur8_mul[255] = 
    {
        512,512,456,512,328,456,335,512,405,328,271,456,388,335,292,512,
        454,405,364,328,298,271,496,456,420,388,360,335,312,292,273,512,
        482,454,428,405,383,364,345,328,312,298,284,271,259,496,475,456,
        437,420,404,388,374,360,347,335,323,312,302,292,282,273,265,512,
        497,482,468,454,441,428,417,405,394,383,373,364,354,345,337,328,
        320,312,305,298,291,284,278,271,265,259,507,496,485,475,465,456,
        446,437,428,420,412,404,396,388,381,374,367,360,354,347,341,335,
        329,323,318,312,307,302,297,292,287,282,278,273,269,265,261,512,
        505,497,489,482,475,468,461,454,447,441,435,428,422,417,411,405,
        399,394,389,383,378,373,368,364,359,354,350,345,341,337,332,328,
        324,320,316,312,309,305,301,298,294,291,287,284,281,278,274,271,
        268,265,262,259,257,507,501,496,491,485,480,475,470,465,460,456,
        451,446,442,437,433,428,424,420,416,412,408,404,400,396,392,388,
        385,381,377,374,370,367,363,360,357,354,350,347,344,341,338,335,
        332,329,326,323,320,318,315,312,310,307,304,302,299,297,294,292,
        289,287,285,282,280,278,275,273,271,269,267,265,263,261,259
    };


    //------------------------------------------------------------------------
    template<class ColorT, class AccumulatorT> 
    int8u  stack_blur<ColorT, AccumulatorT>::g_stack_blur8_shr[255] = 
    {
          9, 11, 12, 13, 13, 14, 14, 15, 15, 15, 15, 16, 16, 16, 16, 17, 
         17, 17, 17, 17, 17, 17, 18, 18, 18, 18, 18, 18, 18, 18, 18, 19, 
         19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 20, 20, 20,
         20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 21,
         21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21,
         21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 22, 22, 22, 22, 22, 22, 
         22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
         22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 23, 
         23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
         23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
         23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 
         23, 23, 23, 23, 23, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 
         24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
         24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
         24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
         24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24
    };


    //=========================================================stack_blur_rgba
    template<class T=unsigned> struct stack_blur_rgba
    {
        typedef T value_type;
        value_type r,g,b,a;

        void clear() 
        { 
            r = g = b = a = 0; 
        }

        template<class ArgT> void add(ArgT& v)
        {
            r += v.r;
            g += v.g;
            b += v.b;
            a += v.a;
        }

        template<class ArgT> void add(ArgT& v, unsigned k)
        {
            r += v.r * k;
            g += v.g * k;
            b += v.b * k;
            a += v.a * k;
        }

        template<class ArgT> void sub(ArgT& v)
        {
            r -= v.r;
            g -= v.g;
            b -= v.b;
            a -= v.a;
        }

        template<class ArgT> void calc_pix(ArgT& v, unsigned div)
        {
            typedef typename ArgT::value_type value_type;
            v.r = value_type(r / div);
            v.g = value_type(g / div);
            v.b = value_type(b / div);
            v.a = value_type(a / div);
        }

        template<class ArgT> void calc_pix(ArgT& v, unsigned mul, unsigned shr)
        {
            typedef typename ArgT::value_type value_type;
            v.r = value_type((r * mul) >> shr);
            v.g = value_type((g * mul) >> shr);
            v.b = value_type((b * mul) >> shr);
            v.a = value_type((a * mul) >> shr);
        }
    };





}




#endif
