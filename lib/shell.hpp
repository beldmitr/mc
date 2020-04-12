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
    Shell()
    {
        Init();
    }

    ~Shell()
    {
    }

private:
    Type type;
    std::string name;
    std::string path;
    std::string real_path;

public:
    Type GetType() const
    {
        return type;
    }

    const char* GetPath() const
    {
        return path.c_str();
    }

private:
    char rp_shell[PATH_MAX];

private:
    void Init();
    void RecognizePath();
    void SetPathFromSystem();
    void RecognizeRealPath();
    std::string GetNameEnv();
};
