/*
 * simroutemgmt.h
 * part of the Simutrans project
 * @author hsiegeln
 * 01/12/2003
 */

#ifndef simlinemgmt_h
#define simlinemgmt_h

#include <string.h>

#include "simtypes.h"
#include "simline.h"
#include "dataobj/loadsave.h"
#include "dataobj/translator.h"

#include "tpl/slist_tpl.h"
#include "simconvoi.h"

#include "simdebug.h"

class fahrplan_t;

#define UNVALID_LINE_ID ((uint16)(-1))

class simlinemgmt_t
{
 public:

 	simlinemgmt_t(karte_t * welt, spieler_t *besitzer);
	void destroy_all();

	/*
	 * add a line
	 * @author hsiegeln
	 */
 	void add_line(simline_t * new_line);

	/*
	 * delete a line
	 * @author hsiegeln
	 */
 	void delete_line(int iroute);
 	void delete_line(simline_t * line);

	/*
	 * update a line -> apply updated fahrplan to all convoys
	 * @author hsiegeln
	 */
 	void update_line(int iroute);
	void update_line(simline_t * line);

	/*
	 * return number off lines
	 * @author hsiegeln
	 */
 	int count_lines() { return all_managed_lines.get_count(); }

	/*
	* return a line
	* @author hsiegeln
	*/
	simline_t * get_line(uint16 i) const {return i<all_managed_lines.get_count() ? all_managed_lines.at(i) : NULL; }
	simline_t * get_line(fahrplan_t *fpl);

	/*
	* return a line by its ID
	* @author hsiegeln
	*/
	simline_t * get_line_by_id(uint16 line);

 	/*
 	 * load or save the linemanagement
 	 */
 	void rdwr(karte_t * welt, loadsave_t * file);

 	/*
 	 * sort the lines by name
 	 */
	void sort_lines();

 	/*
 	 * will register all stops for all lines
 	 */
	void register_all_stops();

	/*
	 * called after game is fully loaded;
	 */
	void laden_abschliessen();

	/**
	 * Creates a unique line id.
	 * @author Hj. Malthaner
	 */
	static uint16 get_unique_line_id();
	static void init_line_ids();

	void new_month();

	/**
	 * @author hsiegeln
	 */
	simline_t * create_line(int ltype);
	simline_t * create_line(int ltype, fahrplan_t * fpl);

	/**
	 * @author hsiegeln
	 */
	 void build_line_list(int type, slist_tpl<simline_t *> * list);

	/*
	 * @author prissi
	 */
	spieler_t *gib_besitzer() const { return besitzer; }

 private:
	static uint8 used_ids[8192];

	vector_tpl<simline_t *> all_managed_lines;

	static int compare_lines(const void *p1, const void *p2);

	karte_t * welt;
	spieler_t *besitzer;
};

#endif
