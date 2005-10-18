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

#ifndef AGG_RENDERER_SCANLINE_INCLUDED
#define AGG_RENDERER_SCANLINE_INCLUDED

#include "agg_basics.h"
#include "agg_renderer_base.h"

namespace agg
{

    //================================================render_scanline_aa_solid
    template<class Scanline, class BaseRenderer, class ColorT> 
    void render_scanline_aa_solid(const Scanline& sl, 
                                  BaseRenderer& ren, 
                                  const ColorT& color)
    {
        int y = sl.y();
        unsigned num_spans = sl.num_spans();
        typename Scanline::const_iterator span = sl.begin();

        for(;;)
        {
            int x = span->x;
            if(span->len > 0)
            {
                ren.blend_solid_hspan(x, y, (unsigned)span->len, 
                                      color, 
                                      span->covers);
            }
            else
            {
                ren.blend_hline(x, y, (unsigned)(x - span->len - 1), 
                                color, 
                                *(span->covers));
            }
            if(--num_spans == 0) break;
            ++span;
        }
    }


    //===============================================render_scanlines_aa_solid
    template<class Rasterizer, class Scanline, 
             class BaseRenderer, class ColorT>
    void render_scanlines_aa_solid(Rasterizer& ras, Scanline& sl, 
                                   BaseRenderer& ren, const ColorT& color)
    {
        if(ras.rewind_scanlines())
        {
            // Explicitly convert "color" to the BaseRenderer color type.
            // For example, it can be called with color type "rgba", while
            // "rgba8" is needed. Otherwise it will be implicitly 
            // converted in the loop many times.
            //----------------------
            typename BaseRenderer::color_type ren_color(color);

            sl.reset(ras.min_x(), ras.max_x());
            while(ras.sweep_scanline(sl))
            {
                //render_scanline_aa_solid(sl, ren, ren_color);

                // This code is equivalent to the above call (copy/paste). 
                // It's just a "manual" optimization for old compilers,
                // like Microsoft Visual C++ v6.0
                //-------------------------------
                int y = sl.y();
                unsigned num_spans = sl.num_spans();
                typename Scanline::const_iterator span = sl.begin();

                for(;;)
                {
                    int x = span->x;
                    if(span->len > 0)
                    {
                        ren.blend_solid_hspan(x, y, (unsigned)span->len, 
                                              ren_color, 
                                              span->covers);
                    }
                    else
                    {
                        ren.blend_hline(x, y, (unsigned)(x - span->len - 1), 
                                        ren_color, 
                                        *(span->covers));
                    }
                    if(--num_spans == 0) break;
                    ++span;
                }
            }
        }
    }


    //==============================================renderer_scanline_aa_solid
    template<class BaseRenderer> class renderer_scanline_aa_solid
    {
    public:
        typedef BaseRenderer base_ren_type;
        typedef typename base_ren_type::color_type color_type;

        //--------------------------------------------------------------------
        renderer_scanline_aa_solid() : m_ren(0) {}
        renderer_scanline_aa_solid(base_ren_type& ren) : m_ren(&ren) {}
        void attach(base_ren_type& ren)
        {
            m_ren = &ren;
        }
        
        //--------------------------------------------------------------------
        void color(const color_type& c) { m_color = c; }
        const color_type& color() const { return m_color; }

        //--------------------------------------------------------------------
        void prepare() {}

        //--------------------------------------------------------------------
        template<class Scanline> void render(const Scanline& sl)
        {
            render_scanline_aa_solid(sl, *m_ren, m_color);
        }
        
    private:
        base_ren_type* m_ren;
        color_type m_color;
    };













    //======================================================render_scanline_aa
    template<class Scanline, class BaseRenderer, 
             class SpanAllocator, class SpanGenerator> 
    void render_scanline_aa(const Scanline& sl, BaseRenderer& ren, 
                            SpanAllocator& alloc, SpanGenerator& span_gen)
    {
        int y = sl.y();

        unsigned num_spans = sl.num_spans();
        typename Scanline::const_iterator span = sl.begin();
        for(;;)
        {
            int x = span->x;
            int len = span->len;
            const typename Scanline::cover_type* covers = span->covers;

            if(len < 0) len = -len;
            typename BaseRenderer::color_type* colors = alloc.allocate(len);
            span_gen.generate(colors, x, y, len);
            ren.blend_color_hspan(x, y, len, colors, 
                                  (span->len < 0) ? 0 : covers, *covers);

            if(--num_spans == 0) break;
            ++span;
        }
    }



