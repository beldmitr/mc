/** \file textconf.h
 *  \brief Header: prints features specific for this build
 */

#pragma once

class TextConf
{
public:
    static void show_version();

    static void show_datadirs_extended();

#ifdef ENABLE_CONFIGURE_ARGS
    static void show_configure_options();
#endif

private:
#ifdef ENABLE_VFS
    static inline const char *const vfs_supported[] = {
#ifdef ENABLE_VFS_CPIO
    "cpiofs",
#endif
#ifdef ENABLE_VFS_TAR
    "tarfs",
#endif
#ifdef ENABLE_VFS_SFS
    "sfs",
#endif
#ifdef ENABLE_VFS_EXTFS
    "extfs",
#endif
#ifdef ENABLE_VFS_UNDELFS
    "ext2undelfs",
#endif
#ifdef ENABLE_VFS_FTP
    "ftpfs",
#endif
#ifdef ENABLE_VFS_SFTP
    "sftpfs",
#endif
#ifdef ENABLE_VFS_FISH
    "fish",
#endif
#ifdef ENABLE_VFS_SMB
    "smbfs",
#endif /* ENABLE_VFS_SMB */
    NULL
};
#endif /* ENABLE_VFS */

    static inline const char *const features[] = {

#ifdef USE_INTERNAL_EDIT
#ifdef HAVE_ASPELL
            N_("With builtin Editor and Aspell support"),
#else
            N_("With builtin Editor"),
#endif /* HAVE_ASPELL */
#endif /* USE_INTERNAL_EDIT */

#ifdef ENABLE_SUBSHELL
#ifdef SUBSHELL_OPTIONAL
            N_("With optional subshell support"),
#else
            N_("With subshell support as default"),
#endif
#endif /* !ENABLE_SUBSHELL */

#ifdef ENABLE_BACKGROUND
            N_("With support for background operations"),
#endif

#ifdef HAVE_LIBGPM
            N_("With mouse support on xterm and Linux console"),
#else
            N_("With mouse support on xterm"),
#endif

#ifdef HAVE_TEXTMODE_X11_SUPPORT
            N_("With support for X11 events"),
#endif

#ifdef ENABLE_NLS
            N_("With internationalization support"),
#endif

#ifdef HAVE_CHARSET
            N_("With multiple codepages support"),
#endif

            NULL
    };
};


/*** inline functions ****************************************************************************/

