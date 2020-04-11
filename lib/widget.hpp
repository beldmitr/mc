/** \file widget.h
 *  \brief Header: MC widget and dialog manager: main include file.
 */

#pragma once

#include "lib/global.hpp"         /* GLib */

/* main forward declarations */
struct Widget;
typedef struct Widget Widget;
struct WGroup;
typedef struct WGroup WGroup;

/* Please note that the first element in all the widgets is a     */
/* widget variable of type Widget.  We abuse this fact everywhere */

#include "lib/widget/rect.hpp"
#include "lib/widget/widget-common.hpp"
#include "lib/widget/group.hpp"
#include "lib/widget/background.hpp"
#include "lib/widget/frame.hpp"
#include "lib/widget/dialog.hpp"
#include "lib/widget/history.hpp"
#include "lib/widget/button.hpp"
#include "lib/widget/buttonbar.hpp"
#include "lib/widget/check.hpp"
#include "lib/widget/hline.hpp"
#include "lib/widget/gauge.hpp"
#include "lib/widget/groupbox.hpp"
#include "lib/widget/label.hpp"
#include "lib/widget/listbox.hpp"
#include "lib/widget/menu.hpp"
#include "lib/widget/radio.hpp"
#include "lib/widget/input.hpp"
#include "lib/widget/listbox-window.hpp"
#include "lib/widget/quick.hpp"
#include "lib/widget/wtools.hpp"
#include "lib/widget/dialog-switch.hpp"

/*** typedefs(not structures) and defined constants **********************************************/

/*** enums ***************************************************************************************/

/*** structures declarations (and typedefs of structures)*****************************************/

/*** global variables defined in .c file *********************************************************/

/*** declarations of public functions ************************************************************/

/*** inline functions ****************************************************************************/

