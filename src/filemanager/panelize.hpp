/** \file panelize.h
 *  \brief Header: External panelization module
 */

#pragma once

/*** typedefs(not structures) and defined constants **********************************************/

/*** enums ***************************************************************************************/

/*** structures declarations (and typedefs of structures)*****************************************/

/*** global variables defined in .c file *********************************************************/

/*** declarations of public functions ************************************************************/

void external_panelize (void);
void load_panelize (void);
void save_panelize (void);
void done_panelize (void);
void cd_panelize_cmd (void);
void panelize_save_panel (WPanel * panel);
void panelize_change_root (const vfs_path_t * new_root);
void panelize_absolutize_if_needed (WPanel * panel);

/*** inline functions ****************************************************************************/


