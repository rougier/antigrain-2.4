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

#ifndef AGG_SPAN_IMAGE_RESAMPLE_RGBA_INCLUDED
#define AGG_SPAN_IMAGE_RESAMPLE_RGBA_INCLUDED

#include "agg_color_rgba.h"
#include "agg_span_image_resample.h"

namespace agg
{

    //========================================span_image_resample_rgba_affine
    template<class ColorT, class Order> 
    class span_image_resample_rgba_affine : 
    public span_image_resample_affine<ColorT>
    {
    public:
        typedef ColorT color_type;
        typedef Order order_type;
        typedef span_image_resample_affine<color_type> base_type;
        typedef typename base_type::interpolator_type interpolator_type;
        typedef typename color_type::value_type value_type;
        typedef typename color_type::long_type long_type;
        enum base_scale_e
        {
            base_shift      = color_type::base_shift,
            base_mask       = color_type::base_mask,
            downscale_shift = image_filter_shift
        };

        //--------------------------------------------------------------------
        span_image_resample_rgba_affine() {}
        span_image_resample_rgba_affine(const rendering_buffer& src, 
                                        const color_type& back_color,
                                        interpolator_type& inter,
                                        const image_filter_lut& filter) :
            base_type(src, back_color, inter, filter) 
        {}


        //--------------------------------------------------------------------
        void generate(color_type* span, int x, int y, unsigned len)
        {
            base_type::interpolator().begin(x + base_type::filter_dx_dbl(), 
                                            y + base_type::filter_dy_dbl(), len);

            long_type fg[4];
            value_type back_r = base_type::background_color().r;
            value_type back_g = base_type::background_color().g;
            value_type back_b = base_type::background_color().b;
            value_type back_a = base_type::background_color().a;

            int diameter = base_type::filter().diameter();
            int filter_scale = diameter << image_subpixel_shift;
            int radius_x = (diameter * base_type::m_rx) >> 1;
            int radius_y = (diameter * base_type::m_ry) >> 1;
            int maxx = base_type::source_image().width() - 1;
            int maxy = base_type::source_image().height() - 1;

            const int16* weight_array = base_type::filter().weight_array();

            do
            {
                base_type::interpolator().coordinates(&x, &y);

                x += base_type::filter_dx_int() - radius_x;
                y += base_type::filter_dy_int() - radius_y;

                fg[0] = fg[1] = fg[2] = fg[3] = image_filter_scale / 2;

                int y_lr = y >> image_subpixel_shift;
                int y_hr = ((image_subpixel_mask - (y & image_subpixel_mask)) * 
                                base_type::m_ry_inv) >> 
                                    image_subpixel_shift;
                int total_weight = 0;
                int x_lr_ini = x >> image_subpixel_shift;
                int x_hr_ini = ((image_subpixel_mask - (x & image_subpixel_mask)) * 
                                   base_type::m_rx_inv) >> 
                                       image_subpixel_shift;
                do
                {
                    int weight_y = weight_array[y_hr];
                    int x_lr = x_lr_ini;
                    int x_hr = x_hr_ini;
                    if(y_lr >= 0 && y_lr <= maxy)
                    {
                        const value_type* fg_ptr = (const value_type*)
                            base_type::source_image().row(y_lr) + (x_lr << 2);
                        do
                        {
                            int weight = (weight_y * weight_array[x_hr] + 
                                         image_filter_scale / 2) >> 
                                         downscale_shift;

                            if(x_lr >= 0 && x_lr <= maxx)
                            {
                                fg[0] += fg_ptr[0] * weight;
                                fg[1] += fg_ptr[1] * weight;
                                fg[2] += fg_ptr[2] * weight;
                                fg[3] += fg_ptr[3] * weight;
                            }
                            else
                            {
                                fg[order_type::R] += back_r * weight;
                                fg[order_type::G] += back_g * weight;
                                fg[order_type::B] += back_b * weight;
                                fg[order_type::A] += back_a * weight;
                            }
                            total_weight += weight;
                            fg_ptr += 4;
                            x_hr   += base_type::m_rx_inv;
                            ++x_lr;
                        }
                        while(x_hr < filter_scale);
                    }
                    else
                    {
                        do
                        {
                            int weight = (weight_y * weight_array[x_hr] + 
                                         image_filter_scale / 2) >> 
                                         downscale_shift;

                            total_weight      += weight;
                            fg[order_type::R] += back_r * weight;
                            fg[order_type::G] += back_g * weight;
                            fg[order_type::B] += back_b * weight;
                            fg[order_type::A] += back_a * weight;
                            x_hr              += base_type::m_rx_inv;
                        }
                        while(x_hr < filter_scale);
                    }
                    y_hr += base_type::m_ry_inv;
                    ++y_lr;
                }
                while(y_hr < filter_scale);

                fg[0] /= total_weight;
                fg[1] /= total_weight;
                fg[2] /= total_weight;
                fg[3] /= total_weight;

                if(fg[0] < 0) fg[0] = 0;
                if(fg[1] < 0) fg[1] = 0;
                if(fg[2] < 0) fg[2] = 0;
                if(fg[3] < 0) fg[3] = 0;

                if(fg[order_type::A] > base_mask)         fg[order_type::A] = base_mask;
                if(fg[order_type::R] > fg[order_type::A]) fg[order_type::R] = fg[order_type::A];
                if(fg[order_type::G] > fg[order_type::A]) fg[order_type::G] = fg[order_type::A];
                if(fg[order_type::B] > fg[order_type::A]) fg[order_type::B] = fg[order_type::A];

                span->r = (value_type)fg[order_type::R];
                span->g = (value_type)fg[order_type::G];
                span->b = (value_type)fg[order_type::B];
                span->a = (value_type)fg[order_type::A];

                ++span;
                ++base_type::interpolator();
            } while(--len);
        }
    };