    //=====================================================render_scanlines_aa
    template<class Rasterizer, class Scanline, class BaseRenderer, 
             class SpanAllocator, class SpanGenerator>
    void render_scanlines_aa(Rasterizer& ras, Scanline& sl, BaseRenderer& ren, 
                             SpanAllocator& alloc, SpanGenerator& span_gen)
    {
        if(ras.rewind_scanlines())
        {
            sl.reset(ras.min_x(), ras.max_x());
            span_gen.prepare();
            while(ras.sweep_scanline(sl))
            {
                render_scanline_aa(sl, ren, alloc, span_gen);
            }
        }
    }



    //====================================================renderer_scanline_aa
    template<class BaseRenderer, class SpanAllocator, class SpanGenerator> 
    class renderer_scanline_aa
    {
    public:
        typedef BaseRenderer  base_ren_type;
        typedef SpanAllocator alloc_type;
        typedef SpanGenerator span_gen_type;

        //--------------------------------------------------------------------
        renderer_scanline_aa() : m_ren(0), m_alloc(0), m_span_gen(0) {}
        renderer_scanline_aa(base_ren_type& ren, 
                             alloc_type& alloc, 
                             span_gen_type& span_gen) :
            m_ren(&ren),
            m_alloc(&alloc),
            m_span_gen(&span_gen)
        {}
        void attach(base_ren_type& ren, 
                    alloc_type& alloc, 
                    span_gen_type& span_gen)
        {
            m_ren = &ren;
            m_alloc = &alloc;
            m_span_gen = &span_gen;
        }
        
        //--------------------------------------------------------------------
        void prepare() { m_span_gen->prepare(); }

        //--------------------------------------------------------------------
        template<class Scanline> void render(const Scanline& sl)
        {
            render_scanline_aa(sl, *m_ren, *m_alloc, *m_span_gen);
        }

