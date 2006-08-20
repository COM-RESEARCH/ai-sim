/*
 * route.cc
 *
 * Copyright (c) 1997 - 2001 Hansj�rg Malthaner
 *
 * This file is part of the Simutrans project and may not be used
 * in other projects without written permission of the author.
 */


#include <stdio.h>
#include <string.h>

#include "../simdebug.h"
#include "../simworld.h"
#include "../simintr.h"
#include "../simtime.h"
#include "../simmem.h"
#include "../simhalt.h"
#include "../boden/wege/weg.h"
#include "../boden/grund.h"
#include "../ifc/fahrer.h"
// history, since no longer support for drivables
//#include "../ifc/route_block_tester_t.h"
static const void * block_tester=NULL;

#include "../tpl/prioqueue_tpl.h"

#include "loadsave.h"
#include "route.h"
#include "umgebung.h"

#include "../tpl/vector_tpl.h"
#include "../tpl/array_tpl.h"


// dies wird fuer die routenplanung benoetigt

class KNode {
private:

public:
    KNode * link;
    uint16  dist;
    uint32  total;
    koord3d pos;

    inline bool operator < (const KNode &k) const {
  return dist+total <= k.dist+k.total;
    };
};

route_t::route_t() : route(0)
{
}

void
route_t::kopiere(const route_t *r)
{
	assert(r != NULL);
	const unsigned int hops = r->gib_max_n();
	route.resize(hops+1);
	route.clear();
	for( unsigned int i=0;  i<=hops;  i++ ) {
		route.append(r->route.at(i));
	}
}

void
route_t::append(const route_t *r)
{
	assert(r != NULL);
	const unsigned int hops = r->gib_max_n();
	route.resize(hops+1+route.get_count());

DBG_MESSAGE("route_t::append()","last=%i,%i,%i   first=%i,%i,%i",
	route.at(gib_max_n()).x, route.at(gib_max_n()).y, route.at(gib_max_n()).z,
	r->position_bei(0).x, r->position_bei(0).y, r->position_bei(0).z );

	// must be identical
//	assert(gib_max_n()<=0  ||  r->position_bei(0)==route.at(gib_max_n()));

	while(gib_max_n()>=0  &&  route.get(gib_max_n())==r->position_bei(0)) {
		// skip identical end tiles
		route.remove_at(gib_max_n());
	}
	// then append
	for( unsigned int i=0;  i<=hops;  i++ ) {
		route.append(r->position_bei(i),16);
	}
}

void
route_t::insert(koord3d k)
{
	if(route.get_size()>=route.get_count()-1) {
		route.resize(route.get_size()+16);
	}
//DBG_MESSAGE("route_t::insert()","insert %d,%d",k.x, k.y);
	route.insert_at(0,k);
}

void
route_t::append(koord3d k)
{
	while(route.get_count()>1  &&  route.get(route.get_count()-1)==route.get(route.get_count()-2)) {
		route.remove_at(route.get_count()-1);
	}
	if(route.get_count()==0  ||  k!=route.get(route.get_count()-1)) {
		route.append(k,16);
	}
	route.append(k,16);	// the last is always double
}


void
route_t::remove_koord_from(int i) {
	while(i<gib_max_n()) {
		route.remove_at(gib_max_n());
	}
	route.append( route.at(gib_max_n()) );	// the last is always double
}



/**
 * Appends a straight line from the last koord3d in route to the desired target.
 * Will return false if failed
 * @author prissi
 */
