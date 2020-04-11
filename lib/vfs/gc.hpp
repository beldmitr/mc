/**
 * \file
 * \brief Header: Virtual File System: garbage collection code
 */

#pragma once

#include "vfs.hpp"

/*** typedefs(not structures) and defined constants **********************************************/

/*** enums ***************************************************************************************/

/*** structures declarations (and typedefs of structures)*****************************************/

/*** global variables defined in .c file *********************************************************/

/*** declarations of public functions ************************************************************/

gboolean vfs_stamp (struct vfs_class *vclass, vfsid id);
void vfs_rmstamp (struct vfs_class *vclass, vfsid id);
void vfs_stamp_create (struct vfs_class *vclass, vfsid id);
void vfs_gc_done (void);

/*** inline functions ****************************************************************************/

