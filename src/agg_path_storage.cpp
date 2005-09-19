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
// Class path_storage
//
//----------------------------------------------------------------------------
#include "agg_path_storage.h"


namespace agg
{
/*

    //------------------------------------------------------------------------
    void path_storage::rewind(unsigned path_id)
    {
        m_iterator = path_id; 
    }



    //------------------------------------------------------------------------
    void path_storage::end_poly(unsigned flags)
    {
    }


    //------------------------------------------------------------------------
    unsigned path_storage::start_new_path()
    {
        if(m_total_vertices)
        {
            if(!is_stop(command(m_total_vertices - 1)))
            {
                add_vertex(0.0, 0.0, path_cmd_stop);
            }
        }
        return m_total_vertices;
    }


    //------------------------------------------------------------------------
    void path_storage::add_poly(const double* vertices, unsigned num,
                                bool solid_path, unsigned end_flags)
    {
        if(num)
        {
            if(!solid_path)
            {
                move_to(vertices[0], vertices[1]);
                vertices += 2;
                --num;
            }
            while(num--)
            {
                line_to(vertices[0], vertices[1]);
                vertices += 2;
            }
            if(end_flags) end_poly(end_flags);
        }
    }


    //------------------------------------------------------------------------
    unsigned path_storage::perceive_polygon_orientation(unsigned start, unsigned end)
    {
        // Calculate signed area (double area to be exact)
        //---------------------
        unsigned np = end - start;
        double area = 0.0;
        unsigned i;
        for(i = 0; i < np; i++)
        {
            double x1, y1, x2, y2;
            vertex(start + i,            &x1, &y1);
            vertex(start + (i + 1) % np, &x2, &y2);
            area += x1 * y2 - y1 * x2;
        }
        return (area < 0.0) ? path_flags_cw : path_flags_ccw;
    }


    //------------------------------------------------------------------------
    void path_storage::invert_polygon(unsigned start, unsigned end)
    {
        unsigned i;
        unsigned tmp_cmd = command(start);
        
        --end; // Make "end" inclusive

        // Shift all commands to one position
        for(i = start; i < end; i++)
        {
            modify_command(i, command(i + 1));
        }

        // Assign starting command to the ending command
        modify_command(end, tmp_cmd);

        // Reverse the polygon
        while(end > start)
        {
            unsigned start_nb = start >> block_shift;
            unsigned end_nb   = end   >> block_shift;
            double* start_ptr = m_coord_blocks[start_nb] + ((start & block_mask) << 1);
            double* end_ptr   = m_coord_blocks[end_nb]   + ((end   & block_mask) << 1);
            double tmp_xy;

            tmp_xy       = *start_ptr;
            *start_ptr++ = *end_ptr;
            *end_ptr++   = tmp_xy;

            tmp_xy       = *start_ptr;
            *start_ptr   = *end_ptr;
            *end_ptr     = tmp_xy;

            tmp_cmd = m_cmd_blocks[start_nb][start & block_mask];
            m_cmd_blocks[start_nb][start & block_mask] = m_cmd_blocks[end_nb][end & block_mask];
            m_cmd_blocks[end_nb][end & block_mask] = (unsigned char)tmp_cmd;

            ++start;
            --end;
        }
    }


    //------------------------------------------------------------------------
    unsigned path_storage::arrange_polygon_orientation(unsigned start, 
                                                       path_flags_e orientation)
    {
        if(orientation == path_flags_none) return start;
        
        // Skip all non-vertices at the beginning
        while(start < m_total_vertices && !is_vertex(command(start))) ++start;

        // Skip all insignificant move_to
        while(start+1 < m_total_vertices && 
              is_move_to(command(start)) &&
              is_move_to(command(start+1))) ++start;

        // Find the last vertex
        unsigned end = start + 1;
        while(end < m_total_vertices && !is_next_poly(command(end))) ++end;

        if(end - start > 2)
        {
            if(perceive_polygon_orientation(start, end) != unsigned(orientation))
            {
                // Invert polygon, set orientation flag, and skip all end_poly
                invert_polygon(start, end);
                unsigned cmd;
                while(end < m_total_vertices && is_end_poly(cmd = command(end)))
                {
                    modify_command(end++, set_orientation(cmd, orientation));
                }
            }
        }
        return end;
    }


    //------------------------------------------------------------------------
    unsigned path_storage::arrange_orientations(unsigned start, 
                                                path_flags_e orientation)
    {
        if(orientation != path_flags_none)
        {
            while(start < m_total_vertices)
            {
                start = arrange_polygon_orientation(start, orientation);
                if(is_stop(command(start)))
                {
                    ++start;
                    break;
                }
            }
        }
        return start;
    }


    //------------------------------------------------------------------------
    void path_storage::arrange_orientations_all_paths(path_flags_e orientation)
    {
        if(orientation != path_flags_none)
        {
            unsigned start = 0;
            while(start < m_total_vertices)
            {
                start = arrange_orientations(start, orientation);
            }
        }
    }


    //------------------------------------------------------------------------
    void path_storage::flip_x(double x1, double x2)
    {
        unsigned i;
        double x, y;
        for(i = 0; i < m_total_vertices; i++)
        {
            unsigned cmd = vertex(i, &x, &y);
            if(is_vertex(cmd))
            {
                modify_vertex(i, x2 - x + x1, y);
            }
        }
    }


    //------------------------------------------------------------------------
    void path_storage::flip_y(double y1, double y2)
    {
        unsigned i;
        double x, y;
        for(i = 0; i < m_total_vertices; i++)
        {
            unsigned cmd = vertex(i, &x, &y);
            if(is_vertex(cmd))
            {
                modify_vertex(i, x, y2 - y + y1);
            }
        }
    }
*/

}

