
#ifndef MC__TTY_SLANG_H
#define MC__TTY_SLANG_H

#ifdef HAVE_SLANG_SLANG_H
#include <slang/slang.h>
#else
#include <slang.h>
#endif /* HAVE_SLANG_SLANG_H */

/*** typedefs(not structures) and defined constants **********************************************/

#define KEY_F(x) (1000 + x)

#define ACS_VLINE    SLSMG_VLINE_CHAR
#define ACS_HLINE    SLSMG_HLINE_CHAR
#define ACS_LTEE     SLSMG_LTEE_CHAR
#define ACS_RTEE     SLSMG_RTEE_CHAR
#define ACS_TTEE     SLSMG_UTEE_CHAR
#define ACS_BTEE     SLSMG_DTEE_CHAR
#define ACS_ULCORNER SLSMG_ULCORN_CHAR
#define ACS_LLCORNER SLSMG_LLCORN_CHAR
#define ACS_URCORNER SLSMG_URCORN_CHAR
#define ACS_LRCORNER SLSMG_LRCORN_CHAR
#define ACS_PLUS     SLSMG_PLUS_CHAR

#define COLS  SLtt_Screen_Cols
#define LINES SLtt_Screen_Rows

/*** enums ***************************************************************************************/
// FIXME this was in enum, but it caused an error
// /home/wbull/CLionProjects/mc/lib/tty/tty-slang.hpp:36:9: error: expected identifier before numeric constant
// 36 |         KEY_BACKSPACE = 400,

#define KEY_BACKSPACE   400
#define KEY_END 401
#define KEY_UP  402
#define KEY_DOWN    403
#define KEY_LEFT    404
#define KEY_RIGHT   405
#define KEY_HOME 406
#define KEY_A1  407
#define KEY_C1 408
#define KEY_NPAGE 409
#define KEY_PPAGE 410
#define KEY_IC  411
#define KEY_ENTER 412
#define KEY_DC  413
#define KEY_SCANCEL 414
#define KEY_BTAB    415

/*** structures declarations (and typedefs of structures)*****************************************/

/*** global variables defined in .c file *********************************************************/

extern int reset_hp_softkeys;

/*** declarations of public functions ************************************************************/

/*** inline functions ****************************************************************************/

#endif /* MC_TTY_SLANG_H */
