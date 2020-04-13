#pragma once

#include <sys/wait.h>
#include <csignal>

#include "lib/global.hpp"
#include "src/filemanager/file.hpp"
#include "src/filemanager/filegui.hpp"
#include "lib/charsets.hpp"
#include "consaver/cons.saver.hpp"        /* cons_saver_pid */


#ifdef ENABLE_SUBSHELL
#include "subshell/subshell.hpp"
#endif

class Util
{
private:
    static void SigchldHandlerNoSubshell (int sig)
    {
#ifdef __linux__
        int pid, status;

        if (mc_global.tty.console_flag == '\0')
            return;

        /* COMMENT: if it were true that after the call to handle_console(..INIT)
           the value of mc_global.tty.console_flag never changed, we could simply not install
           this handler at all if (!mc_global.tty.console_flag && !mc_global.tty.use_subshell). */

        /* That comment is no longer true.  We need to wait() on a sigchld
           handler (that's at least what the tarfs code expects currently). */

        pid = waitpid (cons_saver_pid, &status, WUNTRACED | WNOHANG);

        if (pid == cons_saver_pid)
        {
            if (WIFSTOPPED (status))
            {
                /* Someone has stopped cons.saver - restart it */
                kill (pid, SIGCONT);
            }
            else
            {
                /* cons.saver has died - disable console saving */
                handle_console (CONSOLE_DONE);
                mc_global.tty.console_flag = '\0';
            }
        }
        /* If we got here, some other child exited; ignore it */
#endif /* __linux__ */
    }

public:
    static bool CheckForDefault(const vfs_path_t *default_file_vpath, const vfs_path_t *file_vpath)
    {
        if (!exist_file (vfs_path_as_str (file_vpath)))
        {
            if (!exist_file (vfs_path_as_str (default_file_vpath)))
                return false;

            file_op_context_t *ctx = file_op_context_new (OP_COPY);
            file_op_total_context_t *tctx = file_op_total_context_new ();
            file_op_context_create_ui (ctx, 0, static_cast<filegui_dialog_type_t>(FALSE));
            copy_file_file (tctx, ctx, vfs_path_as_str (default_file_vpath), vfs_path_as_str (file_vpath));
            file_op_total_context_destroy (tctx);
            file_op_context_destroy (ctx);
        }

        return true;
    }

    /** POSIX version.  The only version we support.  */
    static void OS_Setup()
    {
        mc_global.shell = std::make_shared<Shell>();

        /* This is the directory, where MC was installed, on Unix this is DATADIR */
        /* and can be overriden by the MC_DATADIR environment variable */
        // TODO investigate: DB: datadir_env can't be std::string, it finishes with a crash
        const char *datadir_env = std::getenv("MC_DATADIR");
        if (datadir_env)
            mc_global.sysconfig_dir = datadir_env;
        else
            mc_global.sysconfig_dir = SYSCONFDIR;

        mc_global.share_data_dir = DATADIR;
    }

    /**
 * Check MC_SID to prevent running one mc from another.
 *
 * @return true if no parent mc in our session was found, false otherwise.
 */
    static bool CheckSid()
    {
        const char *sid_str = std::getenv ("MC_SID");
        if (!sid_str)
            return true;

        pid_t old_sid = std::strtol(sid_str, nullptr, 0);
        if (old_sid == 0)
            return true;

        pid_t my_sid = getsid (0);
        if (my_sid == -1)
            return true;

        /* The parent mc is in a different session, it's OK */
        return (old_sid != my_sid);
    }

    static void check_codeset ()
    {
        const char *current_system_codepage = str_detect_termencoding ();

#ifdef HAVE_CHARSET
        {
            const char *_display_codepage = CodepageDesc::get_codepage_id (mc_global.display_codepage);

            if (strcmp (_display_codepage, current_system_codepage) != 0)
            {
                mc_global.display_codepage = CodepageDesc::get_codepage_index (current_system_codepage);
                if (mc_global.display_codepage == -1)
                    mc_global.display_codepage = 0;

                mc_config_set_string (mc_global.main_config, CONFIG_MISC_SECTION, "display_codepage",
                                      CodepageDesc::cp_display);
            }
        }
#endif

        mc_global.utf8_display = str_isutf8 (current_system_codepage);
    }

    static void InitSigchld ()
    {
        struct sigaction sigchld_action;

        memset (&sigchld_action, 0, sizeof (sigchld_action));
        sigchld_action.sa_handler =
#ifdef ENABLE_SUBSHELL
mc_global.tty.use_subshell ? sigchld_handler :
#endif /* ENABLE_SUBSHELL */
    SigchldHandlerNoSubshell;

        sigemptyset (&sigchld_action.sa_mask);

#ifdef SA_RESTART
        sigchld_action.sa_flags = SA_RESTART;
#endif /* !SA_RESTART */

        if (sigaction (SIGCHLD, &sigchld_action, NULL) == -1)
        {
#ifdef ENABLE_SUBSHELL
            /*
             * This may happen on QNX Neutrino 6, where SA_RESTART
             * is defined but not implemented.  Fallback to no subshell.
             */
            mc_global.tty.use_subshell = FALSE;
#endif /* ENABLE_SUBSHELL */
        }
    }

};