    //==============================================span_image_resample_rgba
    template<class ColorT,
             class Order, 
             class Interpolator>
    class span_image_resample_rgba : 
    public span_image_resample<ColorT, Interpolator>
    {
    public:
        typedef ColorT color_type;
        typedef Order order_type;
        typedef Interpolator interpolator_type;
        typedef span_image_resample<color_type, interpolator_type> base_type;
        typedef typename color_type::value_type value_type;
        typedef typename color_type::long_type long_type;
        enum base_scale_e
        {
            base_shift = color_type::base_shift,
            base_mask  = color_type::base_mask,
            downscale_shift = image_filter_shift
        };

        //--------------------------------------------------------------------
        span_image_resample_rgba() {}
        span_image_resample_rgba(const rendering_buffer& src, 
                                 const color_type& back_color,
                                 interpolator_type& inter,
                                 const image_filter_lut& filter) :
            base_type(src, back_color, inter, filter)
        {}

        //--------------------------------------------------------------------
        void generate(color_type* span, int x, int y, unsigned len)
        {
            base_type::interpolator().begin(x + base_type::filter_dx_dbl(), 
                                            y + base_type::filter_dy_dbl(), len);
            long_type fg[4];
            value_type back_r = base_type::background_color().r;
            value_type back_g = base_type::background_color().g;
            value_type back_b = base_type::background_color().b;
            value_type back_a = base_type::background_color().a;

            int diameter = base_type::filter().diameter();
            int filter_scale = diameter << image_subpixel_shift;

            const int16* weight_array = base_type::filter().weight_array();

            do
            {
                int rx;
                int ry;
                int rx_inv = image_subpixel_scale;
                int ry_inv = image_subpixel_scale;
                base_type::interpolator().coordinates(&x,  &y);
                base_type::interpolator().local_scale(&rx, &ry);

                rx = (rx * base_type::m_blur_x) >> image_subpixel_shift;
                ry = (ry * base_type::m_blur_y) >> image_subpixel_shift;

                if(rx < image_subpixel_scale)
                {
                    rx = image_subpixel_scale;
                }
                else
                {
                    if(rx > image_subpixel_scale * base_type::m_scale_limit) 
                    {
                        rx = image_subpixel_scale * base_type::m_scale_limit;
                    }
                    rx_inv = image_subpixel_scale * image_subpixel_scale / rx;
                }

                if(ry < image_subpixel_scale)
                {
                    ry = image_subpixel_scale;
                }
                else
                {
                    if(ry > image_subpixel_scale * base_type::m_scale_limit) 
                    {
                        ry = image_subpixel_scale * base_type::m_scale_limit;
                    }
                    ry_inv = image_subpixel_scale * image_subpixel_scale / ry;
                }

                int radius_x = (diameter * rx) >> 1;
                int radius_y = (diameter * ry) >> 1;
                int maxx = base_type::source_image().width() - 1;
                int maxy = base_type::source_image().height() - 1;

                x += base_type::filter_dx_int() - radius_x;
                y += base_type::filter_dy_int() - radius_y;

                fg[0] = fg[1] = fg[2] = fg[3] = image_filter_scale / 2;

                int y_lr = y >> image_subpixel_shift;
                int y_hr = ((image_subpixel_mask - (y & image_subpixel_mask)) * 
                               ry_inv) >> 
                                   image_subpixel_shift;
                int total_weight = 0;
                int x_lr_ini = x >> image_subpixel_shift;
                int x_hr_ini = ((image_subpixel_mask - (x & image_subpixel_mask)) * 
                                   rx_inv) >> 
                                       image_subpixel_shift;

                do
                {
                    int weight_y = weight_array[y_hr];
                    int x_lr = x_lr_ini;
                    int x_hr = x_hr_ini;
                    if(y_lr >= 0 && y_lr <= maxy)
                    {
                        const value_type* fg_ptr = (const value_type*)
                            base_type::source_image().row(y_lr) + (x_lr << 2);
                        do
                        {
                            int weight = (weight_y * weight_array[x_hr] + 
                                         image_filter_scale / 2) >> 
                                         downscale_shift;

                            if(x_lr >= 0 && x_lr <= maxx)
                            {
                                fg[0] += fg_ptr[0] * weight;
                                fg[1] += fg_ptr[1] * weight;
                                fg[2] += fg_ptr[2] * weight;
                                fg[3] += fg_ptr[3] * weight;
                            }
                            else
                            {
                                fg[order_type::R] += back_r * weight;
                                fg[order_type::G] += back_g * weight;
                                fg[order_type::B] += back_b * weight;
                                fg[order_type::A] += back_a * weight;
                            }
                            total_weight += weight;
                            fg_ptr += 4;
                            x_hr   += rx_inv;
                            ++x_lr;
                        }
                        while(x_hr < filter_scale);
                    }
                    else
                    {
                        do
                        {
                            int weight = (weight_y * weight_array[x_hr] + 
                                         image_filter_scale / 2) >> 
                                         downscale_shift;

                            total_weight      += weight;
                            fg[order_type::R] += back_r * weight;
                            fg[order_type::G] += back_g * weight;
                            fg[order_type::B] += back_b * weight;
                            fg[order_type::A] += back_a * weight;
                            x_hr              += rx_inv;
                        }
                        while(x_hr < filter_scale);
                    }
                    y_hr += ry_inv;
                    ++y_lr;
                }
                while(y_hr < filter_scale);

                fg[0] /= total_weight;
                fg[1] /= total_weight;
                fg[2] /= total_weight;
                fg[3] /= total_weight;

                if(fg[0] < 0) fg[0] = 0;
                if(fg[1] < 0) fg[1] = 0;
                if(fg[2] < 0) fg[2] = 0;
                if(fg[3] < 0) fg[3] = 0;

                if(fg[order_type::A] > base_mask)         fg[order_type::A] = base_mask;
                if(fg[order_type::R] > fg[order_type::R]) fg[order_type::R] = fg[order_type::R];
                if(fg[order_type::G] > fg[order_type::G]) fg[order_type::G] = fg[order_type::G];
                if(fg[order_type::B] > fg[order_type::B]) fg[order_type::B] = fg[order_type::B];

                span->r = (value_type)fg[order_type::R];
                span->g = (value_type)fg[order_type::G];
                span->b = (value_type)fg[order_type::B];
                span->a = (value_type)fg[order_type::A];

                ++span;
                ++base_type::interpolator();
            } while(--len);
        }
    };

}


#endif
