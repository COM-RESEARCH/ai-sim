/*
 * Copyright (c) 1997 - 2004 Hansj�rg Malthaner
 *
 * Line management
 *
 * This file is part of the Simutrans project and may not be used
 * in other projects without written permission of the author.
 */

#include <stdio.h>

#include "messagebox.h"

#include "factory_edit.h"

#include "../simcolor.h"
#include "../simtools.h"

#include "../simworld.h"
#include "../simevent.h"
#include "../simgraph.h"
#include "../simplay.h"
#include "../simwerkz.h"

#include "../simwin.h"

#include "../utils/simstring.h"

#include "../dataobj/translator.h"
#include "components/list_button.h"

#include "../bauer/fabrikbauer.h"

#include "../besch/grund_besch.h"
#include "../besch/intro_dates.h"

#include "../utils/cbuffer_t.h"


#define LINE_NAME_COLUMN_WIDTH (int)((BUTTON_WIDTH*2.25)+11)
#define SCL_HEIGHT (170)
#define N_BUTTON_WIDTH  (int)(BUTTON_WIDTH*1.5)

// Hajo: 17-Jan-04: changed layout to make components fit into
// a width of 400 pixels -> original size was unuseable in 640x480



factory_edit_frame_t::factory_edit_frame_t(spieler_t* sp_,karte_t* welt) :
	extend_edit_gui_t(sp_,welt),
	fablist(16),
	lb_production( prod_str, COL_WHITE, gui_label_t::right )
{
	bt_city_chain.init( button_t::square_state, "Only city chains", koord(NAME_COLUMN_WIDTH+11, 10+3*BUTTON_HEIGHT ) );
	bt_city_chain.add_listener(this);
	add_komponente(&bt_city_chain);

	bt_land_chain.init( button_t::square_state, "Only land chains", koord(NAME_COLUMN_WIDTH+11, 10+4*BUTTON_HEIGHT ) );
	bt_land_chain.add_listener(this);
	add_komponente(&bt_land_chain);

	bt_down_production.init( button_t::repeatarrowleft, NULL, koord(NAME_COLUMN_WIDTH/2-16,LINESPACE) );
	bt_down_production.add_listener(this);
	cont.add_komponente(&bt_down_production);

	bt_up_production.init( button_t::repeatarrowright, NULL, koord(NAME_COLUMN_WIDTH/2+50,LINESPACE) );
	bt_up_production.add_listener(this);
	cont.add_komponente(&bt_up_production);

	lb_production.setze_pos( koord( NAME_COLUMN_WIDTH/2+44, LINESPACE ) );
	cont.add_komponente(&lb_production);

	fill_list( is_show_trans_name );

	offset_of_comp = 10+5*BUTTON_HEIGHT+4;

	resize(koord(0,0));
}



// fill the current fablist
void factory_edit_frame_t::fill_list( bool translate )
{
	const bool allow_obsolete = bt_obsolete.pressed;
	const bool use_timeline = bt_timeline.pressed;
	const bool city_chain = bt_city_chain.pressed;
	const bool land_chain = bt_land_chain.pressed;
	const sint32 month_now = bt_timeline.pressed ? welt->get_current_month() : 0;

	fablist.clear();

	// timeline will be obeyed; however, we may show obsolete ones ...
	stringhashtable_iterator_tpl<const fabrik_besch_t *> iter(fabrikbauer_t::gib_fabesch());
	while(iter.next()) {

		const fabrik_besch_t *besch = iter.get_current_value();
		if(besch->gib_gewichtung()>0) {
			// DistributionWeight=0 is obsoluted item, only for backward compatibility

			if(!use_timeline  ||  (!besch->gib_haus()->is_future(month_now)  &&  (!besch->gib_haus()->is_retired(month_now)  ||  allow_obsolete))  ) {
				// timeline allows for this

				if(city_chain) {
					if(besch->gib_platzierung()==fabrik_besch_t::Stadt  &&  besch->gib_produkt(0)==NULL) {
						fablist.append(besch,16);
					}
				}
				if(land_chain) {
					if(besch->gib_platzierung()==fabrik_besch_t::Land  &&  besch->gib_produkt(0)==NULL) {
						fablist.append(besch,16);
					}
				}
				if(!city_chain  &&  !land_chain) {
					fablist.append(besch,16);
				}
			}
		}
	}
	// now buil scrolled list
	scl.clear_elements();
	scl.setze_selection(-1);
	for(  uint i=0;  i<fablist.get_count();  i++  ) {
		// color code for objects: BLACK: normal, YELLOW: consumer only, GREEN: source only
		uint8 color=COL_BLACK;
		if(fablist[i]->gib_produkt(0)==NULL) {
			color = COL_BLUE;
		}
		else if(fablist[i]->gib_lieferant(0)==NULL) {
			color = COL_DARK_GREEN;
		}
		// translate or not?
		if(  translate  ) {
			scl.append_element( translator::translate(fablist[i]->gib_name()), color );
		}
		else {
			scl.append_element( fablist[i]->gib_name(), color );
		}
		if(fablist[i]==bfs.besch) {
			scl.setze_selection(scl.get_count()-1);
		}
	}
	// always update curretn selection (since the tool may depend on it)
	change_item_info( scl.gib_selection() );
}



