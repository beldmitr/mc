/** \file timer.h
 *  \brief Header: shell structure
 */

#pragma once

#include <string>

class Shell
{
public:
    enum Type
    {
        SHELL_NONE,
        SHELL_SH,
        SHELL_BASH,
        SHELL_ASH_BUSYBOX,          /* BusyBox default shell (ash) */
        SHELL_DASH,                 /* Debian variant of ash */
        SHELL_TCSH,
        SHELL_ZSH,
        SHELL_FISH
    };
public:
    Shell() = default;

    Shell(char* path)
        :   path(path)
    {

    }

    ~Shell()
    {
    }

public:
    Type type;
    std::string name;
    std::string path;
    std::string real_path;

private:
    static char rp_shell[PATH_MAX];
public:
    static void Init();

private:
    static void Recognize_path (Shell* mc_shell);
    static Shell* Get_installed_in_system ();
    static void Recognize_real_path (Shell* mc_shell);
    static char* Get_name_env ();
//    static Shell* Get_from_env ();
};
