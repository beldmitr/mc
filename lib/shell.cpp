/*
   Provides a functions for working with shell.

   Copyright (C) 2006-2020
   Free Software Foundation, Inc.

   Written by:
   Slava Zanko <slavazanko@gmail.com>, 2015.

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

/** \file shell.c
 *  \brief Source: provides a functions for working with shell.
 */

#include <pwd.h>                /* for username in xterm title */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "global.hpp"
#include "util.hpp"


char Shell::rp_shell[PATH_MAX];

void Shell::Init()
{
    Shell *mc_shell;

    char *shell_name = Get_name_env();
    if(shell_name)
    {
        mc_shell = new Shell(shell_name);
    }

    if (!mc_shell)
        mc_shell = Get_installed_in_system();

    mc_shell->real_path = mc_realpath(mc_shell->path.c_str(), rp_shell);

    /* Find out what type of shell we have. Also consider real paths (resolved symlinks)
     * because e.g. csh might point to tcsh, ash to dash or busybox, sh to anything. */

    if (!mc_shell->real_path.empty())
        Recognize_real_path(mc_shell);

    if (mc_shell->type == Shell::Type::SHELL_NONE)
        Recognize_path(mc_shell);

    if (mc_shell->type == Shell::Type::SHELL_NONE)
        mc_global.tty.use_subshell = FALSE;

    mc_global.shell = std::shared_ptr<Shell>(mc_shell);

}

void Shell::Recognize_path (Shell* mc_shell)
{
    /* If shell is not symlinked to busybox, it is safe to assume it is a real shell */
    if (std::strstr(mc_shell->path.c_str(), "/bash") != NULL || getenv ("BASH") != NULL)
    {
        mc_shell->type = Shell::Type::SHELL_BASH;
        mc_shell->name = "bash";
    }
    else if (std::strstr(mc_shell->path.c_str(), "/sh") != NULL || getenv ("SH") != NULL)
    {
        mc_shell->type = Shell::Type::SHELL_SH;
        mc_shell->name = "sh";
    }
    else if (std::strstr(mc_shell->path.c_str(), "/ash") != NULL || getenv ("ASH") != NULL)
    {
        mc_shell->type = Shell::Type::SHELL_ASH_BUSYBOX;
        mc_shell->name = "ash";
    }
    else
        mc_shell->type = Shell::Type::SHELL_NONE;
}

Shell* Shell::Get_installed_in_system ()
{
    Shell* mc_shell = new Shell;

    /* 3rd choice: look for existing shells supported as MC subshells.  */
    if (access ("/bin/bash", X_OK) == 0)
        mc_shell->path = "/bin/bash";
    else if (access ("/bin/ash", X_OK) == 0)
        mc_shell->path = "/bin/ash";
    else if (access ("/bin/dash", X_OK) == 0)
        mc_shell->path = "/bin/dash";
    else if (access ("/bin/busybox", X_OK) == 0)
        mc_shell->path = "/bin/busybox";
    else if (access ("/bin/zsh", X_OK) == 0)
        mc_shell->path = "/bin/zsh";
    else if (access ("/bin/tcsh", X_OK) == 0)
        mc_shell->path = "/bin/tcsh";
    else if (access ("/bin/csh", X_OK) == 0)
        mc_shell->path = "/bin/csh";
        /* No fish as fallback because it is so much different from other shells and
         * in a way exotic (even though user-friendly by name) that we should not
         * present it as a subshell without the user's explicit intention. We rather
         * will not use a subshell but just a command line.
         * else if (access("/bin/fish", X_OK) == 0)
         *     mc_global.tty.shell = g_strdup ("/bin/fish");
         */
    else
        /* Fallback and last resort: system default shell */
        mc_shell->path = "/bin/sh";

    return mc_shell;
}

void Shell::Recognize_real_path (Shell* mc_shell)
{
    if (std::strstr(mc_shell->path.c_str(), "/zsh") != NULL
        || std::strstr(mc_shell->real_path.c_str(), "/zsh") != NULL
        || getenv ("ZSH_VERSION") != NULL)
    {
        /* Also detects ksh symlinked to zsh */
        mc_shell->type = Shell::Type::SHELL_ZSH;
        mc_shell->name = "zsh";
    }
    else if (std::strstr(mc_shell->path.c_str(), "/tcsh") != NULL
             || std::strstr(mc_shell->real_path.c_str(), "/tcsh") != NULL)
    {
        /* Also detects csh symlinked to tcsh */
        mc_shell->type = Shell::Type::SHELL_TCSH;
        mc_shell->name = "tcsh";
    }
    else if (std::strstr(mc_shell->path.c_str(), "/csh") != NULL
             || std::strstr(mc_shell->real_path.c_str(), "/csh") != NULL)
    {
        mc_shell->type = Shell::Type::SHELL_TCSH;
        mc_shell->name = "csh";
    }
    else if (std::strstr(mc_shell->path.c_str(), "/fish") != NULL
             || std::strstr(mc_shell->real_path.c_str(), "/fish") != NULL)
    {
        mc_shell->type = Shell::Type::SHELL_FISH;
        mc_shell->name = "fish";
    }
    else if (std::strstr(mc_shell->path.c_str(), "/dash") != NULL
             || std::strstr(mc_shell->real_path.c_str(), "/dash") != NULL)
    {
        /* Debian ash (also found if symlinked to by ash/sh) */
        mc_shell->type = Shell::Type::SHELL_DASH;
        mc_shell->name = "dash";
    }
    else if (std::strstr(mc_shell->real_path.c_str(), "/busybox") != NULL)
    {
        /* If shell is symlinked to busybox, assume it is an ash, even though theoretically
         * it could also be a hush (a mini shell for non-MMU systems deactivated by default).
         * For simplicity's sake we assume that busybox always contains an ash, not a hush.
         * On embedded platforms or on server systems, /bin/sh often points to busybox.
         * Sometimes even bash is symlinked to busybox (CONFIG_FEATURE_BASH_IS_ASH option),
         * so we need to check busybox symlinks *before* checking for the name "bash"
         * in order to avoid that case. */
        mc_shell->type = Shell::Type::SHELL_ASH_BUSYBOX;
        mc_shell->name = mc_shell->path.c_str();
    }
    else
        mc_shell->type = Shell::Type::SHELL_NONE;
}

char* Shell::Get_name_env ()
{
    char *shell_name = NULL;

    const char *shell_env = std::getenv("SHELL");
    if ((shell_env == NULL) || (shell_env[0] == '\0'))
    {
        /* 2nd choice: user login shell */
        struct passwd *pwd = getpwuid (geteuid ());
        if (pwd)
            shell_name = g_strdup (pwd->pw_shell);
    }
    else
        /* 1st choice: SHELL environment variable */
        shell_name = g_strdup (shell_env);

    return shell_name;
}

//Shell* Shell::Get_from_env ()
//{
//    char *shell_name = Get_name_env();
//    if(!shell_name) return nullptr;
//
//    Shell* mc_shell = new Shell(shell_name);
//    return mc_shell;
//}
