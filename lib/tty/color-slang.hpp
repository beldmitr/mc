
/** \file color-slang.h
 *  \brief Header: S-Lang-specific color setup
 */

#pragma once

#include "tty-slang.hpp"          /* S-Lang headers */

/*** typedefs(not structures) and defined constants **********************************************/

/* When using Slang with color, we have all the indexes free but
 * those defined here (A_BOLD, A_ITALIC, A_UNDERLINE, A_REVERSE, A_BLINK)
 */

#ifndef A_BOLD
#define A_BOLD SLTT_BOLD_MASK
#endif /* A_BOLD */
#ifdef SLTT_ITALIC_MASK         /* available since slang-pre2.3.0-107 */
#ifndef A_ITALIC
#define A_ITALIC SLTT_ITALIC_MASK
#endif /* A_ITALIC */
#endif /* SLTT_ITALIC_MASK */
#ifndef A_UNDERLINE
#define A_UNDERLINE SLTT_ULINE_MASK
#endif /* A_UNDERLINE */
#ifndef A_REVERSE
#define A_REVERSE SLTT_REV_MASK
#endif /* A_REVERSE */
#ifndef A_BLINK
#define A_BLINK SLTT_BLINK_MASK
#endif /* A_BLINK */

/*** enums ***************************************************************************************/

// FIXME this was made by enum, but it caused an error
// /home/wbull/CLionProjects/mc/lib/tty/color-slang.hpp:39:5: error: expected identifier before numeric constant
// 39 |     COLOR_BLACK = 0,
#define COLOR_BLACK 0
#define COLOR_RED   1
#define COLOR_GREEN 2
#define COLOR_YELLOW    3
#define COLOR_BLUE  4
#define COLOR_MAGENTA   5
#define COLOR_CYAN  6
#define COLOR_WHITE 7

/*** structures declarations (and typedefs of structures)*****************************************/

/*** global variables defined in .c file *********************************************************/

/*** declarations of public functions ************************************************************/

/*** inline functions ****************************************************************************/


