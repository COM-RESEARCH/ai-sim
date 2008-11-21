/*
 * Copyright (c) 1997 - 2001 Hansj�rg Malthaner
 *
 * This file is part of the Simutrans project under the artistic licence.
 * (see licence.txt)
 */

#include <string.h>

#include "../../simdebug.h"
#include "gui_combobox.h"
#include "../../simevent.h"
#include "../../simgraph.h"
#include "../../simcolor.h"
#include "../../simwin.h"
#include "../../utils/simstring.h"


gui_combobox_t::gui_combobox_t() :
	droplist(gui_scrolled_list_t::select)
{
//	textinp.add_listener(this);

	bt_prev.setze_typ(button_t::arrowleft);
	bt_prev.setze_pos( koord(0,2) );
	bt_prev.setze_groesse( koord(10,10) );

	bt_next.setze_typ(button_t::arrowright);
	bt_next.setze_groesse( koord(10,10) );

	editstr[0] = 0;

	first_call = true;
	finish = false;
	droplist.set_visible(false);
	droplist.add_listener(this);
	setze_groesse(gib_groesse());
	max_size = koord(0,100);
	set_highlight_color(0);
	set_read_only(false);
}


gui_combobox_t::~gui_combobox_t()
{
	release_focus(this);
}



/**
 * Events werden hiermit an die GUI-Komponenten
 * gemeldet
 * @author Hj. Malthaner
 */
void gui_combobox_t::infowin_event(const event_t *ev)
{
	textinp.infowin_event(ev);

	if (!droplist.is_visible()) {
DBG_MESSAGE("event","%d,%d",ev->cx, ev->cy);
		if(bt_prev.getroffen(ev->cx, ev->cy)) {
DBG_MESSAGE("event","HOWDY!");
			bt_prev.pressed = IS_LEFT_BUTTON_PRESSED(ev);
			if(IS_LEFTRELEASE(ev)) {
				bt_prev.pressed = false;
				if (droplist.gib_selection() >= -1) {
					value_t p;
					p.i = droplist.gib_selection() - 1;
					set_selection( p.i );
					call_listeners(p);
				}
			}
			return;
		}
		else if(bt_next.getroffen(ev->cx, ev->cy)) {
			bt_next.pressed = IS_LEFT_BUTTON_PRESSED(ev);
			if(IS_LEFTRELEASE(ev)) {
				bt_next.pressed = false;
				if (droplist.gib_selection() < droplist.get_count() - 1) {
					value_t p;
					p.i = droplist.gib_selection() + 1;
					set_selection( p.i );
					call_listeners(p);
				}
			}
			return;
		}
	}
	else if(  IS_WHEELUP(ev)  ||  IS_WHEELDOWN(ev)  ) {
		// scroll the list
		droplist.infowin_event(ev);
		return;
	}

	if(IS_LEFTCLICK(ev) || IS_LEFTDRAG(ev) || IS_LEFTRELEASE(ev)  ) {

		if(first_call) {
			// prepare for selection

			// swallow the first mouseclick
			if(IS_LEFTRELEASE(ev)) {
				first_call = false;
			}

			droplist.set_visible(true);
			droplist.setze_groesse(koord(this->groesse.x, max_size.y - 16));
			droplist.setze_pos(koord(this->pos.x, this->pos.y + 16));
			droplist.request_groesse(droplist.gib_groesse());
			setze_groesse(droplist.gib_groesse() + koord(0, 16));
			droplist.show_selection(droplist.gib_selection());
		}
		else {
			if (droplist.is_visible()) {
				event_t ev2 = *ev;
				translate_event(&ev2, 0, -16);

				if(droplist.getroffen(ev->cx + pos.x, ev->cy + pos.y)  ||  IS_WHEELUP(ev)  ||  IS_WHEELDOWN(ev)) {
					droplist.infowin_event(&ev2);
					// we selected something?
					if(finish  && IS_LEFTRELEASE(ev)) {
						close_box();
					}
				}
				else {
					// acting on "release" is better than checking for "new selection"
					if (IS_LEFTRELEASE(ev)) {
DBG_MESSAGE("gui_combobox_t::infowin_event()","close");
						close_box();
					}
				}
			}
		}
	} else if(ev->ev_class==INFOWIN  &&  ev->ev_code==WIN_CLOSE) {
DBG_MESSAGE("gui_combobox_t::infowin_event()","close");
		close_box();
	} else if (ev->ev_class == EVENT_KEYBOARD) {
		if(ev->ev_code==13) {
			//return key
			droplist.set_visible(false);
			close_box();
			// update "mouse-click-catch-area"
			setze_groesse(koord(groesse.x, droplist.is_visible() ? max_size.y : 14));
		}
	}
	// update "mouse-click-catch-area"
//	setze_groesse(koord(groesse.x, droplist.is_visible() ? max_size.y : 14));
}



/* selction now handled via callback */
bool gui_combobox_t::action_triggered(gui_komponente_t *komp,value_t p)
{
	if (komp == &droplist) {
DBG_MESSAGE("gui_combobox_t::infowin_event()","scroll selected %i",p.i);
		finish = true;
		// check string of old item
		gui_scrolled_list_t::scrollitem_t *item = droplist.get_element(p.i);
		if(  item  &&  item->is_valid()  ) {
			tstrncpy( editstr, item->get_text(), 128 );
			textinp.setze_text( editstr, 128);
		}
	}
	return false;
}



/**
 * Zeichnet die Komponente
 * @author Hj. Malthaner
 */
void gui_combobox_t::zeichnen(koord offset)
{
	textinp.zeichnen(offset);

	if (droplist.is_visible()) {
		droplist.zeichnen(offset);
	}
	else {
		offset += pos;
		bt_prev.zeichnen(offset);
		bt_next.zeichnen(offset);
	}
	// text changed? Then update it
	gui_scrolled_list_t::scrollitem_t *item = droplist.get_element( droplist.gib_selection() );
	if(  item  &&  item->is_valid()  &&  strncmp(item->get_text(),editstr,127)!=0  ) {
		item->set_text( editstr );
		tstrncpy( editstr, item->get_text(), 128 );
	}
}



/**
 * sets the selection
 * @author hsiegeln
 */
void
gui_combobox_t::set_selection(int s)
{
	if (droplist.is_visible()) {
		// visible? change also offset of scrollbar
		droplist.show_selection( s );
	}
	else {
		// just set it
		droplist.setze_selection(s);
	}
	// edit the text
	gui_scrolled_list_t::scrollitem_t *item = droplist.get_element(s);
	if(  item  &&  item->is_valid()  &&  strncmp(editstr,item->get_text(),127)!=0  ) {
		tstrncpy( editstr, item->get_text(), 128 );
		textinp.setze_text( editstr, 128);
	}
}



/**
* Release the focus if we had it
*/
void
gui_combobox_t::close_box()
{
	if(finish) {
//DBG_MESSAGE("gui_combobox_t::infowin_event()","prepare selected %i for %d listerners",get_selection(),listeners.count());
		value_t p;
		p.i = droplist.gib_selection();
		call_listeners(p);
		finish = false;
	}
	release_focus(this);
	release_focus(&textinp);
	droplist.set_visible(false);
	setze_groesse(koord(groesse.x, 14));
	first_call = true;
};



/**
* Release the focus if we had it
*/
void gui_combobox_t::setze_groesse(koord gr)
{
	textinp.setze_pos( pos+koord(12,0) );
	textinp.setze_groesse( koord(gr.x-26,14) );
	bt_next.setze_pos( koord(gr.x-12,2) );
	gui_komponente_t::groesse = gr;
}
