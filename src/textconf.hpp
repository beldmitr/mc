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
    static const char *const vfs_supported[];
#endif /* ENABLE_VFS */

    static const char *const features[];
};
