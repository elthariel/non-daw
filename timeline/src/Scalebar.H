
/*******************************************************************************/
/* Copyright (C) 2008 Jonathan Moore Liles                                     */
/*                                                                             */
/* This program is free software; you can redistribute it and/or modify it     */
/* under the terms of the GNU General Public License as published by the       */
/* Free Software Foundation; either version 2 of the License, or (at your      */
/* option) any later version.                                                  */
/*                                                                             */
/* This program is distributed in the hope that it will be useful, but WITHOUT */
/* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or       */
/* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for   */
/* more details.                                                               */
/*                                                                             */
/* You should have received a copy of the GNU General Public License along     */
/* with This program; see the file COPYING.  If not,write to the Free Software */
/* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */
/*******************************************************************************/


#pragma once

#include <FL/Fl.H>
#include <FL/Fl_Scrollbar.H>
#include <FL/Fl_Slider.H>

#include <stdio.h>

class Scalebar : public Fl_Scrollbar
{
    int _zoom;

    bool _zoom_changed;

    int _zoom_min;
    int _zoom_max;

    void constrain ( void )
        {
            if ( _zoom > _zoom_max )
                _zoom = _zoom_max;
            else
                if ( _zoom < _zoom_min )
                    _zoom = _zoom_min;
        }

    void maybe_do_callback ( int z )
        {
            if ( z != _zoom )
            {
                _zoom_changed = true;
                do_callback();
                _zoom_changed = false;

                slider_size( w() / maximum() );
            }

        }

public:

    Scalebar ( int X, int Y, int W, int H ) : Fl_Scrollbar ( X, Y, W, H )
        {
            _zoom = 1;
            _zoom_min = 0;
            _zoom_max = 4;
            _zoom_changed = false;

            step( 1 );
        }

    bool zoom_changed ( void ) const { return _zoom_changed; }
    double zoom ( void ) const { return _zoom; }
    void zoom ( int v ) { int z = _zoom ; _zoom = v; constrain(); maybe_do_callback( z ); }
//    double value ( void ) const { return Fl_Slider::value(); }
    void zoom_range ( int zmin, int zmax ) { _zoom_min = zmin; _zoom_max = zmax; }

    void zoom_out ( void ) { int z = _zoom; ++_zoom; constrain(); maybe_do_callback( z ); }
    void zoom_in ( void ) {int z = _zoom; --_zoom; constrain(); maybe_do_callback( z ); }

    int
    handle ( int m )
        {
            switch ( m )
            {
                case FL_MOUSEWHEEL:
                {
                    if ( ! Fl::event_ctrl() )
                        return 0;

                    int d = Fl::event_dy();

                    double z = _zoom;

                    _zoom += d;

                    constrain();

                    maybe_do_callback( z );

                    return 1;
                }
                default:
                    return Fl_Scrollbar::handle( m );
            }
        }
};
