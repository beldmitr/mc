/** \file background.h
 *  \brief Header: Background support
 */

#pragma once

#include <sys/types.h>          /* pid_t */
#include "filemanager/fileopctx.hpp"
/*** typedefs(not structures) and defined constants **********************************************/



/*** enums ***************************************************************************************/

/*** structures declarations (and typedefs of structures)*****************************************/

/*** global variables defined in .c file *********************************************************/



/*** declarations of public functions ************************************************************/



/*** inline functions ****************************************************************************/

class Background
{
private:
static const int MAXCALLARGS = 4;           /* Number of arguments supported */
public:
    enum TaskState
    {
        Task_Running,
        Task_Stopped
    };
    struct TaskList
    {
        int fd;
        int to_child_fd;
        pid_t pid;
        int state;
        char *info;
        struct TaskList *next;
    };
private:
    enum ReturnType
    {
        Return_String,
        Return_Integer
    };
public:
    static inline TaskList* task_list = nullptr;
private:
    /* File descriptor for talking to our parent */
    static inline int parent_fd;

    /* File descriptor for messages from our parent */
    static inline int from_parent_fd;
public:
    /**
 * Try to make the Midnight Commander a background job
 *
 * Returns:
 *  1 for parent
 *  0 for child
 * -1 on failure
 */
    static int do_background(file_op_context_t* ctx, char* info);

    static int parent_call(void* routine, file_op_context_t* ctx, int argc, ...);

    static char* parent_call_string(void* routine, int argc, ...);

    static void unregister_task_running(pid_t pid, int fd);

    static void unregister_task_with_pid(pid_t pid);

    /* event callback */
    static gboolean background_parent_call (const char* event_group_name, const char* event_name, void* init_data, void* data);

    /* event callback */
    static gboolean background_parent_call_string (const char* event_group_name, const char* event_name, void* init_data, void* data);

private:
    static void register_task_running(file_op_context_t* ctx, pid_t pid, int fd, int to_child, char* info);

    static int destroy_task(pid_t pid);

    /* {{{ Parent handlers */

    /* Parent/child protocol
     *
     * the child (the background) process send the following:
     * void *routine -- routine to be invoked in the parent
     * int  nargc    -- number of arguments
     * int  type     -- Return argument type.
     *
     * If the routine is zero, then it is a way to tell the parent
     * that the process is dying.
     *
     * nargc arguments in the following format:
     * int size of the coming block
     * size bytes with the block
     *
     * Now, the parent loads all those and then invokes
     * the routine with pointers to the information passed
     * (we just support pointers).
     *
     * If the return type is integer:
     *
     *     the parent then writes an int to the child with
     *     the return value from the routine and the values
     *     of any global variable that is modified in the parent
     *     currently: do_append and recursive_result.
     *
     * If the return type is a string:
     *
     *     the parent writes the resulting string length
     *     if the result string was NULL or the empty string,
     *     then the length is zero.
     *     The parent then writes the string length and frees
     *     the result string.
     */

    /*
     * Receive requests from background process and invoke the
     * specified routine
     */
    static int reading_failed (int i, char** data);

    static int background_attention(int fd, void* closure);

    /* }}} */

    /* {{{ client RPC routines */

    /* Sends the header for a call to a routine in the parent process.  If the file
     * operation context is not NULL, then it requests that the first parameter of
     * the call be a file operation context.
     */

    static void parent_call_header(void* routine, int argc, ReturnType type, file_op_context_t* ctx);

    static int parent_va_call(void* routine, void* data, int argc, va_list ap);

    static char* parent_va_call_string (void* routine, int argc, va_list ap);
};
