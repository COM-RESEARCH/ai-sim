/*
 * Copyright (c) 1997 - 2001 Hansj�rg Malthaner
 *
 * This file is part of the Simutrans project under the artistic licence.
 * (see licence.txt)
 */

#ifndef boden_boden_h
#define boden_boden_h

#include "grund.h"

/**
 * Der Boden ist der 'Natur'-Untergrund in Simu. Er kann einen Besitzer haben.
 *
 * @author Hj. Malthaner
 */

class boden_t : public grund_t
{
protected:
	static bool show_grid;

	virtual void calc_bild_internal();

public:
	boden_t(karte_t *welt, loadsave_t *file, koord pos ) : grund_t( welt, koord3d(pos,0) ) { rdwr(file); }
	boden_t(karte_t *welt, koord3d pos, hang_t::typ slope);

	inline bool ist_natur() const { return !hat_wege(); }

	const char *gib_name() const;

	grund_t::typ gib_typ() const {return boden;}
};

#endif