    private:
        base_ren_type* m_ren;
        alloc_type*    m_alloc;
        span_gen_type* m_span_gen;
    };















/*
    //===================================================renderer_scanline_bin
    template<class BaseRenderer, class SpanGenerator> class renderer_scanline_bin
    {
    public:
        typedef BaseRenderer  base_ren_type;
        typedef SpanGenerator span_gen_type;

        //--------------------------------------------------------------------
        renderer_scanline_bin() : m_ren(0), m_span_gen(0) {}
        renderer_scanline_bin(base_ren_type& ren, span_gen_type& span_gen) :
            m_ren(&ren),
            m_span_gen(&span_gen)
        {}
        void attach(base_ren_type& ren, span_gen_type& span_gen)
        {
            m_ren = &ren;
            m_span_gen = &span_gen;
        }
        
        //--------------------------------------------------------------------
        void prepare(unsigned max_span_len) 
        { 
            m_span_gen->prepare(max_span_len); 
        }

        //--------------------------------------------------------------------
        template<class Scanline> void render(const Scanline& sl)
        {
            int y = sl.y();
            m_ren->first_clip_box();
            do
            {
                int xmin = m_ren->xmin();
                int xmax = m_ren->xmax();

                if(y >= m_ren->ymin() && y <= m_ren->ymax())
                {
                    unsigned num_spans = sl.num_spans();
                    typename Scanline::const_iterator span = sl.begin();
                    for(;;)
                    {
                        int x = span->x;
                        int len = span->len;

                        if(len < 0) len = -len;
                        if(x < xmin)
                        {
                            len -= xmin - x;
                            x = xmin;
                        }
                        if(len > 0)
                        {
                            if(x + len > xmax)
                            {
                                len = xmax - x + 1;
                            }
                            if(len > 0)
                            {
                                m_ren->blend_color_hspan_no_clip(
                                    x, y, len, 
                                    m_span_gen->generate(x, y, len),
                                    0);
                            }
                        }
                        if(--num_spans == 0) break;
                        ++span;
                    }
                }
            }
            while(m_ren->next_clip_box());
        }
        
    private:
        base_ren_type* m_ren;
        SpanGenerator* m_span_gen;
    };





    //================================================renderer_scanline_direct
    template<class BaseRenderer, class SpanGenerator> class renderer_scanline_direct
    {
    public:
        typedef BaseRenderer  base_ren_type;
        typedef SpanGenerator span_gen_type;
        typedef typename base_ren_type::span_data span_data;

        //--------------------------------------------------------------------
        renderer_scanline_direct() : m_ren(0), m_span_gen(0) {}
        renderer_scanline_direct(base_ren_type& ren, span_gen_type& span_gen) :
            m_ren(&ren),
            m_span_gen(&span_gen)
        {}
        void attach(base_ren_type& ren, span_gen_type& span_gen)
        {
            m_ren = &ren;
            m_span_gen = &span_gen;
        }

        //--------------------------------------------------------------------
        void prepare(unsigned max_span_len) 
        { 
            m_span_gen->prepare(max_span_len); 
        }

        //--------------------------------------------------------------------
        template<class Scanline> void render(const Scanline& sl)
        {
            int y = sl.y();
            m_ren->first_clip_box();
            do
            {
                int xmin = m_ren->xmin();
                int xmax = m_ren->xmax();

                if(y >= m_ren->ymin() && y <= m_ren->ymax())
                {
                    unsigned num_spans = sl.num_spans();
                    typename Scanline::const_iterator span = sl.begin();
                    for(;;)
                    {
                        int x = span->x;
                        int len = span->len;

                        if(len < 0) len = -len;
                        if(x < xmin)
                        {
                            len -= xmin - x;
                            x = xmin;
                        }
                        if(len > 0)
                        {
                            if(x + len > xmax)
                            {
                                len = xmax - x + 1;
                            }
                            if(len > 0)
                            {
                                span_data span = m_ren->span(x, y, len);
                                if(span.ptr)
                                {
                                    m_span_gen->generate(span.x, y, span.len, span.ptr);
                                }
                            }
                        }
                        if(--num_spans == 0) break;
                        ++span;
                    }
                }
            }
            while(m_ren->next_clip_box());
        }
        
    private:
        base_ren_type* m_ren;
        SpanGenerator* m_span_gen;
    };






    //===============================================renderer_scanline_bin_copy
    template<class BaseRenderer, class SpanGenerator> class renderer_scanline_bin_copy
    {
    public:
        typedef BaseRenderer  base_ren_type;
        typedef SpanGenerator span_gen_type;

        //--------------------------------------------------------------------
        renderer_scanline_bin_copy() : m_ren(0), m_span_gen(0) {}
        renderer_scanline_bin_copy(base_ren_type& ren, span_gen_type& span_gen) :
            m_ren(&ren),
            m_span_gen(&span_gen)
        {}
        void attach(base_ren_type& ren, span_gen_type& span_gen)
        {
            m_ren = &ren;
            m_span_gen = &span_gen;
        }
        
        //--------------------------------------------------------------------
        void prepare(unsigned max_span_len) 
        { 
            m_span_gen->prepare(max_span_len); 
        }

        //--------------------------------------------------------------------
        template<class Scanline> void render(const Scanline& sl)
        {
            int y = sl.y();
            m_ren->first_clip_box();
            do
            {
                int xmin = m_ren->xmin();
                int xmax = m_ren->xmax();

                if(y >= m_ren->ymin() && y <= m_ren->ymax())
                {
                    unsigned num_spans = sl.num_spans();
                    typename Scanline::const_iterator span = sl.begin();
                    for(;;)
                    {
                        int x = span->x;
                        int len = span->len;

                        if(len < 0) len = -len;
                        if(x < xmin)
                        {
                            len -= xmin - x;
                            x = xmin;
                        }
                        if(len > 0)
                        {
                            if(x + len > xmax)
                            {
                                len = xmax - x + 1;
                            }
                            if(len > 0)
                            {
                                m_ren->copy_color_hspan_no_clip(
                                    x, y, len, 
                                    m_span_gen->generate(x, y, len));
                            }
                        }
                        if(--num_spans == 0) break;
                        ++span;
                    }
                }
            }
            while(m_ren->next_clip_box());
        }
        
    private:
        base_ren_type* m_ren;
        SpanGenerator* m_span_gen;
    };



    //=============================================renderer_scanline_bin_solid
    template<class BaseRenderer> class renderer_scanline_bin_solid
    {
    public:
        typedef BaseRenderer base_ren_type;
        typedef typename base_ren_type::color_type color_type;

        //--------------------------------------------------------------------
        renderer_scanline_bin_solid() : m_ren(0) {}
        renderer_scanline_bin_solid(base_ren_type& ren) :
            m_ren(&ren)
        {}
        void attach(base_ren_type& ren)
        {
            m_ren = &ren;
        }
        
        //--------------------------------------------------------------------
        void color(const color_type& c) { m_color = c; }
        const color_type& color() const { return m_color; }

        //--------------------------------------------------------------------
        void prepare(unsigned) {}

        //--------------------------------------------------------------------
        template<class Scanline> void render(const Scanline& sl)
        {
            unsigned num_spans = sl.num_spans();
            typename Scanline::const_iterator span = sl.begin();
            for(;;)
            {
                m_ren->blend_hline(span->x, 
                                   sl.y(), 
                                   span->x - 1 + ((span->len < 0) ? 
                                                     -span->len : 
                                                      span->len), 
                                   m_color, 
                                   cover_full);
                if(--num_spans == 0) break;
                ++span;
            }
        }
        
    private:
        base_ren_type* m_ren;
        color_type m_color;
    };
*/












