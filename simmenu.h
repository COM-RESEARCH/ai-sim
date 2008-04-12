/*
 * Copyright (c) 2008 prissi
 *
 * This file is part of the Simutrans project under the artistic licence.
 *
 * New configurable OOP tool system
 */

#ifndef simmenu_h
#define simmenu_h

#include "besch/sound_besch.h"

#include "dataobj/translator.h"
#include "dataobj/koord.h"
#include "dataobj/koord3d.h"

#include "tpl/vector_tpl.h"
#include "tpl/slist_tpl.h"

#include "simtypes.h"
#include "simworld.h"

class werkzeug_waehler_t;
class karte_t;
class spieler_t;
class toolbar_t;

enum {
	// general tools
	WKZ_ABFRAGE=0,
	WKZ_REMOVER,
	WKZ_RAISE_LAND,
	WKZ_LOWER_LAND,
	WKZ_SETSLOPE,
	WKZ_RESTORESLOPE,
	WKZ_MARKER,
	WKZ_CLEAR_RESERVATION,
	WKZ_TRANSFORMER,
	WKZ_ADD_CITY,
	WKZ_CHANGE_CITY_SIZE,
	WKZ_PLANT_TREE,
	WKZ_FAHRPLAN_ADD,
	WKZ_FAHRPLAN_INS,
	WKZ_WEGEBAU,
	WKZ_BRUECKENBAU,
	WKZ_TUNNELBAU,
	WKZ_WAYREMOVER,
	WKZ_WAYOBJ,
	WKZ_STATION,
	WKZ_ROADSIGN,
	WKZ_DEPOT,
	WKZ_BUILD_HAUS,
	WKZ_LAND_CHAIN,
	WKZ_CITY_CHAIN,
	WKZ_BUILD_FACTORY,
	WKZ_LINK_FACTORY,
	WKZ_HEADQUARTER,
	WKZ_LOCK_GAME,
	GENERAL_TOOL_COUNT,
	GENERAL_TOOL = 0x1000
};

enum {
	// simple one click tools
	WKZ_PAUSE = 0,
	WKZ_FASTFORWARD,
	WKZ_SCREENSHOT,
	WKZ_INCREASE_INDUSTRY,
	WKZ_UNDO,
	WKZ_SWITCH_PLAYER,
	WKZ_STEP_YEAR,
	WKZ_CHANGE_GAME_SPEED,
	WKZ_ZOOM_IN,
	WKZ_ZOOM_OUT,
	WKZ_SHOW_COVERAGE,
	WKZ_SHOW_NAMES,
	WKZ_SHOW_GRID,
	WKZ_SHOW_TREES,
	WKZ_SHOW_HOUSES,
	WKZ_SHOW_UNDERGROUND,
	WKZ_ROTATE90,
	WKZ_QUIT,
	SIMPLE_TOOL_COUNT,
	SIMPLE_TOOL = 0x2000
};

enum {
	// dialoge tools
	WKZ_HELP = 0,
	WKZ_OPTIONEN,
	WKZ_MINIMAP,
	WKZ_LINEOVERVIEW,
	WKZ_MESSAGES,
	WKZ_FINANCES,
	WKZ_PLAYERS,
	WKZ_DISPLAYOPTIONS,
	WKZ_SOUND,
	WKZ_LANGUAGE,
	WKZ_PLAYERCOLOR,
	WKZ_JUMP,
	WKZ_LOAD,
	WKZ_SAVE,
	WKZ_LIST_HALT,
	WKZ_LIST_CONVOI,
	WKZ_LIST_TOWN,
	WKZ_LIST_GOODS,
	WKZ_LIST_FACTORY,
	WKZ_LIST_CURIOSITY,
	WKZ_EDIT_FACTORY,
	WKZ_EDIT_ATTRACTION,
	WKZ_EDIT_HOUSE,
	WKZ_EDIT_TREE,
	DIALOGE_TOOL_COUNT,
	DIALOGE_TOOL = 0x4000
};

enum {
	// toolbars
	WKZ_MAINMENU = 0,
	TOOLBAR_TOOL = 0x8000u
};

class werkzeug_t {
public:
	// for sorting: compare tool key
	static bool compare_werkzeug( const werkzeug_t *a, const werkzeug_t *b) {
		uint16 ac = a->command_key & ~32;
		uint16 bc = b->command_key & ~32;
		return ac==bc ? a->command_key < b->command_key : ac < bc;
	}

	// for key loockup
	static vector_tpl<werkzeug_t *>char_to_tool;

	image_id cursor;
	image_id icon;
	int	ok_sound;
	int failed_sound;
	sint8 offset;
	const char *default_param;

	uint16	command_key;// key to toggle action for this function
	uint16	id;			// value to trigger this command (see documentation)

	static vector_tpl<werkzeug_t *> general_tool;
	static vector_tpl<werkzeug_t *> simple_tool;
	static vector_tpl<werkzeug_t *> dialog_tool;
	static vector_tpl<toolbar_t *> toolbar_tool;

	// since only a single toolstr a time can be visible ...
	static char toolstr[1024];

	static void init_menu(cstring_t objfilename);

	werkzeug_t() { id = 0xFFFFu; cursor = icon = IMG_LEER; ok_sound = failed_sound = NO_SOUND; offset = Z_PLAN; default_param = NULL; command_key = 0; }
	virtual ~werkzeug_t() {}

	virtual const char *get_tooltip(spieler_t *) { return NULL; }

	// returning false on init will automatically invoke previous tool
	virtual bool init( karte_t *, spieler_t * ) { return true; }
	virtual bool exit( karte_t *, spieler_t * ) { return true; }

	/* the return string can have different meanings:
	 * NULL: ok
	 * "": unspecified error
	 * "balbal": errors message, will be handled and translated as appropriate
	 */
	virtual const char *work( karte_t *, spieler_t *, koord3d ) { return NULL; }
	virtual const char *move( karte_t *, spieler_t *, uint16 /* buttonstate */, koord3d ) { return ""; }
};

/* toolbar are a new overclass */
class toolbar_t : public werkzeug_t {
public:
	// size of icons
	koord iconsize;
private:
	const char *helpfile;
	werkzeug_waehler_t *wzw;
	slist_tpl<werkzeug_t *>tools;
public:
	toolbar_t( const char *t, const char *h, koord size ) : werkzeug_t()
	{
		default_param = t;
		helpfile = h;
		wzw = NULL;
		iconsize = size;
	}
	const char *get_tooltip(spieler_t *) { return translator::translate(default_param); }
	werkzeug_waehler_t *get_werkzeug_waehler() const { return wzw; }
	// show this toolbar
	bool init(karte_t *, spieler_t *);
	void update(karte_t *, spieler_t *);	// just refresh content
	void append(werkzeug_t *w) { tools.append(w); }
};


#endif
