#include "../simdebug.h"

#include "../simimg.h"
#include "../simworld.h"
#include "../simwin.h"
#include "../simhalt.h"

#include "../besch/grund_besch.h"

#include "../dataobj/loadsave.h"
#include "../gui/ground_info.h"
#include "../tpl/ptrhashtable_tpl.h"

#include "brueckenboden.h"

brueckenboden_t::brueckenboden_t(karte_t *welt, loadsave_t *file) : grund_t(welt)
{
	rdwr(file);
}


brueckenboden_t::brueckenboden_t(karte_t *welt, koord3d pos, int grund_hang, int weg_hang) : grund_t(welt, pos)
{
	slope = grund_hang;
	this->weg_hang = weg_hang;
}


void brueckenboden_t::calc_bild_internal()
{
	if(ist_tunnel()) {
		clear_back_bild();
		setze_bild(IMG_LEER);
	}
	else {
		if(ist_karten_boden()) {
			setze_bild( grund_besch_t::gib_ground_tile(slope,gib_pos().z) );
			grund_t::calc_back_bild(gib_pos().z/Z_TILE_STEP,slope);
			set_flag(draw_as_ding);
			koord pos = gib_pos().gib_2d()+koord(gib_grund_hang());
			grund_t *gr = welt->lookup_kartenboden(pos);
			if(gr) {
				gr->calc_bild();
			}
		}
		else {
			clear_back_bild();
			setze_bild( IMG_LEER );
		}
	}
}


void
brueckenboden_t::rdwr(loadsave_t *file)
{
	grund_t::rdwr(file);

	if(file->get_version()<88009) {
		uint8 sl;
		file->rdwr_byte( sl, " " );
		slope = sl;
	}
	file->rdwr_byte(weg_hang, "\n");
}


void brueckenboden_t::rotate90()
{
	weg_hang = hang_t::rotate90( weg_hang );
	grund_t::rotate90();
}


sint8 brueckenboden_t::gib_weg_yoff() const
{
	if(ist_karten_boden() && weg_hang == 0) {
		return TILE_HEIGHT_STEP;
	}
	else {
		return 0;
	}
}


bool
brueckenboden_t::zeige_info()
{
	if(gib_halt().is_bound()) {
		gib_halt()->zeige_info();
		return true;
	}
	else {
		create_win(new grund_info_t(this), w_info, (long)this);
		return true;
	}
	return false;
}