bool
route_t::append_straight_route(karte_t *welt, koord3d dest )
{
	if(route.get_count()<=1  ||  !welt->ist_in_kartengrenzen(dest.gib_2d())) {
		return false;
	}

	while(route.get_count()>1  &&  route.get(route.get_count()-2)==route.get(route.get_count()-1)) {
		route.remove_at(route.get_count()-1);
	}

	// then try to calculate direct route
	koord pos=route.get(gib_max_n()).gib_2d();
	const koord ziel=dest.gib_2d();
	route.resize( route.get_count()+abs_distance(pos,ziel)+2 );
	while(pos!=ziel) {
		// shortest way
		koord diff;
		if(abs(pos.x-ziel.x)>=abs(pos.y-ziel.y)) {
			diff = (pos.x>ziel.x) ? koord(-1,0) : koord(1,0);
		}
		else {
			diff = (pos.y>ziel.y) ? koord(0,-1) : koord(0,1);
		}
		pos += diff;
		if(!welt->ist_in_kartengrenzen(pos)) {
			break;
		}
		route.append( welt->lookup(pos)->gib_kartenboden()->gib_pos(),16 );
	}
	route.append( route.get(route.get_count()-1),16 );
	DBG_MESSAGE("route_t::append_straight_route()","to (%i,%i) found.",ziel.x,ziel.y);

	return pos==ziel;
}




static inline bool am_i_there(karte_t *welt,
            koord3d pos,
            koord3d ziel,
            bool block_test)
{
	bool ok = false;

	if(block_test) {
		halthandle_t halt1 = haltestelle_t::gib_halt(welt, pos);
		halthandle_t halt2 = haltestelle_t::gib_halt(welt, ziel);

		ok = halt1.is_bound() ? (halt1 == halt2) : (pos == ziel);
	}
	else {
		ok = (pos == ziel);
	}

	return ok;
}




// node arrays
route_t::nodestruct* route_t::nodes=NULL;
bool route_t::node_in_use=false;

/* find the route to an unknow location
 * @author prissi
 */
