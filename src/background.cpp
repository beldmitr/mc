/* {{{ Copyright */

/* Background support.

   Copyright (C) 1996-2020
   Free Software Foundation, Inc.

   Written by:
   Miguel de Icaza, 1996

   This file is part of the Midnight Commander.

   The Midnight Commander is free software: you can redistribute it
   and/or modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation, either version 3 of the License,
   or (at your option) any later version.

   The Midnight Commander is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* }}} */

/** \file background.c
 *  \brief Source: Background support
 */

#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>           /* waitpid() */

#include "lib/global.hpp"

#include "lib/unixcompat.hpp"
#include "lib/tty/key.hpp"        /* add_select_channel(), delete_select_channel() */
#include "lib/widget.hpp"         /* message() */
#include "lib/event-types.hpp"

#include "filemanager/fileopctx.hpp"      /* file_op_context_t */

#include "background.hpp"

int Background::do_background(file_op_context_t* ctx, char* info)
{
    int comm[2];                /* control connection stream */
    int back_comm[2];           /* back connection */

    if (pipe(comm) == -1)
        return (-1);

    if (pipe(back_comm) == -1)
        return (-1);

    pid_t pid = fork ();
    if (pid == -1)
    {
        int saved_errno = errno;

        close(comm[0]);
        close(comm[1]);
        close(back_comm[0]);
        close(back_comm[1]);
        errno = saved_errno;

        return (-1);
    }

    if (pid == 0)
    {
        parent_fd = comm[1];
        from_parent_fd = back_comm[0];

        mc_global.we_are_background = TRUE;
        top_dlg = nullptr;

        /* Make stdin/stdout/stderr point somewhere */
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);

        int nullfd = open ("/dev/null", O_RDWR);
        if (nullfd != -1)
        {
            while (dup2(nullfd, STDIN_FILENO) == -1 && errno == EINTR)
            {}

            while (dup2(nullfd, STDOUT_FILENO) == -1 && errno == EINTR)
            {}

            while (dup2(nullfd, STDERR_FILENO) == -1 && errno == EINTR)
            {}
        }

        return 0;
    }
    else
    {
        ctx->pid = pid;
        register_task_running(ctx, pid, comm[0], back_comm[1], info);
        return 1;
    }
}

int Background::parent_call(void* routine, file_op_context_t* ctx, int argc, ...)
{
    va_list ap;

    va_start(ap, argc);
    int ret = parent_va_call(routine, (void*)ctx, argc, ap);
    va_end(ap);

    return ret;
}

char* Background::parent_call_string(void* routine, int argc, ...)
{
    va_list ap;

    va_start(ap, argc);
    char *str = parent_va_call_string(routine, argc, ap);
    va_end(ap);

    return str;
}

void Background::unregister_task_running(pid_t pid, int fd)
{
    destroy_task(pid);
    delete_select_channel(fd);
}

void Background::unregister_task_with_pid (pid_t pid)
{
    int fd = destroy_task(pid);
    if (fd != -1)
        delete_select_channel(fd);
}

gboolean Background::background_parent_call (const char* , const char* , void* , void* data)
{
    auto* event_data = static_cast<ev_background_parent_call_t *>(data);

    event_data->ret.i = parent_va_call(event_data->routine, event_data->ctx, event_data->argc, event_data->ap);

    return TRUE;
}

gboolean Background::background_parent_call_string(const char* , const char* , void* , void* data)
{
    auto* event_data = static_cast<ev_background_parent_call_t*>(data);

    event_data->ret.s = parent_va_call_string(event_data->routine, event_data->argc, event_data->ap);

    return TRUE;
}

void Background::register_task_running(file_op_context_t* ctx, pid_t pid, int fd, int to_child, char* info)
{
    TaskList *New = g_new (TaskList, 1);
    New->pid = pid;
    New->info = info;
    New->state = Task_Running;
    New->next = task_list;
    New->fd = fd;
    New->to_child_fd = to_child;
    task_list = New;

    add_select_channel(fd, background_attention, ctx);
}

