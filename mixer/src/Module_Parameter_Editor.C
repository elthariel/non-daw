
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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <FL/fl_draw.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Menu_Button.H>
#include <FL/Fl_Counter.H>
#include "FL/Boxtypes.H"
#include "FL/Fl_Flowpack.H"
#include "FL/Fl_Labelpad_Group.H"
#include "FL/Fl_Value_SliderX.H"
#include "FL/Fl_Arc_Dial.H"

#include "Module.H"
#include "Module_Parameter_Editor.H"
#include "Controller_Module.H"
#include "Chain.H"

#include "debug.h"



Module_Parameter_Editor::Module_Parameter_Editor ( Module *module ) : Fl_Double_Window( 0, 0, 800, 600 )
{
    _module = module;
    _resized = false;

    char lab[256];
    if ( strcmp( module->name(), module->label() ) )
    {
        snprintf( lab, sizeof( lab ), "%s : %s", module->name(), module->label() );
    }
    else
        strcpy( lab, module->label() );


    char title[512];
    snprintf( title, sizeof( title ), "%s - %s - %s", "Mixer", module->chain()->name(), lab );

    label( title );

    { Fl_Pack *o = main_pack = new Fl_Pack( 0, y(), w(), h() - 10 );
        o->type( FL_VERTICAL );
/*         o->label( strdup( lab ) ); */
/*         o->labeltype( FL_SHADOW_LABEL ); */
/*         o->labelsize( 18 ); */
/*         o->align( FL_ALIGN_TOP | FL_ALIGN_RIGHT | FL_ALIGN_INSIDE ); */


        { Fl_Pack *o = new Fl_Pack( 0, 0, 50, 25 );
            o->type( FL_HORIZONTAL );
            o->spacing( 20 );

            { Fl_Menu_Button *o = mode_choice = new Fl_Menu_Button( 0, 0, 25, 25 );
                o->add( "Knobs" );
                o->add( "Horizontal Sliders" );
                o->add( "Vertical Sliders" );
                o->label( NULL );
                o->value( 0 );
                o->when( FL_WHEN_CHANGED );
                o->callback( cb_mode_handle, this );
            }

/*             { Fl_Box *o = new Fl_Box( 0, 0, 300, 25 ); */
/*                 o->box( FL_ROUNDED_BOX ); */
/*                 o->color( FL_YELLOW ); */
/*                 o->label( strdup( lab ) ); */
/*                 o->labeltype( FL_SHADOW_LABEL ); */
/*                 o->labelsize( 18 ); */
/*                 o->align( (Fl_Align)(FL_ALIGN_TOP | FL_ALIGN_RIGHT | FL_ALIGN_INSIDE ) ); */
/* //                Fl_Group::current()->resizable( o ); */
/*             } */
            o->end();
        }
        { Fl_Group *o = new Fl_Group( 0, 0, w(), h() );
            { Fl_Flowpack *o = control_pack = new Fl_Flowpack( 50, 0, w() - 100, h() );
/*                 o->box( FL_ROUNDED_BOX ); */
/*                 o->color( FL_GRAY ); */
                o->vspacing( 10 );
                o->hspacing( 10 );
                o->end();
            }
            o->resizable( 0 );
            o->end();
        }
        o->end();
    }

    end();

//    draw();

    make_controls();
}

Module_Parameter_Editor::~Module_Parameter_Editor ( )
{


}



