
/** \file tty-internal.h
 *  \brief Header: internal stuff of the terminal controlling library
 */

#pragma once

#include "lib/global.hpp"         /* include <glib.h> */

/*** typedefs(not structures) and defined constants **********************************************/

/* Taken from S-Lang's slutty.c */
#ifdef _POSIX_VDISABLE
#define NULL_VALUE _POSIX_VDISABLE
#else
#define NULL_VALUE 255
#endif

/*** enums ***************************************************************************************/

/*** structures declarations (and typedefs of structures)*****************************************/

/*** global variables defined in .c file *********************************************************/

/* The mouse is currently: TRUE - enabled, FALSE - disabled */
extern gboolean mouse_enabled;

/* terminal ca capabilities */
extern char *smcup;
extern char *rmcup;

/* pipe to handle SIGWINCH */
extern int sigwinch_pipe[2];

/*** declarations of public functions ************************************************************/

void tty_create_winch_pipe (void);
void tty_destroy_winch_pipe (void);

char *mc_tty_normalize_from_utf8 (const char *);
void tty_init_xterm_support (gboolean is_xterm);
int tty_lowlevel_getch (void);

/*** inline functions ****************************************************************************/