int Background::destroy_task(pid_t pid)
{
    TaskList *p = task_list;
    TaskList *prev = nullptr;

    while(p != nullptr)
    {
        if(p->pid == pid)
        {
            int fd = p->fd;

            if(prev != nullptr)
                prev->next = p->next;
            else
                task_list = p->next;
            g_free(p->info);
            g_free(p);
            return fd;
        }
        prev = p;
        p = p->next;
    }

    /* pid not found */
    return (-1);
}

int Background::reading_failed(int i, char** data)
{
    while (i >= 0)
        g_free(data[i--]);
    message(D_ERROR, _("Background protocol error"), "%s", _("Reading failed"));
    return 0;
}

int Background::background_attention(int fd, void* closure)
{
    int have_ctx;
    union
    {
        int (*have_ctx0) (int);
        int (*have_ctx1) (int, char *);
        int (*have_ctx2) (int, char *, char *);
        int (*have_ctx3) (int, char *, char *, char *);
        int (*have_ctx4) (int, char *, char *, char *, char *);

        int (*non_have_ctx0) (file_op_context_t *, int);
        int (*non_have_ctx1) (file_op_context_t *, int, char *);
        int (*non_have_ctx2) (file_op_context_t *, int, char *, char *);
        int (*non_have_ctx3) (file_op_context_t *, int, char *, char *, char *);
        int (*non_have_ctx4) (file_op_context_t *, int, char *, char *, char *, char *);

        char *(*ret_str0) ();
        char *(*ret_str1) (char *);
        char *(*ret_str2) (char *, char *);
        char *(*ret_str3) (char *, char *, char *);
        char *(*ret_str4) (char *, char *, char *, char *);

        void *pointer;
    } routine;
    /*    void *routine; */
    int argc, i, status;
    char *data[MAXCALLARGS];
    TaskList *p;
    int to_child_fd = -1;
    ReturnType type;
    const char* background_process_error = _("Background process error");

    auto* ctx = static_cast<file_op_context_t*>(closure); // Smelly, but should be enough for now

    ssize_t bytes = read(fd, &routine.pointer, sizeof(routine));
    if (bytes == -1 || (size_t)bytes < (sizeof(routine)))
    {
        unregister_task_running(ctx->pid, fd);

        if (waitpid(ctx->pid, &status, WNOHANG) == 0)
        {
            /* the process is still running, but it misbehaves - kill it */
            kill(ctx->pid, SIGTERM);
            message(D_ERROR, background_process_error, "%s", _("Unknown error in child"));
            return 0;
        }

        /* 0 means happy end */
        if (WIFEXITED(status) && (WEXITSTATUS(status) == 0))
            return 0;

        message(D_ERROR, background_process_error, "%s", _("Child died unexpectedly"));

        return 0;
    }

    if (read(fd, &argc, sizeof(argc)) != sizeof(argc) ||
        read(fd, &type, sizeof(type)) != sizeof(type) ||
        read(fd, &have_ctx, sizeof(have_ctx)) != sizeof(have_ctx))
        return reading_failed(-1, data);

    if (argc > MAXCALLARGS)
        message(D_ERROR, _("Background protocol error"), "%s",
                 _("Background process sent us a request for more arguments\n"
                   "than we can handle."));

    if (have_ctx != 0 && read(fd, ctx, sizeof(*ctx)) != sizeof(*ctx))
        return reading_failed(-1, data);

    for (i = 0; i < argc; i++)
    {
        int size;

        if (read (fd, &size, sizeof(size)) != sizeof(size))
            return reading_failed(i - 1, data);

        data[i] = static_cast<char*>(g_malloc(size + 1));

        if (read(fd, data[i], size) != size)
            return reading_failed(i, data);

        data[i][size] = '\0';   /* NULL terminate the blocks (they could be strings) */
    }

    /* Find child task info by descriptor */
    /* Find before call, because process can destroy self after */
    for (p = task_list; p != nullptr; p = p->next)
        if(p->fd == fd)
            break;

    if(p != nullptr)
        to_child_fd = p->to_child_fd;

    if(to_child_fd == -1)
        message(D_ERROR, background_process_error, "%s", _("Unknown error in child"));

    /* Handle the call */
    if (type == Return_Integer)
    {
        int result = 0;

        if(have_ctx == 0)
            switch(argc)
            {
                case 0:
                    result = routine.have_ctx0(::Background);
                    break;
                case 1:
                    result = routine.have_ctx1(::Background, data[0]);
                    break;
                case 2:
                    result = routine.have_ctx2(::Background, data[0], data[1]);
                    break;
                case 3:
                    result = routine.have_ctx3(::Background, data[0], data[1], data[2]);
                    break;
                case 4:
                    result = routine.have_ctx4(::Background, data[0], data[1], data[2], data[3]);
                    break;
                default:
                    break;
            }
        else
            switch (argc)
            {
                case 0:
                    result = routine.non_have_ctx0(ctx, ::Background);
                    break;
                case 1:
                    result = routine.non_have_ctx1(ctx, ::Background, data[0]);
                    break;
                case 2:
                    result = routine.non_have_ctx2(ctx, ::Background, data[0], data[1]);
                    break;
                case 3:
                    result = routine.non_have_ctx3(ctx, ::Background, data[0], data[1], data[2]);
                    break;
                case 4:
                    result = routine.non_have_ctx4(ctx, ::Background, data[0], data[1], data[2], data[3]);
                    break;
                default:
                    break;
            }

        /* Send the result code and the value for shared variables */
        write(to_child_fd, &result, sizeof(result));
        if (have_ctx != 0 && to_child_fd != -1)
            write(to_child_fd, ctx, sizeof(*ctx));
    }
    else if (type == Return_String)
    {
        int len;
        char *resstr;

        /* FIXME: string routines should also use the Foreground/Background
         * parameter.  Currently, this is not used here
         */
        switch (argc)
        {
            case 0:
                resstr = routine.ret_str0();
                break;
            case 1:
                resstr = routine.ret_str1(data[0]);
                break;
            case 2:
                resstr = routine.ret_str2(data[0], data[1]);
                break;
            case 3:
                resstr = routine.ret_str3(data[0], data[1], data[2]);
                break;
            case 4:
                resstr = routine.ret_str4(data[0], data[1], data[2], data[3]);
                break;
            default:
                g_assert_not_reached();
        }

        if (resstr != nullptr)
        {
            len = strlen(resstr);
            write(to_child_fd, &len, sizeof(len));
            if (len != 0)
                write(to_child_fd, resstr, len);
            g_free(resstr);
        }
        else
        {
            len = 0;
            write(to_child_fd, &len, sizeof(len));
        }
    }

    for (i = 0; i < argc; i++)
        g_free(data[i]);

    repaint_screen();

    return 0;
}

