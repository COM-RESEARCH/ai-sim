#ifndef dataobj_einstellungen_h
#define dataobj_einstellungen_h

#include "../utils/cstring_t.h"

/**
 * einstellungen.h
 *
 * Spieleinstellungen
 *
 * Hj. Malthaner
 *
 * April 2000
 */

class loadsave_t;

class einstellungen_t
{
public:
  enum consts {MIN_INDUSTRY_DENSITY=120, MAX_INDUSTRY_DENSITY=1200};

private:
    int groesse;
    int nummer;

    int industrie_dichte;

    int anzahl_staedte;

    int scroll_multi;

    /**
     * ab welchem level erzeugen gebaeude verkehr ?
     */
    int verkehr_level;

    /**
     * sollen Fussgaenger angezeigt werden ?
     */
    int show_pax;

     /**
     * Grundwasserspiegel ?
     */

    int grundwasser;

    double max_mountain_height;                  //01-Dec-01        Markus Weber    Added
    double map_roughness;                        //01-Dec-01        Markus Weber    Added



public:

    /**
     * If map is read from a heightfield, this is the name of the heightfield.
     * Set to empty string in order to avoid loading.
     * @author Hj. Malthaner
     */
    cstring_t heightfield;

    einstellungen_t();

    /**
     * Copy constructor, needed becuase of cstring
     * @author Hj. Malthaner
     */
    einstellungen_t(const einstellungen_t *);

    void setze_groesse(int g) {groesse=g;};
    int gib_groesse() const {return groesse;};

    void setze_karte_nummer(int n) {nummer=n;};
    int gib_karte_nummer() const {return nummer;};

    void setze_industrie_dichte(int d) {industrie_dichte=d;};
    int gib_industrie_dichte() const {return industrie_dichte;};

    void setze_anzahl_staedte(int n) {anzahl_staedte=n;};
    int gib_anzahl_staedte() const {return anzahl_staedte;};

    void setze_scroll_multi(int n) {scroll_multi=n;};
    int gib_scroll_multi() const {return scroll_multi;};

    void setze_verkehr_level(int l) {verkehr_level=l;};
    int gib_verkehr_level() const {return verkehr_level;};

    void setze_show_pax(bool yesno) {show_pax=yesno;};
    bool gib_show_pax() const {return show_pax != 0;};

    void rdwr(loadsave_t *file);

    void setze_grundwasser(int n) {grundwasser=n;};
    int gib_grundwasser() const {return grundwasser;};

    void setze_max_mountain_height(double n) {max_mountain_height=n;};          //01-Dec-01        Markus Weber    Added
    double gib_max_mountain_height() const {return max_mountain_height;};

    void setze_map_roughness(double n) {map_roughness=n;};                      //01-Dec-01        Markus Weber    Added
    double gib_map_roughness() const {return map_roughness;};

};


#endif // dataobj_einstellungen_h
