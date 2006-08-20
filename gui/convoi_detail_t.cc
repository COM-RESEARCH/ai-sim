/*
 * convoi_detail_t.cc
 *
 * Copyright (c) 1997 - 2001 Hansj�rg Malthaner
 *
 * This file is part of the Simutrans project and may not be used
 * in other projects without written permission of the author.
 */


#include <stdio.h>

#include "convoi_detail_t.h"

#include "../simplay.h"
#include "../simtime.h"
#include "../simconvoi.h"
#include "../simdepot.h"
#include "../simvehikel.h"
#include "../simcolor.h"
#include "../simgraph.h"
#include "../simworld.h"
#include "../dataobj/fahrplan.h"
#include "../dataobj/translator.h"
#include "fahrplan_gui.h"
// @author hsiegeln
#include "../simlinemgmt.h"
#include "../simline.h"
#include "../boden/grund.h"
#include "messagebox.h"

#include "../utils/simstring.h"

#include "components/gui_chart.h"
#include "components/list_button.h"


/**
 * Konstruktor.
 * @author Hj. Malthaner
 */
convoi_detail_t::convoi_detail_t(convoihandle_t cnv)
: gui_frame_t(cnv->gib_name(), cnv->gib_besitzer()->kennfarbe),
  scrolly(&veh_info),
  veh_info(cnv)
{
	this->cnv = cnv;

	scrolly.setze_pos(koord(0, 64));
	add_komponente(&scrolly);

	setze_opaque(true);
	setze_fenstergroesse(koord(TOTAL_WIDTH, 278));

	set_min_windowsize(koord(TOTAL_WIDTH, 194));
	set_resizemode(diagonal_resize);
	resize(koord(0,0));
}


/**
 * Destruktor.
 * @author Hj. Malthaner
 */
convoi_detail_t::~convoi_detail_t() { }



static koord groesse_vehicle(300,5);


/**
 * komponente neu zeichnen. Die �bergebenen Werte beziehen sich auf
 * das Fenster, d.h. es sind die Bildschirkoordinaten des Fensters
 * in dem die Komponente dargestellt wird.
 * @author Hj. Malthaner
 */
void
convoi_detail_t::zeichnen(koord pos, koord gr)
{
	if(cnv.is_bound()) {
		// all gui stuff set => display it
		gui_frame_t::zeichnen(pos, gr);
		veh_info.setze_groesse(groesse_vehicle);
		scrolly.setze_groesse(get_client_windowsize()-scrolly.gib_pos());

#if 0
		// convoi information
		static char tmp[256];
		static cbuffer_t info_buf(256);

		// use median speed to avoid flickering
		mean_convoi_speed += speed_to_kmh(cnv->gib_akt_speed()*4);
		mean_convoi_speed /= 2;
		sprintf(tmp,translator::translate("%i km/h (max. %ikm/h)"), (mean_convoi_speed+3)/4, speed_to_kmh(cnv->gib_min_top_speed()) );
		display_proportional( pos.x+11, pos.y+16+20, tmp, ALIGN_LEFT, SCHWARZ, true );

		// next important: income stuff
		info_buf.clear();
		info_buf.append( translator::translate("Gewinn") );
		int len = 5+display_proportional( pos.x+11, pos.y+16+20+1*LINESPACE, info_buf, ALIGN_LEFT, SCHWARZ, true );
		money_to_string( tmp, cnv->gib_jahresgewinn()/100.0 );
		len += 5+display_proportional( pos.x+11+len, pos.y+16+20+1*LINESPACE, tmp, ALIGN_LEFT, cnv->gib_jahresgewinn()>0?MONEY_PLUS:MONEY_MINUS, true );
		sprintf(tmp," (%1.2f$/km)", cnv->get_running_cost()/100.0 );
		display_proportional( pos.x+11+len, pos.y+16+20+1*LINESPACE, tmp, ALIGN_LEFT, SCHWARZ, true );

		// the weight entry
		info_buf.clear();
		info_buf.append( translator::translate("Gewicht") );
		info_buf.append( ": " );
		info_buf.append( cnv->gib_sum_gesamtgewicht() );
		info_buf.append( " (" );
		info_buf.append( cnv->gib_sum_gesamtgewicht()-cnv->gib_sum_gewicht() );
		info_buf.append( ") t" );
		display_proportional( pos.x+11, pos.y+16+20+2*LINESPACE, info_buf, ALIGN_LEFT, SCHWARZ, true );

		// next stop
		const fahrplan_t * fpl = cnv->gib_fahrplan();
		info_buf.clear();
		info_buf.append(translator::translate("Fahrtziel:"));
		info_buf.append(" ");
		fahrplan_gui_t::gimme_short_stop_name(info_buf, cnv->gib_welt(), fpl, fpl->aktuell, 34);
		len = display_proportional( pos.x+11, pos.y+16+20+3*LINESPACE, info_buf, ALIGN_LEFT, SCHWARZ, true );

		// convoi load indicator
		const int offset = max( len+11, 167)+3;
		route_bar.setze_pos(koord(offset,22+3*LINESPACE));
		route_bar.setze_groesse(koord(view.gib_pos().x-offset-5, 4));

		/*
		* only show assigned line, if there is one!
		* @author hsiegeln
		*/
		if (cnv->get_line() != NULL) {
			info_buf.clear();
			info_buf.append( translator::translate("Serves Line:") );
			info_buf.append( " " );
			info_buf.append( cnv->get_line()->get_name() );
			display_proportional( pos.x+11, pos.y+16+20+4*LINESPACE, info_buf, ALIGN_LEFT, SCHWARZ, true );
		}
#endif
	}
}