void
Module_Parameter_Editor::make_controls ( void )
{
    Module *module = _module;

    control_pack->clear();

    for ( unsigned int i = 0; i < module->control_input.size(); ++i )
    {
        Fl_Widget *w;

        Module::Port *p = &module->control_input[i];

        if ( p->hints.type == Module::Port::Hints::BOOLEAN )
        {
            Fl_Button *o = new Fl_Button( 0, 0, 30, 30, p->name() );
            w = o;
            o->selection_color( FL_GREEN );
            o->type( FL_TOGGLE_BUTTON );
            o->value( p->control_value() );

        }
        else if ( p->hints.type == Module::Port::Hints::INTEGER )
        {

            Fl_Counter *o = new Fl_Counter(0, 0, 58, 24, p->name() );
            w = o;

            o->type(1);
            o->step(1);

            if ( p->hints.ranged )
            {
                o->minimum( p->hints.minimum );
                o->maximum( p->hints.maximum );
            }

            o->value( p->control_value() );

        }
        else
        {
            if ( mode_choice->value() == 0 )
            {
                Fl_Arc_Dial *o = new Fl_Arc_Dial( 0, 0, 50, 50, p->name() );
                w = o;
                if ( p->hints.ranged )
                {
                    o->minimum( p->hints.minimum );
                    o->maximum( p->hints.maximum );
                }

                o->box( FL_BURNISHED_OVAL_BOX );
                o->color( fl_darker( fl_darker( FL_GRAY ) ) );
                o->selection_color( FL_WHITE );
                o->value( p->control_value() );

//                o->step( fabs( ( o->maximum() - o->minimum() ) ) / 32.0f );
            }
            else
            {
                Fl_Value_SliderX *o = new Fl_Value_SliderX( 0, 0, 120, 24, p->name() );
                w = o;

                if ( mode_choice->value() == 1 )
                {
                    o->type( FL_HORIZONTAL );
                    o->size( 120, 24 );
                    if ( p->hints.ranged )
                    {
                        o->minimum( p->hints.minimum );
                        o->maximum( p->hints.maximum );
                    }
                }
                else
                {
                    o->type( FL_VERTICAL );
                    o->size( 32, 120 );
                    /* have to reverse the meaning of these to get the
                     * orientation of the slider right */
                    o->maximum( p->hints.minimum );
                    o->minimum( p->hints.maximum );
                }

//                o->step( fabs( ( o->maximum() - o->minimum() ) ) / 32.0f );
                o->slider( FL_THIN_UP_BOX );
                o->color( fl_darker( fl_darker( FL_GRAY ) ) );
                o->selection_color( FL_WHITE );
                o->value( p->control_value() );
            }

        }

        Fl_Button *bound;

        w->align(FL_ALIGN_TOP);
        w->labelsize( 10 );
        w->callback( cb_value_handle, new callback_data( this, i ) );

        { Fl_Group *o = new Fl_Group( 0, 0, 50, 75 );
            {
                Fl_Labelpad_Group *flg = new Fl_Labelpad_Group( w );

                { Fl_Button *o = bound = new Fl_Button( 0, 50, 14, 14 );
                    o->selection_color( FL_YELLOW );
                    o->type( 0 );
                    o->labelsize( 8 );

                    o->value( p->connected() );

                    o->callback( cb_bound_handle, new callback_data( this, i ) );
                }

                o->resizable( 0 );
                o->end();

                o->set_visible_focus();
                flg->set_visible_focus();

                flg->position( o->x(), o->y() );
                bound->position( o->x(), flg->y() + flg->h() );
                o->size( flg->w(), flg->h() + bound->h() );
                o->init_sizes();
            }
            control_pack->add( o );
        }

    }

    int width = control_pack->max_width() + 100;
    int height = control_pack->h() + 50;

    main_pack->size( width, height );
    size( width, height );
}

void
Module_Parameter_Editor::cb_value_handle ( Fl_Widget *w, void *v )
{
    callback_data *cd = (callback_data*)v;

    cd->base_widget->set_value( cd->port_number, ((Fl_Valuator*)w)->value() );
}

void
Module_Parameter_Editor::cb_mode_handle ( Fl_Widget *, void *v )
{
    ((Module_Parameter_Editor*)v)->make_controls();
}

void
Module_Parameter_Editor::cb_bound_handle ( Fl_Widget *w, void *v )
{
    callback_data *cd = (callback_data*)v;

    Fl_Button *fv = (Fl_Button*)w;

    fv->value( 1 );

    cd->base_widget->bind_control( cd->port_number );
}

void
Module_Parameter_Editor::bind_control ( int i )
{
    Module::Port *p = &_module->control_input[i];

    if ( p->connected() )
        /* can only bind once */
        return;

    Controller_Module *o = new Controller_Module();
    o->label( p->name() );
    o->chain( _module->chain() );

    o->connect_to( p );

    _module->chain()->add_control( o );
    _module->redraw();
}

void
Module_Parameter_Editor::set_value (int i, float value )
{
    _module->control_input[i].control_value( value );
    _module->handle_control_changed( &_module->control_input[i] );
}
