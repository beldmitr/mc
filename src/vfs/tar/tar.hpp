#pragma once
#include <sys/types.h>
#include <errno.h>
#include <ctype.h>


#include "lib/global.hpp"
#include "lib/util.hpp"
#include "lib/unixcompat.hpp"     /* makedev() */
#include "lib/widget.hpp"         /* message() */

#include "lib/vfs/vfs.hpp"
#include "lib/vfs/utilvfs.hpp"
#include "lib/vfs/xdirentry.hpp"
#include "lib/vfs/gc.hpp"         /* vfs_rmstamp */

class Tar
{
private:
    /* tar files are made in basic blocks of this size.  */
    static inline const int BLOCKSIZE = 512;

    /* Sparse files are not supported in POSIX ustar format.  For sparse files
       with a POSIX header, a GNU extra header is provided which holds overall
       sparse information and a few sparse descriptors.  When an old GNU header
       replaces both the POSIX header and the GNU extra header, it holds some
       sparse descriptors too.  Whether POSIX or not, if more sparse descriptors
       are still needed, they are put into as many successive sparse headers as
       necessary.  The following constants tell how many sparse descriptors fit
       in each kind of header able to hold them.  */
    static inline const int SPARSES_IN_EXTRA_HEADER = 16;
    static inline const int SPARSES_IN_OLDGNU_HEADER = 4;
    static inline const int SPARSES_IN_SPARSE_HEADER = 21;
private:
    enum archive_format
    {
        TAR_UNKNOWN = 0,
        TAR_V7,
        TAR_USTAR,
        TAR_POSIX,
        TAR_GNU
    };

    typedef struct
    {
        struct vfs_s_super base;    /* base class */

        int fd;
        struct stat st;
        enum archive_format type;   /* Type of the archive */
    } tar_super_t;

    typedef enum
    {
        STATUS_BADCHECKSUM,
        STATUS_SUCCESS,
        STATUS_EOFMARK,
        STATUS_EOF
    } ReadStatus;

    /* The old GNU format header conflicts with POSIX format in such a way that
       POSIX archives may fool old GNU tar's, and POSIX tar's might well be
       fooled by old GNU tar archives.  An old GNU format header uses the space
       used by the prefix field in a POSIX header, and cumulates information
       normally found in a GNU extra header.  With an old GNU tar header, we
       never see any POSIX header nor GNU extra header.  Supplementary sparse
       headers are allowed, however.  */
    /* Descriptor for a single file hole */
    struct sparse
    {                               /* byte offset */
        /* cppcheck-suppress unusedStructMember */
        char offset[12];            /*   0 */
        /* cppcheck-suppress unusedStructMember */
        char numbytes[12];          /*  12 */
        /*  24 */
    };

    /* POSIX header */
    struct posix_header
    {                               /* byte offset */
        char name[100];             /*   0 */
        char mode[8];               /* 100 */
        char uid[8];                /* 108 */
        char gid[8];                /* 116 */
        char size[12];              /* 124 */
        char mtime[12];             /* 136 */
        char chksum[8];             /* 148 */
        char typeflag;              /* 156 */
        char linkname[100];         /* 157 */
        char magic[6];              /* 257 */
        char version[2];            /* 263 */
        char uname[32];             /* 265 */
        char gname[32];             /* 297 */
        char devmajor[8];           /* 329 */
        char devminor[8];           /* 337 */
        char prefix[155];           /* 345 */
        /* 500 */
    };

    struct oldgnu_header
    {                               /* byte offset */
        char unused_pad1[345];      /*   0 */
        char atime[12];             /* 345 */
        char ctime[12];             /* 357 */
        char offset[12];            /* 369 */
        char longnames[4];          /* 381 */
        char unused_pad2;           /* 385 */
        struct sparse sp[SPARSES_IN_OLDGNU_HEADER];
        /* 386 */
        char isextended;            /* 482 */
        char realsize[12];          /* 483 */
        /* 495 */
    };

    /* Extension header for sparse files, used immediately after the GNU extra
       header, and used only if all sparse information cannot fit into that
       extra header.  There might even be many such extension headers, one after
       the other, until all sparse information has been recorded.  */
    struct sparse_header
    {                               /* byte offset */
        struct sparse sp[SPARSES_IN_SPARSE_HEADER];
        /*   0 */
        char isextended;            /* 504 */
        /* 505 */
    };

    /* The GNU extra header contains some information GNU tar needs, but not
       foreseen in POSIX header format.  It is only used after a POSIX header
       (and never with old GNU headers), and immediately follows this POSIX
       header, when typeflag is a letter rather than a digit, so signaling a GNU
       extension.  */
    struct extra_header
    {                               /* byte offset */
        char atime[12];             /*   0 */
        char ctime[12];             /*  12 */
        char offset[12];            /*  24 */
        char realsize[12];          /*  36 */
        char longnames[4];          /*  48 */
        char unused_pad1[68];       /*  52 */
        struct sparse sp[SPARSES_IN_EXTRA_HEADER];
        /* 120 */
        char isextended;            /* 504 */
        /* 505 */
    };

    /* tar Header Block, overall structure */
    union block
    {
        char buffer[BLOCKSIZE];
        struct posix_header header;
        struct extra_header extra_header;
        struct oldgnu_header oldgnu_header;
        struct sparse_header sparse_header;
    };

public:
    static void vfs_init_tarfs();

private:
    static int tar_fh_open (struct vfs_class *me, vfs_file_handler_t * fh, int flags, mode_t mode);

    static ssize_t tar_read (void *fh, char *buffer, size_t count);

    static int tar_super_same (const vfs_path_element_t * vpath_element, struct vfs_s_super *parc, const vfs_path_t * vpath, void *cookie);

    static void* tar_super_check (const vfs_path_t * vpath);

    /**
     * Main loop for reading an archive.
     * Returns 0 on success, -1 on error.
     */
    static int tar_open_archive(struct vfs_s_super *archive, const vfs_path_t * vpath, const vfs_path_element_t * vpath_element);

    /**
     * Return 1 for success, 0 if the checksum is bad, EOF on eof,
     * 2 for a block full of zeros (EOF marker).
     */
    static ReadStatus tar_read_header (struct vfs_class *me, struct vfs_s_super *archive, int tard, size_t * h_size);

    static void tar_fill_stat (struct vfs_s_super *archive, struct stat *st, union block *header, size_t h_size);

    static size_t tar_decode_header (union block *header, tar_super_t * arch);

    static ReadStatus tar_checksum (const union block *header);

    static void tar_skip_n_records (struct vfs_s_super *archive, int tard, size_t n);

    static union block* tar_get_next_block (struct vfs_s_super *archive, int tard);

    /* Returns fd of the open tar file */
    static int tar_open_archive_int (struct vfs_class *me, const vfs_path_t * vpath, struct vfs_s_super *archive);

    static void tar_free_archive (struct vfs_class *me, struct vfs_s_super *archive);

    static struct vfs_s_super* tar_new_archive (struct vfs_class* me);

    /**
     * Quick and dirty octal conversion.
     *
     * Result is -1 if the field is invalid (all blank, or nonoctal).
     */
    static long tar_from_oct(int digs, const char* where);

private:
    static inline struct vfs_s_subclass tarfs_subclass;
    static inline struct vfs_class* vfs_tarfs_ops = VFS_CLASS (&tarfs_subclass);

    /* As we open one archive at a time, it is safe to have this static */
    static inline off_t current_tar_position = 0;

    static inline union block block_buf;
};
