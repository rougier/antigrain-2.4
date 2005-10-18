//----------------------------------------------------------------------------
// Anti-Grain Geometry - Version 2.3
// Copyright (C) 2002-2005 Maxim Shemanarev (http://www.antigrain.com)
//
// Permission to copy, use, modify, sell and distribute this software 
// is granted provided this copyright notice appears in all copies. 
// This software is provided "as is" without express or implied
// warranty, and with no claim as to its suitability for any purpose.
//
//----------------------------------------------------------------------------
//
// The author gratefully acknowleges the support of David Turner, 
// Robert Wilhelm, and Werner Lemberg - the authors of the FreeType 
// libray - in producing this work. See http://www.freetype.org for details.
//
//----------------------------------------------------------------------------
// Contact: mcseem@antigrain.com
//          mcseemagg@yahoo.com
//          http://www.antigrain.com
//----------------------------------------------------------------------------
//
// Adaptation for 32-bit screen coordinates has been sponsored by 
// Liberty Technology Systems, Inc., visit http://lib-sys.com
//
// Liberty Technology Systems, Inc. is the provider of
// PostScript and PDF technology for software developers.
// 
//----------------------------------------------------------------------------
#ifndef AGG_RASTERIZER_COMPOUND_AA_INCLUDED
#define AGG_RASTERIZER_COMPOUND_AA_INCLUDED

#include "agg_rasterizer_cells_aa.h"

namespace agg
{

    //-----------------------------------------------------------cell_style_aa
    // A pixel cell. There're no constructors defined and it was done 
    // intentionally in order to avoid extra overhead when allocating an 
    // array of cells.
    struct cell_style_aa
    {
        int   x;
        int   y;
        int   cover;
        int   area;
        int16 left, right;

        void initial()
        {
            x     = 0x7FFFFFFF;
            y     = 0x7FFFFFFF;
            cover = 0;
            area  = 0;
            left  = -1;
            right = -1;
        }

        bool operator != (const cell_style_aa& c) const
        {
            return left != c.left || right != c.right;
        }
    };

    //------------------------------------------------------------------------
    template<unsigned XScale=1, unsigned AA_Shift=8> class rasterizer_compound_aa
    {
        enum status
        {
            status_initial,
            status_line_to,
            status_closed
        };

        struct style_info 
        { 
            unsigned start_cell;
            unsigned num_cells;
            int      last_x;
        };

        struct cell_info
        {
            int x, area, cover; 
        };

    public:
        enum aa_scale_e
        {
            aa_shift = AA_Shift,
            aa_num   = 1 << aa_shift,
            aa_mask  = aa_num - 1,
            aa_2num  = aa_num * 2,
            aa_2mask = aa_2num - 1
        };

        //--------------------------------------------------------------------
        rasterizer_compound_aa() : 
            m_min_style(0x7FFFFFFF),
            m_max_style(-0x7FFFFFFF)
        {
        }

        //--------------------------------------------------------------------
        void reset(); 
        void styles(int left, int right);
        void add_vertex(double x, double y, unsigned cmd);
        void move_to(int x, int y);
        void line_to(int x, int y);
        void move_to_d(double x, double y);
        void line_to_d(double x, double y);
        void add_xy(const double* x, const double* y, unsigned n);

        //-------------------------------------------------------------------
        template<class VertexSource>
        void add_path(VertexSource& vs, unsigned path_id=0)
        {
            double x;
            double y;

            unsigned cmd;
            vs.rewind(path_id);
            while(!is_stop(cmd = vs.vertex(&x, &y)))
            {
                add_vertex(x, y, cmd);
            }
        }

        
        //--------------------------------------------------------------------
        int min_x()     const { return m_outline.min_x(); }
        int min_y()     const { return m_outline.min_y(); }
        int max_x()     const { return m_outline.max_x(); }
        int max_y()     const { return m_outline.max_y(); }
        int min_style() const { return m_min_style; }
        int max_style() const { return m_max_style; }

        //--------------------------------------------------------------------
        void sort();
        bool rewind_scanlines();
        void add_style(int style_id);
        unsigned sweep_styles();
        unsigned style(unsigned style_idx) const;

        //--------------------------------------------------------------------
        AGG_INLINE unsigned calculate_alpha(int area) const
        {
            int cover = area >> (poly_base_shift*2 + 1 - aa_shift);

            if(cover < 0) cover = -cover;

            //cover &= aa_2mask;
            //if(cover > aa_num)
            //{
            //    cover = aa_2num - cover;
            //}

            if(cover > aa_mask) cover = aa_mask;
            return cover;
        }


        //--------------------------------------------------------------------
        // Sweeps one scanline with one style index. The style ID can be 
        // determined by calling style(). 
        template<class Scanline> bool sweep_scanline(Scanline& sl, int style_idx)
        {
            int cur_y = m_cur_y - 1;
            if(cur_y > m_outline.max_y()) return false;

            sl.reset_spans();

            if(style_idx < 0) style_idx  = 0;
            else              style_idx++;

            const style_info& st = m_styles[m_ast[style_idx]];

            unsigned num_cells = st.num_cells;
            cell_info* cell = &m_cells[st.start_cell];

            int cover = 0;
            while(num_cells--)
            {
                unsigned alpha;
                int x = cell->x;
                int area = cell->area;

                cover += cell->cover;

                ++cell;

                if(area)
                {
                    alpha = calculate_alpha((cover << (poly_base_shift + 1)) - area);
                    sl.add_cell(x, alpha);
                    x++;
                }

                if(num_cells && cell->x > x)
                {
                    alpha = calculate_alpha(cover << (poly_base_shift + 1));
                    if(alpha)
                    {
                        sl.add_span(x, cell->x - x, alpha);
                    }
                }
            }

            if(sl.num_spans() == 0) return false;
            sl.finalize(cur_y);
            return true;

        }

    private:
        //--------------------------------------------------------------------
        // Disable copying
        rasterizer_compound_aa(const rasterizer_compound_aa<XScale, AA_Shift>&);
        const rasterizer_compound_aa<XScale, AA_Shift>& 
            operator = (const rasterizer_compound_aa<XScale, AA_Shift>&);

    private:
        rasterizer_cells_aa<cell_style_aa> m_outline;
        pod_vector<style_info> m_styles;  // Active Styles
        pod_vector<unsigned>   m_ast;     // Active Style Table (unique values)
        pod_vector<int8u>      m_asm;     // Active Style Mask 
        pod_vector<cell_info>  m_cells;

        int m_min_style;
        int m_max_style;
        int m_cur_y;
    };










