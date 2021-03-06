
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

#include "Sequence_Region.H"
#include "Track.H"



Sequence_Region::Sequence_Region ( )
{
    color( FL_CYAN );
}

Sequence_Region::Sequence_Region ( const Sequence_Region &rhs ) : Sequence_Widget( rhs )
{
}

Sequence_Region::~Sequence_Region ( )
{
}



void
Sequence_Region::get ( Log_Entry &e ) const
{
    e.add( ":color",  _box_color );
    e.add( ":length", _r->length );

    Sequence_Widget::get( e );
}

void
Sequence_Region::set ( Log_Entry &e )
{
    for ( int i = 0; i < e.size(); ++i )
    {
        const char *s, *v;

        e.get( i, &s, &v );

        if ( ! strcmp( s, ":color" ) )
            _box_color = (Fl_Color)atoll( v );
        else if ( ! strcmp( s, ":length" ) )
            _r->length = atoll( v );

    }

    Sequence_Widget::set( e );
}

void
Sequence_Region::trim_left ( nframes_t where )
{
    long td = where - _r->start;

    if ( td < 0 && _r->offset < (nframes_t)( 0 - td ) )
        td = 0 - _r->offset;

    if ( td > 0 && (nframes_t)td >= _r->length )
        td = _r->length - timeline->x_to_ts( 1 );

    _r->trim_left( 0 - td );

    nframes_t f = _r->start;

    /* snap to beat/bar lines */
    if ( timeline->nearest_line( &f ) )
        _r->set_left( f );

}

void
Sequence_Region::trim_right ( nframes_t where )
{
    long td =  ( _r->start + _r->length ) - where;

    if ( td >= 0 && _r->length < (nframes_t)td )
        td = _r->length - timeline->x_to_ts( 1 );

    _r->trim_right( 0 - td );

    nframes_t f = _r->start + _r->length;

    /* snap to beat/bar lines */
    if ( timeline->nearest_line( &f ) )
        _r->set_right( f );

}

void
Sequence_Region::trim ( enum trim_e t, int X )
{
    redraw();

    nframes_t where = timeline->x_to_offset( X );

    switch ( t )
    {
        case LEFT:
            trim_left( where );
            break;
        case RIGHT:
            trim_right( where );
            break;
        default:
            break;
    }
}

/** split region at absolute frame /where/. due to inheritance issues,
 * the copy must be made in the derived classed and passed in */
void
Sequence_Region::split ( Sequence_Region * copy, nframes_t where )
{
    trim_right( where );
    copy->trim_left( where );
    sequence()->add( copy );
}


#include "FL/test_press.H"

int
Sequence_Region::handle ( int m )
{
    static enum trim_e trimming;

    static bool copied = false;

    int X = Fl::event_x();
    int Y = Fl::event_y();

    Logger _log( this );

    switch ( m )
    {
        case FL_PUSH:
        {
            /* trimming */
            if ( Fl::event_shift() && ! Fl::event_ctrl() )
            {
                switch ( Fl::event_button() )
                {
                    case 1:
                        trim( trimming = LEFT, X );
                        begin_drag( Drag( x() - X, y() - Y ) );
                        _log.hold();
                        break;
                    case 3:
                        trim( trimming = RIGHT, X );
                        begin_drag( Drag( x() - X, y() - Y ) );
                        _log.hold();
                        break;
                    default:
                        return 0;
                        break;
                }

                fl_cursor( FL_CURSOR_WE );
                return 1;
            }
            else if ( test_press( FL_BUTTON2 ) )
            {
                if ( Sequence_Widget::current() == this )
                {
                    if ( selected() )
                        deselect();
                    else
                        select();
                }

                redraw();
                return 1;
            }

/*             else if ( test_press( FL_CTRL + FL_BUTTON1 ) ) */
/*             { */
/*                 /\* duplication *\/ */
/*                 fl_cursor( FL_CURSOR_MOVE ); */
/*                 return 1; */
/*             } */

            else
                return Sequence_Widget::handle( m );
        }
        case FL_RELEASE:
        {
            Sequence_Widget::handle( m );

            copied = false;
            if ( trimming != NO )
                trimming = NO;

            return 1;
        }
        case FL_DRAG:
        {
            if ( ! _drag )
            {
                begin_drag( Drag( x() - X, y() - Y, x_to_offset( X ) ) );
                _log.hold();
            }

            /* trimming */
            if ( Fl::event_shift() )
            {
                if ( trimming )
                {
                    trim( trimming, X );
                    return 1;
                }
                else
                    return 0;
            }

            return Sequence_Widget::handle( m );
        }
        default:
            return Sequence_Widget::handle( m );
            break;
    }

    return 0;

}

void
Sequence_Region::draw_box ( void )
{
    fl_draw_box( box(), line_x(), y(), abs_w(), h(), box_color() );
}

void
Sequence_Region::draw ( void )
{
}
