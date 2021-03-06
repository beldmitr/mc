
/** \file hline.h
 *  \brief Header: WHLine widget
 */

#pragma once

/*** typedefs(not structures) and defined constants **********************************************/

#define HLINE(x) ((WHLine *)(x))

/*** enums ***************************************************************************************/

/*** structures declarations (and typedefs of structures)*****************************************/

typedef struct
{
    Widget widget;
    char *text;
    gboolean auto_adjust_cols;  /* Compute widget.cols from parent width? */
    gboolean transparent;       /* Paint in the default color fg/bg */
} WHLine;

/*** global variables defined in .c file *********************************************************/

/*** declarations of public functions ************************************************************/

WHLine *hline_new (int y, int x, int width);
void hline_set_text (WHLine * l, const char *text);

/*** inline functions ****************************************************************************/


