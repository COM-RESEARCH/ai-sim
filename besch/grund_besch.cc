/*
 *
 *  grund_besch.cpp
 *
 *  Copyright (c) 1997 - 2002 by Volker Meyer & Hansj�rg Malthaner
 *
 *  This file is part of the Simutrans project and may not be used in other
 *  projects without written permission of the authors.
 *
 *  Modulbeschreibung:
 *      ...
 *
 */


#include <stdio.h>

#include "../simdebug.h"
#include "spezial_obj_tpl.h"
#include "grund_besch.h"

/* may change due to summer winter changes *
 * @author prissi
 */

#ifdef DOUBLE_GROUNDS
const uint8 grund_besch_t::slopetable[80] =
{
	0, 1 , 0xFF,
	2 , 3 , 0xFF, 0xFF, 0xFF, 0xFF,
	4 , 5, 0xFF, 6, 7, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	8, 9, 0xFF, 10, 11, 0xFF, 0xFF, 0xFF, 0xFF, 12, 13, 0xFF,  14, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};
#endif

const grund_besch_t *grund_besch_t::boden = NULL;
const grund_besch_t *grund_besch_t::ufer = NULL;

/*
 *  static data
 */

const grund_besch_t *grund_besch_t::standard_boden = NULL;
const grund_besch_t *grund_besch_t::standard_ufer = NULL;
const grund_besch_t *grund_besch_t::winter_boden = NULL;
const grund_besch_t *grund_besch_t::winter_ufer = NULL;
const grund_besch_t *grund_besch_t::wasser = NULL;
const grund_besch_t *grund_besch_t::fundament = NULL;
const grund_besch_t *grund_besch_t::ausserhalb = NULL;

static spezial_obj_tpl<grund_besch_t> spezial_objekte[] = {
    { &grund_besch_t::standard_boden,	    "Gras" },
    { &grund_besch_t::standard_ufer,	    "Shore" },
    { &grund_besch_t::wasser,	    "Water" },
    { &grund_besch_t::fundament,    "Basement" },
    { &grund_besch_t::ausserhalb,   "Outside" },
    { NULL, NULL }
};

static spezial_obj_tpl<grund_besch_t> winter_spezial_objekte[] = {
    { &grund_besch_t::winter_boden,	    "WinterGras" },
    { &grund_besch_t::winter_ufer,	    "WinterShore" },
    { NULL, NULL }
};

/*
 *  member function:
 *      grund_besch_t::register_besch()
 *
 *  Autor:
 *      Volker Meyer
 *
 *  Beschreibung:
 *      Ein weiteres geladenes Objekt rgistrieren.
 *
 *  Return type:
 *      bool
 *
 *  Argumente:
 *      const grund_besch_t *besch
 */
bool grund_besch_t::register_besch(const grund_besch_t *besch)
{
	if(!::register_besch(spezial_objekte, besch)) {
		return ::register_besch(winter_spezial_objekte, besch);
	}
	return true;
}

/*
 *  member function:
 *      grund_besch_t::alles_geladen()
 *
 *  Autor:
 *      Volker Meyer
 *
 *  Beschreibung:
 *      Alle zwingend erforderlich
 *
 *  Return type:
 *      bool
 */
bool grund_besch_t::alles_geladen()
{
	// standard is "summer"
	for(int i=0; i<78;  i++  ) {
		DBG_MESSAGE("boden","%i=%p",i,static_cast<const bildliste2d_besch_t *>(grund_besch_t::standard_boden->gib_kind(2))->gib_liste(i) );
	}
	grund_besch_t::boden = grund_besch_t::standard_boden;
	grund_besch_t::ufer = grund_besch_t::standard_ufer;
	return ::alles_geladen(spezial_objekte);
}