bool
route_t::find_route(karte_t *welt,
                    const koord3d start,
                    fahrer_t *fahr, const uint32 /*max_khm*/, uint8 start_dir, uint32 max_depth )
{
	bool ok = false;

	// check for existing koordinates
	if(welt->lookup(start)==NULL) {
		return false;
	}

	// some thing for the search
	const weg_t::typ wegtyp = fahr->gib_wegtyp();
	const grund_t *gr;
	grund_t *to;

	// memory in static list ...
	const int MAX_STEP = 65536;
	if(nodes==NULL) {
		nodes = new ANode[MAX_STEP+4+1];
	}
	int step = 0;

//	welt->unmarkiere_alle();	// test in closed list are likely faster ...
	INT_CHECK("route 347");

	// arrays for A*
	static vector_tpl <ANode *> open = vector_tpl <ANode *>(0);
	vector_tpl <ANode *> close =vector_tpl <ANode *>(0);

	// nothing in lists
	open.clear();
	close.clear();	// close list may be short than mark/unmark (hopefully)

	// we clear it here probably twice: does not hurt ...
	route.clear();

	ANode *tmp = &(nodes[step++]);
	tmp->parent = NULL;
	tmp->gr = welt->lookup(start);

	// first tile is not valid?!?
	if(!fahr->ist_befahrbar(tmp->gr)) {
		return false;
	}

	GET_NODE();

	// start in open
	open.append(tmp,256);

//DBG_MESSAGE("route_t::find_route()","calc route from %d,%d,%d",start.x, start.y, start.z);
	do {
		// Hajo: this is too expensive to be called each step
		if((step & 127) == 0) {
			INT_CHECK("route 161");
		}

		tmp = open.at( 0 );
		open.remove_at( 0 );

		close.append(tmp,16);
		gr = tmp->gr;

//DBG_DEBUG("add to close","(%i,%i,%i) f=%i",gr->gib_pos().x,gr->gib_pos().y,gr->gib_pos().z,tmp->f);
		// already there
		if(fahr->ist_ziel(gr)) {
			// we added a target to the closed list: we are finished
			break;
		}

		// testing all four possible directions
		// (since this goes the other way round compared to normal search, we must eventually invert the mask)
		const weg_t *w=gr->gib_weg(wegtyp);
		ribi_t::ribi ribi = w->gib_ribi_unmasked();
		if(w  &&  w->gib_ribi_maske()!=ribi_t::keine) {
			ribi &= (~ribi_t::rueckwaerts(w->gib_ribi_maske()));
		}

		for(int r=0; r<4; r++) {

			// a way goes here, and it is not marked (i.e. in the closed list)
			if(  (ribi & ribi_t::nsow[r] & start_dir)!=0  // allowed dir (we can restrict the first step by start_dir)
				&& abs_distance(start.gib_2d(),gr->gib_pos().gib_2d()+koord::nsow[r])<max_depth	// not too far away
				&& gr->get_neighbour(to, wegtyp/*weg_t::invalid*/, koord::nsow[r])  // is connected
				&& fahr->ist_befahrbar(to)	// can be driven on
			) {
				unsigned index;

				// already in open list?
				for(  index=0;  index<open.get_count();  index++  ) {
					if(  open.get(index)->gr==to  ) {
						break;
					}
				}
				// in open list => ignore this
				if(index<open.get_count()) {
					continue;
				}


				// already in closed list (i.e. all processed nodes)
				for( index=0;  index<close.get_count();  index++  ) {
					if(  close.get(index)->gr==to  ) {
						break;
					}
				}
				// in close list => ignore this
				if(index<close.get_count()) {
					continue;
				}

				// not in there or taken out => add new
				ANode *k=&(nodes[step++]);

				k->parent = tmp;
				k->gr = to;

//DBG_DEBUG("insert to open","%i,%i,%i",to->gib_pos().x,to->gib_pos().y,to->gib_pos().z);
				// insert here
				open.append(k,16);

			}
		}

		// ok, now no more restrains
		start_dir = ribi_t::alle;

	} while(open.get_count()>0  &&  step<MAX_STEP  &&  open.get_count()<max_depth);

	INT_CHECK("route 194");

//DBG_DEBUG("reached","");
	// target reached?
	if(!fahr->ist_ziel(gr)  ||  step >= MAX_STEP) {
		dbg->warning("route_t::find_route()","Too many steps (%i>=max %i) in route (too long/complex)",step,MAX_STEP);
	}
	else {
		// reached => construct route
		route.insert_at( 0, tmp->gr->gib_pos() );
		while(tmp != NULL) {
			route.insert_at( 0, tmp->gr->gib_pos() );
//DBG_DEBUG("add","%i,%i",tmp->gr->gib_pos().x,tmp->gr->gib_pos().y);
			tmp = tmp->parent;
		}
		ok = true;
    }

	RELEASE_NODE();
	return ok;
}