bool factory_edit_frame_t::action_triggered(gui_komponente_t *komp,value_t e)
{
	// only one chain can be shown
	if(  komp==&bt_city_chain  ) {
		bt_city_chain.pressed ^= 1;
		if(bt_city_chain.pressed) {
			bt_land_chain.pressed = 0;
		}
		fill_list( is_show_trans_name );
	}
	else if(  komp==&bt_land_chain  ) {
		bt_land_chain.pressed ^= 1;
		if(bt_land_chain.pressed) {
			bt_city_chain.pressed = 0;
		}
		fill_list( is_show_trans_name );
	}
	else if(bfs.besch) {
		if(  komp==&bt_up_production  ) {
			bfs.production += (bfs.production>=200) ? 10 : 1;
		}
		else if(  komp==&bt_down_production  &&  bfs.production>0) {
			bfs.production -= (bfs.production>200) ? 10 : 1;
		}
		sprintf( prod_str, "%i", bfs.production );
	}
	return extend_edit_gui_t::action_triggered(komp,e);
}



void factory_edit_frame_t::change_item_info(sint32 entry)
{
	for(int i=0;  i<4;  i++  ) {
		img[i].set_image( IMG_LEER );
	}
	buf.clear();
	prod_str[0] = 0;

	if(entry>=0  &&  entry<(sint32)fablist.get_count()) {

		const fabrik_besch_t *fab_besch = fablist[entry];

		// production
		buf.append( translator::translate("Produktion") );
		buf.append("\n");
		buf.append("\n");
		bfs.production = (fab_besch->gib_produktivitaet()+simrand(fab_besch->gib_bereich()) )<<(welt->ticks_bits_per_tag-18);
		sprintf( prod_str, "%i", bfs.production );

		if(fab_besch->gib_produkte()>0) {
			buf.append( translator::translate("Produktion") );
			buf.append("\n");
			for(  int i=0;  i<fab_besch->gib_produkte();  i++  ) {
				buf.append( translator::translate(fab_besch->gib_produkt(i)->gib_ware()->gib_name()) );
				buf.append( " (" );
				buf.append( translator::translate(fab_besch->gib_produkt(i)->gib_ware()->gib_catg_name()) );
				buf.append( ")\n" );
			}
			buf.append("\n");
		}

		if(fab_besch->gib_lieferanten()>0) {
			buf.append( translator::translate("Verbrauch") );
			buf.append("\n");
			for(  int i=0;  i<fab_besch->gib_lieferanten();  i++  ) {
				buf.append( translator::translate(fab_besch->gib_lieferant(i)->gib_ware()->gib_name()) );
				buf.append( " (" );
				buf.append( translator::translate(fab_besch->gib_lieferant(i)->gib_ware()->gib_catg_name()) );
				buf.append( ")\n" );
			}
			buf.append("\n");
		}

		// now the house stuff
		const haus_besch_t *besch = fab_besch->gib_haus();

		// climates
		buf.append( translator::translate("allowed climates:\n") );
		uint16 cl = besch->get_allowed_climate_bits();
		if(cl==0) {
			buf.append( translator::translate("none") );
			buf.append("\n");
		}
		else {
			for(uint16 i=0;  i<=arctic_climate;  i++  ) {
				if(cl &  (1<<i)) {
					buf.append( translator::translate( grund_besch_t::get_climate_name_from_bit( (enum climate)i ) ) );
					buf.append("\n");
				}
			}
		}
		buf.append("\n");

		buf.append(translator::translate("Passagierrate"));
		buf.append(": ");
		buf.append(fablist[entry]->gib_pax_level());
		buf.append("\n");

		buf.append(translator::translate("Postrate"));
		buf.append(": ");
		buf.append(besch->gib_post_level());
		buf.append("\n");

		buf.append(translator::translate("\nBauzeit von"));
		buf.append(besch->get_intro_year_month()/12);
		if(besch->get_retire_year_month()!=DEFAULT_RETIRE_DATE*12) {
			buf.append(translator::translate("\nBauzeit bis"));
			buf.append(besch->get_retire_year_month()/12);
		}

		const char *maker=besch->gib_copyright();
		if(maker!=NULL  && maker[0]!=0) {
			buf.append("\n");
			buf.printf(translator::translate("Constructed by %s"), maker);
		}
		buf.append("\n");
		info_text.recalc_size();
		cont.setze_groesse( info_text.gib_groesse() );

		// now the images (maximum is 2x2 size)
		uint8 rot = 0; //to be changed
		if(besch->gib_b(rot)==1) {
			if(besch->gib_h(rot)==1) {
				img[3].set_image( besch->gib_tile(0)->gib_hintergrund(0,0,0) );
			}
			else {
				img[2].set_image( besch->gib_tile(rot,0,0)->gib_hintergrund(0,0,0) );
				img[3].set_image( besch->gib_tile(rot,0,1)->gib_hintergrund(0,0,0) );
			}
		}
		else {
			if(besch->gib_h(rot)==1) {
				img[1].set_image( besch->gib_tile(rot,0,0)->gib_hintergrund(0,0,0) );
				img[3].set_image( besch->gib_tile(rot,1,0)->gib_hintergrund(0,0,0) );
			}
			else {
				// maximum 2x2 image
				for(int i=0;  i<4;  i++  ) {
					img[i].set_image( besch->gib_tile(rot,i/2,i&1)->gib_hintergrund(0,0,0) );
				}
			}
		}

		// now for the tool
		bfs.besch = fablist[entry];
		bfs.ignore_climates = bt_climates.pressed;
		bfs.rotation = 255; // no definition yet
		if(bt_land_chain.pressed) {
			welt->setze_maus_funktion( wkz_build_industries_land, skinverwaltung_t::undoc_zeiger->gib_bild_nr(0), welt->Z_PLAN, (value_t)&bfs,  SFX_JACKHAMMER, SFX_FAILURE );
		}
		else if(bt_city_chain.pressed) {
			welt->setze_maus_funktion( wkz_build_industries_city, skinverwaltung_t::undoc_zeiger->gib_bild_nr(0), welt->Z_PLAN, (value_t)&bfs,  SFX_JACKHAMMER, SFX_FAILURE );
		}
		else {
			welt->setze_maus_funktion( wkz_build_fab, skinverwaltung_t::undoc_zeiger->gib_bild_nr(0), welt->Z_PLAN, (value_t)&bfs,  SFX_JACKHAMMER, SFX_FAILURE );
		}
	}
	else if(bfs.besch!=NULL) {
		bfs.besch = NULL;
		welt->setze_maus_funktion( wkz_abfrage, skinverwaltung_t::fragezeiger->gib_bild_nr(0), welt->Z_PLAN,  NO_SOUND, NO_SOUND );
	}
}


