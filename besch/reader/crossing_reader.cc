#include <stdio.h>

#include "../../bauer/wegbauer.h"

#include "../kreuzung_besch.h"
#include "crossing_reader.h"

#include "../obj_node_info.h"

#include "../../simdebug.h"


void crossing_reader_t::register_obj(obj_besch_t *&data)
{
    kreuzung_besch_t *besch = static_cast<kreuzung_besch_t *>(data);

    wegbauer_t::register_besch(besch);
}



bool crossing_reader_t::successfully_loaded() const
{
    return wegbauer_t::alle_kreuzungen_geladen();
}


obj_besch_t * crossing_reader_t::read_node(FILE *fp, obj_node_info_t &node)
{
#ifdef _MSC_VER /* no var array on the stack supported */
    char *besch_buf = static_cast<char *>(alloca(node.size));
#else
  // Hajo: reading buffer is better allocated on stack
  char besch_buf [node.size];
#endif


  char *info_buf = new char[sizeof(obj_besch_t) + node.children * sizeof(obj_besch_t *)];

  kreuzung_besch_t *besch = new kreuzung_besch_t();
  besch->node_info = reinterpret_cast<obj_besch_info_t *>(info_buf);

  // Hajo: Read data
  fread(besch_buf, node.size, 1, fp);
  char * p = besch_buf;

  // Hajo: old versions of PAK files have no version stamp.
  // But we know, the higher most bit was always cleared.

  const uint16 v = decode_uint16(p);
  const int version = v & 0x8000 ? v & 0x7FFF : 0;

  if(version == 0) {
    // old, nonversion node
    besch->wegtyp_ns = (uint8)v;
    besch->wegtyp_ow = (uint8)decode_uint16(p);
  }
  DBG_DEBUG("kreuzung_besch_t::read_node()","version=%i, ns=%d, ow=%d",v,besch->wegtyp_ns,besch->wegtyp_ow);
  return besch;
}