bool
route_t::intern_calc_route(karte_t *welt, const koord3d ziel, const koord3d start, fahrer_t *fahr, const uint32 max_speed, const uint32 max_cost)
{
	bool ok = false;

	// check for existing koordinates
	if(welt->lookup(start)==NULL  ||  welt->lookup(ziel)==NULL) {
		return false;
	}

	// some thing for the search
	const weg_t::typ wegtyp = fahr->gib_wegtyp();
	const grund_t *gr;
	grund_t *to;

	// memory in static list ...
	const int MAX_STEP = max(65530,umgebung_t::max_route_steps);	// may need very much memory => configurable
	if(nodes==NULL) {
		nodes = new ANode[MAX_STEP+4+1];
	}
	int step = 0;

//	welt->unmarkiere_alle();	// test in closed list are likely faster ...
	INT_CHECK("route 347");

	// arrays for A*
	static vector_tpl <ANode *> open = vector_tpl <ANode *>(0);

	const bool is_airplane = fahr->gib_wegtyp()==weg_t::luft;

	// nothing in lists
	open.clear();
	welt->unmarkiere_alle();

	// we clear it here probably twice: does not hurt ...
	route.clear();

	ANode *tmp = &(nodes[step++]);
	tmp->parent = NULL;
	tmp->gr = welt->lookup(start);
	tmp->f = calc_distance(start,ziel);
	tmp->g = 0;
	tmp->dir = 0;

	// first tile is not valid?!?
	if(!fahr->ist_befahrbar(tmp->gr)) {
		return false;
	}

	GET_NODE();

	// start in open
	open.append(tmp,256);

//DBG_MESSAGE("route_t::itern_calc_route()","calc route from %d,%d,%d to %d,%d,%d",ziel.x, ziel.y, ziel.z, start.x, start.y, start.z);
	do {
		tmp = open.at( open.get_count()-1 );
		open.remove_at( open.get_count()-1 );

		gr = tmp->gr;
		welt->markiere(gr);

//DBG_DEBUG("add to close","(%i,%i,%i) f=%i",gr->gib_pos().x,gr->gib_pos().y,gr->gib_pos().z,tmp->f);

		// already there
		if(gr->gib_pos() == ziel) {
			// we added a target to the closed list: we are finished
			break;
		}

		// testing all four possible directions
		const ribi_t::ribi ribi =  fahr->gib_ribi(gr);
		for(int r=0; r<4; r++) {

			// a way in our direction?
			if(  (ribi & ribi_t::nsow[r])==0  ) {
				continue;
			}

			to = NULL;
			if(is_airplane) {
				const planquadrat_t *pl=welt->lookup(gr->gib_pos().gib_2d()+koord::nsow[r]);
				if(pl) {
					to = pl->gib_kartenboden();
				}
			}

			// a way goes here, and it is not marked (i.e. in the closed list)
			if((to  ||  gr->get_neighbour(to, wegtyp, koord::nsow[r]))  &&  fahr->ist_befahrbar(to)  &&  !welt->ist_markiert(to)) {

				// new values for cost g
				uint32 new_g = tmp->g + fahr->gib_kosten(to,max_speed);

				// check for curves (usually, one would need the lastlast and the last;
				// if not there, then we could just take the last
				uint8 current_dir;
				if(tmp->parent!=NULL) {
					current_dir = ribi_typ( tmp->parent->gr->gib_pos().gib_2d(), to->gib_pos().gib_2d() );
					if(tmp->dir!=current_dir) {
						new_g += 5;
						if(tmp->parent->dir!=tmp->dir) {
							// discourage double turns
							new_g += 10;
						}
					}
				}
				else {
					 current_dir = ribi_typ( gr->gib_pos().gib_2d(), to->gib_pos().gib_2d() );
				}

				const uint32 new_f = new_g+calc_distance( to->gib_pos(), ziel );

				unsigned index;

				// already in open list and better?
				if(open.get_count()>0) {
					for(  index=open.get_count()-1;  index>0  &&  open.get(index)->f<=new_f;  index--  ) {
						if(open.get(index)->gr==gr) {
							break;
						}
					}

					if(open.get(index)->gr==gr) {
						// it is already contained in the list
						// and it is lower in f ...
						continue;
					}
				}

				// it may or may not be in the list; but since the arrays are sorted
				// we find out about this during inserting!

				// not in there or taken out => add new
				ANode *k=&(nodes[step++]);

				// Hajo: this is too expensive to be called each step
				if((step & 15) == 0) {
					INT_CHECK("route 161");
				}

				k->parent = tmp;
				k->gr = to;
				k->g = new_g;
				k->f = new_f;
				k->dir = current_dir;

				// insert sorted
				for(  index=0;  index<open.get_count()  &&  new_f<open.get(index)->f;  index++  ) {
					if(open.get(index)->gr==to) {
						open.remove_at(index);
						index --;
					}
				}
				// was best f so far => append
				if(index>=open.get_count()) {
					open.append(k,16);
				}
				else {
					open.insert_at(index,k);
				}

//DBG_DEBUG("insert to open","(%i,%i,%i)  f=%i at %i",to->gib_pos().x,to->gib_pos().y,to->gib_pos().z,k->f, index);
			}
		}
	} while(open.get_count()>0  &&  !am_i_there(welt, gr->gib_pos(), ziel, false)  &&  step<MAX_STEP  &&  tmp->g<max_cost);

	INT_CHECK("route 194");

//DBG_DEBUG("reached","%i,%i",tmp->pos.x,tmp->pos.y);
	// target reached?
	if(!am_i_there(welt, tmp->gr->gib_pos(), ziel, false)  || step >= MAX_STEP  ||  tmp->parent==NULL) {
		dbg->warning("route_t::intern_calc_route()","Too many steps (%i>=max %i) in route (too long/complex)",step,MAX_STEP);
	}
	else {
		// reached => construct route
		while(tmp != NULL) {
			route.append( tmp->gr->gib_pos(), 256 );
//DBG_DEBUG("add","%i,%i",tmp->pos.x,tmp->pos.y);
			tmp = tmp->parent;
		}
		ok = true;
    }

	RELEASE_NODE();

	return ok;
}