    //------------------------------------------------------------------------
    template<unsigned XScale, unsigned AA_Shift> 
    void rasterizer_compound_aa<XScale, AA_Shift>::reset() 
    { 
        m_outline.reset(); 
        m_min_style =  0x7FFFFFFF;
        m_max_style = -0x7FFFFFFF;
    }

    //------------------------------------------------------------------------
    template<unsigned XScale, unsigned AA_Shift> 
    void rasterizer_compound_aa<XScale, AA_Shift>::styles(int left, int right)
    {
        cell_style_aa cell;
        cell.initial();
        cell.left = (int16)left;
        cell.right = (int16)right;
        m_outline.seed_cell(cell);
        if(left  >= 0 && left  < m_min_style) m_min_style = left;
        if(left  >= 0 && left  > m_max_style) m_max_style = left;
        if(right >= 0 && right < m_min_style) m_min_style = right;
        if(right >= 0 && right > m_max_style) m_max_style = right;
    }

    //------------------------------------------------------------------------
    template<unsigned XScale, unsigned AA_Shift> 
    void rasterizer_compound_aa<XScale, AA_Shift>::move_to(int x, int y)
    {
        if(m_outline.sorted()) 
        {
            reset();
        }
        m_outline.move_to(x * XScale, y); 
    }


    //------------------------------------------------------------------------
    template<unsigned XScale, unsigned AA_Shift> 
    void rasterizer_compound_aa<XScale, AA_Shift>::line_to(int x, int y)
    {
        m_outline.line_to(x * XScale, y); 
    }


    //------------------------------------------------------------------------
    template<unsigned XScale, unsigned AA_Shift> 
    void rasterizer_compound_aa<XScale, AA_Shift>::add_vertex(double x, double y, unsigned cmd)
    {
        if(is_move_to(cmd)) 
        {
            move_to(poly_coord(x), poly_coord(y));
        }
        else 
        {
            if(is_vertex(cmd))
            {
                line_to(poly_coord(x), poly_coord(y));
            }
        }
    }

    //------------------------------------------------------------------------
    template<unsigned XScale, unsigned AA_Shift> 
    void rasterizer_compound_aa<XScale, AA_Shift>::move_to_d(double x, double y) 
    { 
        move_to(poly_coord(x), poly_coord(y)); 
    }

    //------------------------------------------------------------------------
    template<unsigned XScale, unsigned AA_Shift> 
    void rasterizer_compound_aa<XScale, AA_Shift>::line_to_d(double x, double y) 
    { 
        line_to(poly_coord(x), poly_coord(y)); 
    }

    //------------------------------------------------------------------------
    template<unsigned XScale, unsigned AA_Shift> 
    void rasterizer_compound_aa<XScale, AA_Shift>::add_xy(const double* x, 
                                                          const double* y, 
                                                          unsigned n)
    {
        if(n > 2)
        {
            move_to_d(*x++, *y++);
            --n;
            do
            {
                line_to_d(*x++, *y++);
            }
            while(--n);
        }
    }

    //--------------------------------------------------------------------
    template<unsigned XScale, unsigned AA_Shift> 
    AGG_INLINE void rasterizer_compound_aa<XScale, AA_Shift>::sort()
    {
        m_outline.sort_cells();
    }