    //===============================================render_scanline_bin_solid
    template<class Scanline, class BaseRenderer, class ColorT> 
    void render_scanline_bin_solid(const Scanline& sl, 
                                   BaseRenderer& ren, 
                                   const ColorT& color)
    {
        unsigned num_spans = sl.num_spans();
        typename Scanline::const_iterator span = sl.begin();
        for(;;)
        {
            ren.blend_hline(span->x, 
                            sl.y(), 
                            span->x - 1 + ((span->len < 0) ? 
                                              -span->len : 
                                               span->len), 
                               color, 
                               cover_full);
            if(--num_spans == 0) break;
            ++span;
        }
    }


    //==============================================render_scanlines_bin_solid
    template<class Rasterizer, class Scanline, 
             class BaseRenderer, class ColorT>
    void render_scanlines_bin_solid(Rasterizer& ras, Scanline& sl, 
                                    BaseRenderer& ren, const ColorT& color)
    {
        if(ras.rewind_scanlines())
        {
            // Explicitly convert "color" to the BaseRenderer color type.
            // For example, it can be called with color type "rgba", while
            // "rgba8" is needed. Otherwise it will be implicitly 
            // converted in the loop many times.
            //----------------------
            typename BaseRenderer::color_type ren_color(color);

            sl.reset(ras.min_x(), ras.max_x());
            while(ras.sweep_scanline(sl))
            {
                //render_scanline_bin_solid(sl, ren, ren_color);

                // This code is equivalent to the above call (copy/paste). 
                // It's just a "manual" optimization for old compilers,
                // like Microsoft Visual C++ v6.0
                //-------------------------------
                unsigned num_spans = sl.num_spans();
                typename Scanline::const_iterator span = sl.begin();
                for(;;)
                {
                    ren.blend_hline(span->x, 
                                    sl.y(), 
                                    span->x - 1 + ((span->len < 0) ? 
                                                      -span->len : 
                                                       span->len), 
                                       ren_color, 
                                       cover_full);
                    if(--num_spans == 0) break;
                    ++span;
                }
            }
        }
    }


    //=============================================renderer_scanline_bin_solid
    template<class BaseRenderer> class renderer_scanline_bin_solid
    {
    public:
        typedef BaseRenderer base_ren_type;
        typedef typename base_ren_type::color_type color_type;

        //--------------------------------------------------------------------
        renderer_scanline_bin_solid() : m_ren(0) {}
        renderer_scanline_bin_solid(base_ren_type& ren) : m_ren(&ren) {}
        void attach(base_ren_type& ren)
        {
            m_ren = &ren;
        }
        
        //--------------------------------------------------------------------
        void color(const color_type& c) { m_color = c; }
        const color_type& color() const { return m_color; }

        //--------------------------------------------------------------------
        void prepare() {}

        //--------------------------------------------------------------------
        template<class Scanline> void render(const Scanline& sl)
        {
            render_scanline_bin_solid(sl, *m_ren, m_color);
        }
        
    private:
        base_ren_type* m_ren;
        color_type m_color;
    };













    //========================================================render_scanlines
    template<class Rasterizer, class Scanline, class Renderer>
    void render_scanlines(Rasterizer& ras, Scanline& sl, Renderer& ren)
    {
        if(ras.rewind_scanlines())
        {
            sl.reset(ras.min_x(), ras.max_x());
            ren.prepare();
            while(ras.sweep_scanline(sl))
            {
                ren.render(sl);
            }
        }
    }


    //========================================================render_all_paths
    template<class Rasterizer, class Scanline, class Renderer, 
             class VertexSource, class ColorStorage, class PathId>
    void render_all_paths(Rasterizer& ras, 
                          Scanline& sl,
                          Renderer& r, 
                          VertexSource& vs, 
                          const ColorStorage& as, 
                          const PathId& path_id,
                          unsigned num_paths)
    {
        for(unsigned i = 0; i < num_paths; i++)
        {
            ras.reset();
            ras.add_path(vs, path_id[i]);
            r.color(as[i]);
            render_scanlines(ras, sl, r);
        }
    }




/*
    template<class Rasterizer, class Scanline, class BaseRenderer>
    void render_scanlines_compound(Rasterizer& ras, 
                                   Scanline& sl, 
                                   BaseRenderer& ren)
    {
        if(ras.rewind_scanlines())
        {
            sl.reset(ras.min_x(), ras.max_x());
            ren.prepare(unsigned(ras.max_x() - ras.min_x() + 2));

            unsigned num_styles;
            while((num_styles = ras.sweep_styles()) > 0)
            {
                unsigned i;
                for(i = 0; i < num_styles; i++)
                {
                    if(ras.sweep_scanline(sl, i))
                    {
                        ren.color(st[ras.style(i)]);
                        ren.render(sl);
                    }
                }
            }
        }

    }
*/




}

#endif