/**
 * Resize the contents of the window
 * @author Markus Weber
 */
void convoi_detail_t::resize(const koord delta)
{
	gui_frame_t::resize(delta);
	scrolly.setze_groesse(get_client_windowsize()-scrolly.gib_pos());
}



gui_vehicleinfo_t::gui_vehicleinfo_t(convoihandle_t cnv)
{
	this->cnv = cnv;
}


/**
 * Events werden hiermit an die GUI-Komponenten
 * gemeldet
 * @author Hj. Malthaner
 */
void gui_vehicleinfo_t::infowin_event(const event_t *ev)
{
	if(IS_LEFTRELEASE(ev) && cnv.is_bound()) {
		// if there are further details, open dialog
	}
}



/**
 * Zeichnet die Komponente
 * @author Hj. Malthaner
 */
void gui_vehicleinfo_t::zeichnen(koord offset) const
{
	int total_height=5;
	if(cnv.is_bound()) {
		char buf[256], tmp[256];

		// for bonus stuff
		const sint32 ref_speed = cnv->gib_welt()->get_average_speed( cnv->gib_vehikel(0)->gib_wegtyp() );
		const sint32 speed_base = (100*speed_to_kmh(cnv->gib_min_top_speed()))/ref_speed-100;

		static cbuffer_t freight_info(1024);
		for(unsigned veh=0;  veh<cnv->gib_vehikel_anzahl(); veh++ ) {
			vehikel_t *v=cnv->gib_vehikel(veh);
			int returns = 0;
			freight_info.clear();

			// first image
			int x, y, w, h;
			const image_id bild=v->gib_basis_bild();
			display_get_image_offset(bild, &x, &y, &w, &h );
			display_color_img(bild,11-x+pos.x+offset.x,pos.y+offset.y+total_height-y,cnv->gib_besitzer()->kennfarbe,false,true);
			w = max(40,w+4)+11;

			// now add the other info
			int extra_y=0;

			// age
			sint32 month = v->gib_insta_zeit();
			sprintf( buf, "%s %s %i", translator::translate("Manufactured:"), translator::translate(month_names[month%12]), month/12  );
			display_proportional_clip( pos.x+w+offset.x, pos.y+offset.y+total_height+extra_y, buf, ALIGN_LEFT, BLACK, true );
			extra_y += LINESPACE;

			// value
			sint32 current = v->calc_restwert();
			money_to_string( tmp, current/100.0 );
			sprintf( buf, "%s %s", translator::translate("Restwert:"), tmp );
			display_proportional_clip( pos.x+w+offset.x, pos.y+offset.y+total_height+extra_y, buf, ALIGN_LEFT, MONEY_PLUS, true );
			extra_y += LINESPACE;

			// power
			if(v->gib_besch()->gib_leistung()>0) {
				sprintf( buf, "%s %i %s %f", translator::translate("Power:"), v->gib_besch()->gib_leistung(), translator::translate("Gear:"), v->gib_besch()->get_gear()/100.0 );
				display_proportional_clip( pos.x+w+offset.x, pos.y+offset.y+total_height+extra_y, buf, ALIGN_LEFT, MONEY_PLUS, true );
				extra_y += LINESPACE;
			}

			if(v->gib_fracht_max() > 0) {

				// bonus stuff
				int len = 5+display_proportional_clip( pos.x+w+offset.x, pos.y+offset.y+total_height+extra_y, translator::translate("Max income:"), ALIGN_LEFT, SCHWARZ, true );
				const sint32 grundwert128 = v->gib_fracht_typ()->gib_preis()<<7;
				const sint32 grundwert_bonus = v->gib_fracht_typ()->gib_preis()*(1000l+speed_base*v->gib_fracht_typ()->gib_speed_bonus());
				const sint32 price = (v->gib_fracht_max()*(grundwert128>grundwert_bonus ? grundwert128 : grundwert_bonus))/30 - v->gib_betriebskosten();
				money_to_string( tmp, price/100.0 );
				display_proportional_clip( pos.x+w+offset.x+len, pos.y+offset.y+total_height+extra_y, tmp, ALIGN_LEFT, price>0?MONEY_PLUS:MONEY_MINUS, true );
				extra_y += LINESPACE;

				freight_info.append(v->gib_fracht_menge());
				freight_info.append("/");
				freight_info.append(v->gib_fracht_max());
				freight_info.append(translator::translate(v->gib_fracht_mass()));
				freight_info.append(" ");
				freight_info.append(v->gib_fracht_typ()->gib_catg() == 0 ? translator::translate(v->gib_fracht_typ()->gib_name()) : translator::translate(v->gib_fracht_typ()->gib_catg_name()));
				freight_info.append("\n");
				v->gib_fracht_info(freight_info);
				// show it
				display_multiline_text( pos.x+offset.x+w, pos.y+offset.y+total_height+extra_y, freight_info, SCHWARZ );
				// count returns
				const char *p=freight_info;
				for(int i=0; i<freight_info.len(); i++ ) {
					if(p[i]=='\n') {
						returns ++;
					}
				}
				extra_y += returns*LINESPACE;
			}
			//skip at least five lines
			total_height += max(extra_y,4*LINESPACE)+5;
		}
	}
	// since the methods is const, we have to use dirty tricks ...
	groesse_vehicle = koord(300, total_height+1);
}