/* searches route, uses intern_calc_route() for distance between stations
 * handles only driving in stations by itself
 * corrected 12/2005 for station search
 * @author Hansj�rg Malthaner, prissi
 */
bool
route_t::calc_route(karte_t *welt,
                    const koord3d ziel, const koord3d start,
                    fahrer_t *fahr, const uint32 max_khm, const uint32 max_cost)
{
	block_tester = 0;
	route.clear();

	INT_CHECK("route 336");

	bool ok = intern_calc_route(welt, ziel, start, fahr, max_khm,max_cost);

	INT_CHECK("route 343");

	if( !ok ) {
DBG_MESSAGE("route_t::calc_route()","No route from %d,%d to %d,%d found",start.x, start.y, ziel.x, ziel.y);
		// no route found
		route.resize(1);
		route.append(start);	// just to be save
		return false;

	}
	else {
		halthandle_t halt = welt->lookup(start)->gib_halt();

		// only needed for stations: go to the very end
		if(halt.is_bound()) {

			// does only make sence for trains
			if(fahr->gib_wegtyp()==weg_t::schiene  ||  fahr->gib_wegtyp()==weg_t::monorail  ||  fahr->gib_wegtyp()==weg_t::schiene_strab) {

				int max_n = route.get_count()-1;

				const koord zv = route.at(max_n).gib_2d() - route.at(max_n-1).gib_2d();
//DBG_DEBUG("route_t::calc_route()","zv=%i,%i",zv.x,zv.y);

				const int ribi = ribi_typ(zv);//fahr->gib_ribi(welt->lookup(start));
				grund_t *gr=welt->lookup(start);

				while(gr->get_neighbour(gr,fahr->gib_wegtyp(),zv)  &&  gr->gib_halt() == halt  &&   fahr->ist_befahrbar(gr)   &&  (fahr->gib_ribi(gr)&&ribi)!=0) {
					// stop at end of track! (prissi)
//DBG_DEBUG("route_t::calc_route()","add station at %i,%i",gr->gib_pos().x,gr->gib_pos().y);
					route.append(gr->gib_pos(),12);
				}

			}
		}
//DBG_DEBUG("route_t::calc_route()","calc route from %d,%d to %d,%d with %i",start.x, start.y, ziel.x, ziel.y, route.get_count()-2 );
	}
	return true;
}




void
route_t::rdwr(loadsave_t *file)
{
	int max_n = route.get_count()-1;
	int i;

	file->rdwr_long(max_n, "\n");
	if(file->is_loading()) {
		koord3d k;
		route.clear();
		route.resize(max_n+2);
		for(i=0;  i<=max_n;  i++ ) {
			k.rdwr(file);
			route.append( k );
		}
	}
	else {
		// writing
		for(i=0; i<=max_n; i++) {
			route.at(i).rdwr( file );
		}
	}
}
