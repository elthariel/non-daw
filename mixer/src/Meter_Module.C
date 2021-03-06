
/*******************************************************************************/
/* Copyright (C) 2009 Jonathan Moore Liles                                     */
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

#include "const.h"

#include <math.h>
#include <FL/Fl.H>
#include <FL/Fl_Single_Window.H>

#include "FL/Fl_Scalepack.H"
#include "FL/test_press.H"

#include "Meter_Module.H"
#include "DPM.H"
#include "JACK/Port.H"



const float METER_UPDATE_FREQ = 0.1f;



Meter_Module::Meter_Module ( )
    : Module ( 50, 100, name() )
{
    box( FL_THIN_UP_FRAME );
    dpm_pack = new Fl_Scalepack( x(), y(), w(), h() );
    dpm_pack->type( FL_HORIZONTAL );

    control_value = 0;

    color( FL_BLACK );

    end();

    Port p( this, Port::OUTPUT, Port::CONTROL, "dB level" );
    p.hints.type = Port::Hints::LOGARITHMIC;
    p.hints.ranged = true;
    p.hints.maximum = 6.0f;
    p.hints.minimum = -70.0f;
    p.hints.dimensions = 1;
    p.connect_to( new float[1] );
    p.control_value_no_callback( -70.0f );

    add_port( p );

    Fl::add_timeout( METER_UPDATE_FREQ, update_cb, this );

    log_create();
}

Meter_Module::~Meter_Module ( )
{
    if ( control_value )
        delete[] control_value;

    Fl::remove_timeout( update_cb, this );

    log_destroy();
}



void
Meter_Module::update_cb ( void *v )
{
    ((Meter_Module*)v)->update_cb();
}

void
Meter_Module::update_cb ( void )
{
    Fl::repeat_timeout( METER_UPDATE_FREQ, update_cb, this );

    for ( int i = dpm_pack->children(); i--; )
        ((DPM*)dpm_pack->child( i ))->value( control_value[i] );
}

bool
Meter_Module::configure_inputs ( int n )
{
    THREAD_ASSERT( UI );

    int tx, ty, tw, th;
    bbox( tx,ty,tw,th );

    int on = audio_input.size();

    if ( n > on )
    {
        for ( int i = on; i < n; ++i )
        {
            DPM *dpm = new DPM( tx, ty, tw, th );
            dpm->type( FL_VERTICAL );
            align( (Fl_Align)(FL_ALIGN_CENTER | FL_ALIGN_INSIDE ) );

            dpm_pack->add( dpm );

            add_port( Port( this, Port::INPUT, Port::AUDIO ) );
            add_port( Port( this, Port::OUTPUT, Port::AUDIO ) );

        }
    }
    else
    {
        for ( int i = on; i > n; --i )
        {
            DPM *dpm = (DPM*)dpm_pack->child( dpm_pack->children() - 1 );
            dpm_pack->remove( dpm );
            delete dpm;

            audio_input.back().disconnect();
            audio_input.pop_back();
            audio_output.back().disconnect();
            audio_output.pop_back();
        }
    }

    control_output[0].hints.dimensions = n;
    delete[] (float*)control_output[0].buffer();
    {
        float *f = new float[n];

        for ( int i = n; i--; )
            f[i] = -70.0f;

        control_output[0].connect_to( f );
    }

    if ( control_value )
        delete [] control_value;

    control_value = new float[n];
    for ( int i = n; i--; )
        control_value[i] = -70.0f;

    if ( control_output[0].connected() )
        control_output[0].connected_port()->module()->handle_control_changed( control_output[0].connected_port() );

    return true;
}



int
Meter_Module::handle ( int m )
{
    switch ( m )
    {
        case FL_PUSH:
        {
            if ( test_press( FL_BUTTON1 ) )
            {
                /* don't let Module::handle eat our click */
                return Fl_Group::handle( m );
            }
            return Module::handle( m );
        }
    }

    return Module::handle( m );
}



/**********/
/* Engine */
/**********/

static float
get_peak_sample ( const sample_t* buf, nframes_t nframes )
{
    float p = 0.0f;

    const sample_t *f = buf;

    for ( int j = nframes; j--; ++f )
    {
        float s = *f;

        /* rectify */
        if ( s < 0.0f )
            s = 0 - s;

        if ( s > p )
            p = s;
    }

    return p;
}

void
Meter_Module::process ( nframes_t nframes )
{
    for ( unsigned int i = 0; i < audio_input.size(); ++i )
    {
        if ( audio_input[i].connected() )
        {
            float dB = 20 * log10( get_peak_sample( (float*)audio_input[i].buffer(), nframes ) / 2.0f );

            ((float*)control_output[0].buffer())[i] = dB;
            control_value[i] = dB;
        }
    }
}