    //--------------------------------------------------------------------
    template<unsigned XScale, unsigned AA_Shift> 
    AGG_INLINE bool rasterizer_compound_aa<XScale, AA_Shift>::rewind_scanlines()
    {
        m_outline.sort_cells();
        if(m_outline.total_cells() == 0) 
        {
            return false;
        }
        if(m_max_style < m_min_style)
        {
            return false;
        }
        m_cur_y = m_outline.min_y();
        m_styles.allocate(m_max_style - m_min_style + 2, 128);
        return true;
    }

    //--------------------------------------------------------------------
    template<unsigned XScale, unsigned AA_Shift> 
    AGG_INLINE void 
    rasterizer_compound_aa<XScale, AA_Shift>::add_style(int style_id)
    {
        if(style_id < 0) style_id  = 0;
        else             style_id -= m_min_style - 1;

        unsigned nbyte = style_id >> 3;
        unsigned mask = 1 << (style_id & 7);

        style_info* style = &m_styles[style_id];
        if((m_asm[nbyte] & mask) == 0)
        {
            m_ast.add(style_id);
            m_asm[nbyte] |= mask;
            style->start_cell = 0;
            style->num_cells = 0;
            style->last_x = -0x7FFFFFFF;
        }
        ++style->start_cell;
    }

    //--------------------------------------------------------------------
    // Returns the number of styles
    template<unsigned XScale, unsigned AA_Shift> 
    unsigned rasterizer_compound_aa<XScale, AA_Shift>::sweep_styles()
    {
        for(;;)
        {
            if(m_cur_y > m_outline.max_y()) return 0;
            unsigned num_cells = m_outline.scanline_num_cells(m_cur_y);
            const cell_style_aa* const* cells = m_outline.scanline_cells(m_cur_y);
            unsigned num_styles = m_max_style - m_min_style + 2;
            const cell_style_aa* cur_cell;
            unsigned style_id;
            style_info* style;
            cell_info* cell;

            m_cells.allocate(num_cells * 2, 256); // Each cell can have two styles
            m_ast.capacity(num_styles, 64);
            m_asm.allocate((num_styles + 7) >> 3, 8);
            m_asm.zero();

            // Pre-add zero (for no-fill style, that is, -1).
            // We need that to ensure that the "-1 style" would go first.
            m_asm[0] |= 1; 
            m_ast.add(0);
            style = &m_styles[0];
            style->start_cell = 0;
            style->num_cells = 0;
            style->last_x = -0x7FFFFFFF;

            while(num_cells--)
            {
                cur_cell = *cells++;
                add_style(cur_cell->left);
                add_style(cur_cell->right);
            }

            // Convert the Y-histogram into the array of starting indexes
            unsigned i;
            unsigned start_cell = 0;
            for(i = 0; i < m_ast.size(); i++)
            {
                style_info& st = m_styles[m_ast[i]];
                unsigned v = st.start_cell;
                st.start_cell = start_cell;
                start_cell += v;
            }

            cells = m_outline.scanline_cells(m_cur_y);
            num_cells = m_outline.scanline_num_cells(m_cur_y);

            while(num_cells--)
            {
                cur_cell = *cells++;
                style_id = (cur_cell->left < 0) ? 0 : 
                            cur_cell->left - m_min_style + 1;

                style = &m_styles[style_id];
                if(cur_cell->x == style->last_x)
                {
                    cell = &m_cells[style->start_cell + style->num_cells - 1];
                    cell->area  += cur_cell->area;
                    cell->cover += cur_cell->cover;
                }
                else
                {
                    cell = &m_cells[style->start_cell + style->num_cells];
                    cell->x       = cur_cell->x;
                    cell->area    = cur_cell->area;
                    cell->cover   = cur_cell->cover;
                    style->last_x = cur_cell->x;
                    style->num_cells++;
                }

                style_id = (cur_cell->right < 0) ? 0 : 
                            cur_cell->right - m_min_style + 1;

                style = &m_styles[style_id];
                if(cur_cell->x == style->last_x)
                {
                    cell = &m_cells[style->start_cell + style->num_cells - 1];
                    cell->area  -= cur_cell->area;
                    cell->cover -= cur_cell->cover;
                }
                else
                {
                    cell = &m_cells[style->start_cell + style->num_cells];
                    cell->x       =  cur_cell->x;
                    cell->area    = -cur_cell->area;
                    cell->cover   = -cur_cell->cover;
                    style->last_x =  cur_cell->x;
                    style->num_cells++;
                }
            }
            if(m_ast.size()) break;
            ++m_cur_y;
        }
        ++m_cur_y;
        return m_ast.size() - 1;
    }

    //--------------------------------------------------------------------
    // Returns style ID depending of the existing style index
    template<unsigned XScale, unsigned AA_Shift> 
    AGG_INLINE unsigned 
    rasterizer_compound_aa<XScale, AA_Shift>::style(unsigned style_idx) const
    {
        return m_ast[style_idx + 1] + m_min_style - 1;
    }


}



#endif