void Background::parent_call_header(void* routine, int argc, ReturnType type, file_op_context_t* ctx)
{
    int have_ctx = ctx != nullptr ? 1 : 0;

    write(parent_fd, &routine, sizeof(routine));
    write(parent_fd, &argc, sizeof(argc));
    write(parent_fd, &type, sizeof(type));
    write(parent_fd, &have_ctx, sizeof(have_ctx));
    if (have_ctx != 0)
        write (parent_fd, ctx, sizeof(*ctx));
}

int Background::parent_va_call(void* routine, void* data, int argc, va_list ap)
{
    auto* ctx = static_cast<file_op_context_t*>(data);

    parent_call_header(routine, argc, Return_Integer, ctx);

    int i;
    for(i = 0; i < argc; i++)
    {
        void *value;
        int len = va_arg (ap, int);
        value = va_arg (ap, void*);
        write(parent_fd, &len, sizeof(len));
        write(parent_fd, value, len);
    }

    read(from_parent_fd, &i, sizeof(i));
    if (ctx != nullptr)
        read(from_parent_fd, ctx, sizeof(*ctx));

    return i;
}

char* Background::parent_va_call_string(void *routine, int argc, va_list ap)
{
    parent_call_header(routine, argc, Return_String, nullptr);

    int i;
    for(i = 0; i < argc; i++)
    {
        int len;
        void *value;

        len = va_arg(ap, int);
        value = va_arg(ap, void *);
        if (write(parent_fd, &len, sizeof(len)) != sizeof(len) || write(parent_fd, value, len) != len)
            return nullptr;
    }

    if (read(from_parent_fd, &i, sizeof (i)) != sizeof (i) || i == 0)
        return nullptr;

    char *str = static_cast<char*>(g_malloc (i + 1));
    if (read(from_parent_fd, str, i) != i)
    {
        g_free (str);
        return nullptr;
    }
    str[i] = '\0';
    return str;
}
