/** \file info.h
 *  \brief Header: panel managing
 */

#pragma once

/*** typedefs(not structures) and defined constants **********************************************/

/*** enums ***************************************************************************************/

/*** structures declarations (and typedefs of structures)*****************************************/

struct WInfo;
typedef struct WInfo WInfo;

/*** global variables defined in .c file *********************************************************/

/*** declarations of public functions ************************************************************/

WInfo *info_new (int y, int x, int lines, int cols);

/*** inline functions ****************************************************************************/

