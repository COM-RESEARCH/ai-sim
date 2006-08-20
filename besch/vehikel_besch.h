/*
 *
 *  vehikel_besch.h
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
#ifndef __VEHIKEL_BESCH_H
#define __VEHIKEL_BESCH_H

/*
 *  includes
 */
#include "text_besch.h"
#include "ware_besch.h"
#include "bildliste_besch.h"
#include "skin_besch.h"
#include "../dataobj/ribi.h"

#include "../boden/wege/weg.h"

#include "intro_dates.h"

/**
 * Vehicle type description - all attributes of a vehicle type
 *
 *  Kindknoten:
 *	0   Name
 *	1   Copyright
 *	2   Ware
 *	3   Rauch
 *	4   Bildliste leer
 *	5   Bildliste mit Fracht
 *	6   erlaubter Vorg�nger 1
 *	7   erlaubter Vorg�nger 2
 *	... ...
 *	n+5 erlaubter Vorg�nger n
 *	n+6 erlaubter Nachfolger 1
 *	n+7 erlaubter Nachfolger 2
 *	... ...
 *
 * @author Volker Meyer, Hj. Malthaner
 */
class vehikel_besch_t : public obj_besch_t {
    friend class vehicle_writer_t;
    friend class vehicle_reader_t;

public:
#if 0
    // Hajo: be careful, some of these values must match the entries
    // in schiene_t
    enum weg_t { strasse=0,
		 schiene=1,
		 wasser=2,
		 luft=3,
		 // Hajo: unused ATM: schiene_elektrifiziert=4,
		 schiene_monorail=5,
		 schiene_maglev=6,
		 schiene_strab=7, // Dario: Tramway
    };
#endif

    /**
     * Engine type
     * @author Hj. Malthaner
     */
    enum engine_t {
    	 unknown=-1,
      steam=0,
      diesel,
      electric,
      bio,
      sail,
      fuel_cell,
      hydrogene
    };


private:
    uint32  preis;
    uint16  zuladung;
    uint16  geschw;
    uint16  gewicht;
    uint32  leistung;
    uint16  betriebskosten;

    uint16  intro_date; // Hajo: introduction date
    uint16  obsolete_date; //phase out at
    uint16  gear;       // Hajo: engine gear (power multiplier)

    sint8  typ;         // Hajo: Das ist der weg_t
    sint8  sound;

    uint8  vorgaenger;	// Anzahl m�glicher Vorg�nger
    uint8  nachfolger;	// Anzahl m�glicher Nachfolger

    uint8  engine_type; // Hajo: diesel, steam, electric

	uint8 length[8];	// length to the next vehicle

public:
    const char *gib_name() const
    {
        return static_cast<const text_besch_t *>(gib_kind(0))->gib_text();
    }
    const char *gib_copyright() const
    {
		if(gib_kind(1)==NULL) {
			return NULL;
		}
		return static_cast<const text_besch_t *>(gib_kind(1))->gib_text();
    }
    const ware_besch_t *gib_ware() const
    {
	return static_cast<const ware_besch_t *>(gib_kind(2));
    }
    const skin_besch_t *gib_rauch() const
    {
	return static_cast<const skin_besch_t *>(gib_kind(3));
    }
    int gib_basis_bild() const
    {
	return gib_bild_nr(ribi_t::dir_sued, false);
    }

    int gib_bild_nr(ribi_t::dir dir, bool empty) const
    {
	const bildliste_besch_t *liste = static_cast<const bildliste_besch_t *>(gib_kind(empty ? 4 : 5));

	if(!liste) {
	    liste = static_cast<const bildliste_besch_t *>(gib_kind(4));
	}
	const bild_besch_t *bild = liste->gib_bild(dir);

	if(!bild) {
	    bild = liste->gib_bild(dir - 4);
	    if(!bild) {
		return -1;
	    }
	}
	return bild->bild_nr;
    }
    //
    // Liefert die erlaubten Vorg�nger.
    // liefert gib_vorgaenger(0) == NULL, so bedeutet das entweder alle
    // Vorg�nger sind erlaubt oder keine. Um das zu unterscheiden, sollte man
    // vorher hat_vorgaenger() befragen
    //
    const vehikel_besch_t *gib_vorgaenger(int i) const
    {
	if(i < 0 || i >= vorgaenger) {
	    return 0;
	}
	return static_cast<const vehikel_besch_t *>(gib_kind(6 + i));
    }
    //
    // Liefert die erlaubten Nachfolger.
    // liefert gib_nachfolger(0) == NULL, so bedeutet das entweder alle
    // Nachfolger sind erlaubt oder keine. Um das zu unterscheiden, sollte
    // man vorher hat_nachfolger() befragen
    //
    const vehikel_besch_t *gib_nachfolger(int i) const
    {
	if(i < 0 || i >= nachfolger) {
	    return 0;
	}
	return static_cast<const vehikel_besch_t *>(gib_kind(6 + vorgaenger + i));
    }
    int gib_vorgaenger_count() const
    {
	return vorgaenger;
    }
    int gib_nachfolger_count() const
    {
	return nachfolger;
    }
    weg_t::typ gib_typ() const
    {
	return static_cast<weg_t::typ>(typ);
    }
    int gib_zuladung() const
    {
	return zuladung;
    }
    int gib_preis() const
    {
	return preis;
    }
    int gib_geschw() const
    {
	return geschw;
    }
    int gib_gewicht() const
    {
	return gewicht;
    }
    int gib_leistung() const
    {
	return leistung;
    }
    int gib_betriebskosten() const
    {
	return betriebskosten;
    }
    int gib_sound() const
    {
	return sound;
    }


    /**
     * @return introduction year
     * @author Hj. Malthaner
     */
    uint16 get_intro_year_month() const {
      return intro_date;
    }

    /**
     * @return time when obsolete
     * @author prissi
     */
    uint16 get_retire_year_month() const {
      return obsolete_date;
    }

	// true if future
	bool is_future (const uint16 month_now) const
	{
		return month_now  &&  intro_date > month_now;
	}

	// true if obsolete
	bool is_retired (const uint16 month_now) const
	{
		return month_now  &&  obsolete_date <= month_now;
	}

    /**
     * 64 = 1.00
     * @return gear value
     * @author Hj. Malthaner
     */
    int get_gear() const {
      return gear;
    }


    /**
     * @return engine type
     * @author Hj. Malthaner
     */
    uint8 get_engine_type() const {
      return engine_type;
    }


	/* @return the normalized distance to the next vehicle
	 * @author prissi
	 */
	uint8 get_length_to_next( uint8 next_dir ) const {
		return length[next_dir];
	}

	/* @returns the real coordinates
	 * @author prissi
	 */
	const uint8 *gib_bild_daten(int dir)
	{
		const bildliste_besch_t *liste = static_cast<const bildliste_besch_t *>(gib_kind(4));
		const bild_besch_t *bild = liste->gib_bild(dir);

		if(!bild) {
			bild = liste->gib_bild(dir - 4);
			if(!bild) {
				return 0;
			}
		}
		return (const uint8 *)(bild->gib_daten());
	}

};

#endif // __VEHIKEL_BESCH_H
