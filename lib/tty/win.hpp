/** \file win.h
 *  \brief Header: X terminal management: xterm and rxvt
 */

#pragma once

#include "lib/global.hpp"         /* <glib.h> */

/*** typedefs(not structures) and defined constants **********************************************/

/*** enums ***************************************************************************************/

/*** structures declarations (and typedefs of structures)*****************************************/

/*** global variables defined in .c file *********************************************************/

/*** declarations of public functions ************************************************************/

void show_rxvt_contents (int starty, unsigned char y1, unsigned char y2);
gboolean look_for_rxvt_extensions (void);

/*** inline functions ****************************************************************************/

