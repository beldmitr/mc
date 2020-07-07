#pragma once

#include <ext2fs/ext2fs.h>

#include "lib/vfs/xdirentry.hpp"

class Undelfs
{
public:
    static void vfs_init_undelfs();

    /**
     * This function overrides com_err() from libcom_err library.
     * It is used in libext2fs to report errors.
     */
    static void com_err(const char* whoami, long err_code, const char* fmt, ...);

private:
    struct deleted_info
    {
        ext2_ino_t ino;
        unsigned short mode;
        unsigned short uid;
        unsigned short gid;
        unsigned long size;
        time_t dtime;
        int num_blocks;
        int free_blocks;
    };

    struct lsdel_struct
    {
        ext2_ino_t inode;
        int num_blocks;
        int free_blocks;
        int bad_blocks;
    };

    typedef struct
    {
        int f_index;                /* file index into delarray */
        char *buf;
        int error_code;             /*  */
        off_t pos;                  /* file position */
        off_t current;              /* used to determine current position in itereate */
        gboolean finished;
        ext2_ino_t inode;
        int bytes_read;
        off_t size;

        /* Used by undelfs_read: */
        char *dest_buffer;          /* destination buffer */
        size_t count;               /* bytes to read */
    } undelfs_file;

private:
    /* We only allow one opened ext2fs */
    static inline char *ext2_fname;
    static inline ext2_filsys fs = nullptr;
    static inline struct lsdel_struct lsd;
    static inline struct deleted_info *delarray;
    static inline int num_delarray, max_delarray;
    static inline char *block_buf;
    static inline const char* undelfserr = N_("undelfs: error");
    static inline int readdir_ptr;
    static inline int undelfs_usage;

    static inline struct vfs_s_subclass undelfs_subclass;
    static inline struct vfs_class* vfs_undelfs_ops = VFS_CLASS (&undelfs_subclass);

private:
    static void undelfs_shutdown();

    static void undelfs_get_path(const vfs_path_t* vpath, char** fsname, char** file);

    static int undelfs_lsdel_proc(ext2_filsys _fs, blk_t* block_nr, int blockcnt, void* Private);

    /**
     * Load information about deleted files.
     * Don't abort if there is not enough memory - load as much as we can.
     */
    static int undelfs_loaddel();

    static void* undelfs_opendir(const vfs_path_t* vpath);

    static void* undelfs_readdir(void *vfs_info);

    static int undelfs_closedir (void *vfs_info);

    /* We do not support lseek */
    static void* undelfs_open (const vfs_path_t* vpath, int flags, mode_t mode);

    static int undelfs_close(void* vfs_info);

    static int undelfs_dump_read(ext2_filsys param_fs, blk_t* blocknr, int blockcnt, void* Private);

    static ssize_t undelfs_read(void* vfs_info, char* buffer, size_t count);

    static long undelfs_getindex(char *path);

    static int undelfs_stat_int (int inode_index, struct stat *buf);

    static int undelfs_lstat(const vfs_path_t* vpath, struct stat* buf);

    static int undelfs_fstat (void *vfs_info, struct stat *buf);

    static int undelfs_chdir (const vfs_path_t * vpath);

    /* this has to stay here for now: vfs layer does not know how to emulate it */
    static off_t undelfs_lseek (void* vfs_info, off_t offset, int whence);

    static vfsid undelfs_getid (const vfs_path_t* vpath);

    static gboolean undelfs_nothingisopen(vfsid id);

    static void undelfs_free(vfsid id);

#ifdef ENABLE_NLS
    static int undelfs_init (struct vfs_class* me);
#else
    #define undelfs_init nullptr
#endif

};
