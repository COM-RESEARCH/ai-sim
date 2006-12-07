#ifndef gui_schedule_list_h
#define gui_schedule_list_h

#include "gui_frame.h"
#include "gui_container.h"
#include "components/gui_chart.h"
#include "components/gui_textinput.h"
#include "components/gui_scrolled_list.h"
#include "components/gui_scrollpane.h"
#include "components/gui_tab_panel.h"
#include "gui_convoiinfo.h"
#include "../simline.h"

class spieler_t;
class karte_t;

/**
 * Window.
 * Will display list of schedules.
 * Contains buttons: edit new remove
 * Resizable.
 *
 * @author Niels Roest
 * @author hsiegeln: major redesign
 */
class schedule_list_gui_t : public gui_frame_t, public action_listener_t
{
private:
	karte_t *welt;
	spieler_t *sp;

	button_t bt_new_line, bt_change_line, bt_delete_line;
	gui_container_t cont, cont_haltestellen;
	gui_scrollpane_t scrolly, scrolly_haltestellen;
	gui_scrolled_list_t scl;
	gui_speedbar_t filled_bar;
	gui_textinput_t inp_name;
	gui_chart_t chart;
	button_t filterButtons[MAX_LINE_COST];
	gui_tab_panel_t tabs;

	static const char cost_type[MAX_LINE_COST][64];
	static const int cost_type_color[MAX_LINE_COST];
	static uint8 statistic[MAX_LINE_COST];
	static uint8 statistic_type[MAX_LINE_COST];

	int selection, capacity, load, loadfactor;

	void display(koord pos);

	void update_lineinfo(linehandle_t new_line);

	linehandle_t line;

	slist_tpl <linehandle_t> lines;

	void build_line_list(int filter);

public:
	schedule_list_gui_t(karte_t *welt,spieler_t *sp);

	/**
	* in top-level fenstern wird der Name in der Titelzeile dargestellt
	* @return den nicht uebersetzten Namen der Komponente
	* @author Hj. Malthaner
	*/
	const char* gib_name() const { return "Line Management"; }

	/**
	* Manche Fenster haben einen Hilfetext assoziiert.
	* @return den Dateinamen f�r die Hilfe, oder NULL
	* @author Hj. Malthaner
	*/
	const char* gib_hilfe_datei() const { return "linemanagement.txt"; }

	/**
	* Does this window need a min size button in the title bar?
	* @return true if such a button is needed
	* @author Hj. Malthaner
	*/
	bool has_min_sizer() const {return true;}

	/**
	* komponente neu zeichnen. Die �bergebenen Werte beziehen sich auf
	* das Fenster, d.h. es sind die Bildschirkoordinaten des Fensters
	* in dem die Komponente dargestellt wird.
	*
	* @author Hj. Malthaner
	*/
	void zeichnen(koord pos, koord gr);

	/**
	* resize window in response to a resize event
	* @author Hj. Malthaner
	*/
	void resize(const koord delta);

	/**
	* This method is called if an action is triggered
	* @author Hj. Malthaner
	*
	* Returns true, if action is done and no more
	* components should be triggered.
	* V.Meyer
	*/
	bool action_triggered(gui_komponente_t *komp, value_t extra);
};

#endif
