/** \file  file.h
 *  \brief Header: File and directory operation routines
 */

#pragma once

#include "lib/global.hpp"

/*** typedefs(not structures) and defined constants **********************************************/

/*** enums ***************************************************************************************/

/*** structures declarations (and typedefs of structures)*****************************************/

/*** global variables defined in .c file *********************************************************/

/*** declarations of public functions ************************************************************/

/* Misc Unix functions */
int my_mkdir (const vfs_path_t * vpath, mode_t mode);
int my_rmdir (const char *path);

/*** inline functions ****************************************************************************/



