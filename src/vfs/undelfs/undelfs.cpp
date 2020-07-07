/*
   UnDel File System: Midnight Commander file system.

   This file system is intended to be used together with the
   ext2fs library to recover files from ext2fs file systems.

   Parts of this program were taken from the lsdel.c and dump.c files
   written by Ted Ts'o (tytso@mit.edu) for the ext2fs package.

   Copyright (C) 1995-2020
   Free Software Foundation, Inc.

   Written by:
   Miguel de Icaza, 1995
   Norbert Warmuth, 1997
   Pavel Machek, 2000

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

/**
 * \file
 * \brief Source: UnDel File System
 *
 * Assumptions:
 *
 * 1. We don't handle directories (thus undelfs_get_path is easy to write).
 * 2. Files are on the local file system (we do not support vfs files
 *    because we would have to provide an io_manager for the ext2fs tools,
 *    and I don't think it would be too useful to undelete files
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>

#ifdef HAVE_EXT2FS_EXT2_FS_H
#include <ext2fs/ext2_fs.h>
#else
/* asm/types.h defines its own umode_t */
#undef umode_t
#include <linux/ext2_fs.h>
#endif

#include <ctype.h>

#include "lib/global.hpp"
#include "lib/util.hpp"
#include "lib/widget.hpp"         /* message() */
#include "lib/vfs/vfs.hpp"

#include "undelfs.hpp"

/* To generate the . and .. entries use -2 */
#define READDIR_PTR_INIT 0

#define undelfs_stat undelfs_lstat


void Undelfs::vfs_init_undelfs()
{
    /* NULLize vfs_s_subclass members */
    memset (&undelfs_subclass, 0, sizeof (undelfs_subclass));

    vfs_init_class (vfs_undelfs_ops, "undelfs", VFSF_UNKNOWN, "undel");
    vfs_undelfs_ops->init = undelfs_init;
    vfs_undelfs_ops->open = undelfs_open;
    vfs_undelfs_ops->close = undelfs_close;
    vfs_undelfs_ops->read = undelfs_read;
    vfs_undelfs_ops->opendir = undelfs_opendir;
    vfs_undelfs_ops->readdir = undelfs_readdir;
    vfs_undelfs_ops->closedir = undelfs_closedir;
    vfs_undelfs_ops->stat = undelfs_stat;
    vfs_undelfs_ops->lstat = undelfs_lstat;
    vfs_undelfs_ops->fstat = undelfs_fstat;
    vfs_undelfs_ops->chdir = undelfs_chdir;
    vfs_undelfs_ops->lseek = undelfs_lseek;
    vfs_undelfs_ops->getid = undelfs_getid;
    vfs_undelfs_ops->nothingisopen = undelfs_nothingisopen;
    vfs_undelfs_ops->free = undelfs_free;
    vfs_register_class (vfs_undelfs_ops);
}

void Undelfs::com_err(const char* whoami, long err_code, const char* fmt, ...)
{
    va_list ap;

    va_start (ap, fmt);
    char *str = g_strdup_vprintf (fmt, ap);
    va_end (ap);

    message (D_ERROR, _("Ext2lib error"), "%s (%s: %ld)", str, whoami, err_code);
    g_free (str);
}

void Undelfs::undelfs_shutdown()
{
    if (fs)
        ext2fs_close (fs);

    fs = nullptr;
    MC_PTR_FREE (ext2_fname);
    MC_PTR_FREE (delarray);
    MC_PTR_FREE (block_buf);
}

void Undelfs::undelfs_get_path(const vfs_path_t* vpath, char** fsname, char** file)
{
    const vfs_path_element_t *path_element = vfs_path_get_by_index (vpath, -1);

    /* To look like filesystem, we have virtual directories
       undel://XXX, which have no subdirectories. XXX is replaced with
       hda5, sdb8 etc, which is assumed to live under /dev.
       -- pavel@ucw.cz */

    const char* dirname = path_element->path;

    *fsname = nullptr;

    if (strncmp (dirname, "undel://", 8) != 0)
        return;

    dirname += 8;

    /* Since we don't allow subdirectories, it's easy to get a filename,
     * just scan backwards for a slash */
    if (*dirname == 0)
        return;

    const char *p = dirname + strlen (dirname);
#if 0
    /* Strip trailing ./
     */
    if (p - dirname > 2 && IS_PATH_SEP (p[-1]) && p[-2] == '.')
        *(p = p - 2) = 0;
#endif

    while (p > dirname)
    {
        if (IS_PATH_SEP (*p))
        {
            char *tmp;

            *file = g_strdup (p + 1);
            tmp = g_strndup (dirname, p - dirname);
            *fsname = g_strconcat ("/dev/", tmp, (char *) nullptr);
            g_free (tmp);
            return;
        }
        p--;
    }
    *file = g_strdup ("");
    *fsname = g_strconcat ("/dev/", dirname, (char *) nullptr);
}

int Undelfs::undelfs_lsdel_proc (ext2_filsys _fs, blk_t* block_nr, int /*blockcnt*/, void* Private)
{
    auto *_lsd = static_cast<lsdel_struct*>(Private);

    _lsd->num_blocks++;

    if (*block_nr < _fs->super->s_first_data_block || *block_nr >= _fs->super->s_blocks_count)
    {
        _lsd->bad_blocks++;
        return BLOCK_ABORT;
    }

    if (!ext2fs_test_block_bitmap (_fs->block_map, *block_nr))
        _lsd->free_blocks++;

    return 0;
}

int Undelfs::undelfs_loaddel()
{
    int retval, count;
    ext2_ino_t ino;
    struct ext2_inode inode;
    ext2_inode_scan scan;

    max_delarray = 100;
    num_delarray = 0;
    delarray = static_cast<struct deleted_info*>(g_try_malloc (sizeof (struct deleted_info) * max_delarray));
    if (!delarray)
    {
        message (D_ERROR, undelfserr, "%s", _("not enough memory"));
        return 0;
    }
    block_buf = static_cast<char*>(g_try_malloc (fs->blocksize * 3));
    if (!block_buf)
    {
        message (D_ERROR, undelfserr, "%s", _("while allocating block buffer"));
        goto free_delarray;
    }
    retval = ext2fs_open_inode_scan (fs, 0, &scan);
    if (retval != 0)
    {
        message (D_ERROR, undelfserr, _("open_inode_scan: %d"), retval);
        goto free_block_buf;
    }
    retval = ext2fs_get_next_inode (scan, &ino, &inode);
    if (retval != 0)
    {
        message (D_ERROR, undelfserr, _("while starting inode scan %d"), retval);
        goto error_out;
    }
    count = 0;
    while (ino)
    {
        if ((count++ % 1024) == 0)
            vfs_print_message (_("undelfs: loading deleted files information %d inodes"), count);
        if (inode.i_dtime == 0)
            goto next;

        if (S_ISDIR (inode.i_mode))
            goto next;

        lsd.inode = ino;
        lsd.num_blocks = 0;
        lsd.free_blocks = 0;
        lsd.bad_blocks = 0;

        retval = ext2fs_block_iterate (fs, ino, 0, block_buf, undelfs_lsdel_proc, &lsd);
        if (retval)
        {
            message (D_ERROR, undelfserr, _("while calling ext2_block_iterate %d"), retval);
            goto next;
        }
        if (lsd.free_blocks && !lsd.bad_blocks)
        {
            if (num_delarray >= max_delarray)
            {
                auto *delarray_new = static_cast<deleted_info*>(g_try_realloc (delarray, sizeof (struct deleted_info) * (max_delarray + 50)));
                if (!delarray_new)
                {
                    message (D_ERROR, undelfserr, "%s", _("no more memory while reallocating array"));
                    goto error_out;     // TODO DB get rid of goto
                }
                delarray = delarray_new;
                max_delarray += 50;
            }

            delarray[num_delarray].ino = ino;
            delarray[num_delarray].mode = inode.i_mode;
            delarray[num_delarray].uid = inode.i_uid;
            delarray[num_delarray].gid = inode.i_gid;
            delarray[num_delarray].size = inode.i_size;
            delarray[num_delarray].dtime = inode.i_dtime;
            delarray[num_delarray].num_blocks = lsd.num_blocks;
            delarray[num_delarray].free_blocks = lsd.free_blocks;
            num_delarray++;
        }

        next:
        retval = ext2fs_get_next_inode (scan, &ino, &inode);
        if (retval)
        {
            message (D_ERROR, undelfserr, _("while doing inode scan %d"), retval);
            goto error_out;
        }
    }
    readdir_ptr = READDIR_PTR_INIT;
    ext2fs_close_inode_scan (scan);
    return 1;

    error_out:
    ext2fs_close_inode_scan (scan);
    free_block_buf:
    MC_PTR_FREE (block_buf);
    free_delarray:
    MC_PTR_FREE (delarray);
    return 0;
}

void* Undelfs::undelfs_opendir(const vfs_path_t* vpath)
{
    char *file, *f = nullptr;
    const vfs_path_element_t *path_element;

    path_element = vfs_path_get_by_index (vpath, -1);
    undelfs_get_path (vpath, &file, &f);
    if (file == nullptr)
    {
        g_free (f);
        return nullptr;
    }

    /* We don't use the file name */
    g_free (f);

    if (!ext2_fname || std::strcmp(ext2_fname, file))
    {
        undelfs_shutdown ();
        ext2_fname = file;
    }
    else
    {
        /* To avoid expensive re-scannings */
        readdir_ptr = READDIR_PTR_INIT;
        g_free (file);
        return fs;
    }

    if (ext2fs_open (ext2_fname, 0, 0, 0, unix_io_manager, &fs))
    {
        message (D_ERROR, undelfserr, _("Cannot open file %s"), ext2_fname);
        return nullptr;
    }
    vfs_print_message ("%s", _("undelfs: reading inode bitmap..."));
    if (ext2fs_read_inode_bitmap (fs))
    {
        message (D_ERROR, undelfserr, _("Cannot load inode bitmap from:\n%s"), ext2_fname);
        goto quit_opendir;
    }
    vfs_print_message ("%s", _("undelfs: reading block bitmap..."));
    if (ext2fs_read_block_bitmap (fs))
    {
        message (D_ERROR, undelfserr, _("Cannot load block bitmap from:\n%s"), ext2_fname);
        goto quit_opendir;
    }
    /* Now load the deleted information */
    if (!undelfs_loaddel ())
        goto quit_opendir;
    vfs_print_message (_("%s: done."), path_element->Class->name);
    return fs;
    quit_opendir:
    vfs_print_message (_("%s: failure"), path_element->Class->name);
    ext2fs_close (fs);
    fs = nullptr;
    return nullptr;
}

void* Undelfs::undelfs_readdir(void *vfs_info)
{
    static union vfs_dirent undelfs_readdir_data;
    static char *const dirent_dest = undelfs_readdir_data.dent.d_name;

    if (vfs_info != fs)
    {
        message (D_ERROR, undelfserr, "%s", _("vfs_info is not fs!"));
        return nullptr;
    }
    if (readdir_ptr == num_delarray)
        return nullptr;
    if (readdir_ptr < 0)
        strcpy (dirent_dest, readdir_ptr == -2 ? "." : "..");
    else
        g_snprintf (dirent_dest, MC_MAXPATHLEN, "%ld:%d",
                    (long) delarray[readdir_ptr].ino, delarray[readdir_ptr].num_blocks);
    readdir_ptr++;

    return &undelfs_readdir_data;
}

int Undelfs::undelfs_closedir (void* /*vfs_info*/)
{
    return 0;
}

void* Undelfs::undelfs_open(const vfs_path_t* vpath, int flags, mode_t mode)
{
    char *file, *f = nullptr;
    ext2_ino_t inode, i;
    undelfs_file *p = nullptr;
    (void) flags;
    (void) mode;

    /* Only allow reads on this file system */
    undelfs_get_path (vpath, &file, &f);
    if (file == nullptr)
    {
        g_free (f);
        return nullptr;
    }

    if (!ext2_fname || std::strcmp(ext2_fname, file))
    {
        message (D_ERROR, undelfserr, "%s", _("You have to chdir to extract files first"));
        g_free (file);
        g_free (f);
        return nullptr;
    }
    inode = atol(f);

    /* Search the file into delarray */
    for (i = 0; i < (ext2_ino_t) num_delarray; i++)
    {
        if (inode != delarray[i].ino)
            continue;

        /* Found: setup all the structures needed by read */
        p = (undelfs_file *) g_try_malloc (((gsize) sizeof (undelfs_file)));
        if (!p)
        {
            g_free (file);
            g_free (f);
            return nullptr;
        }
        p->buf = static_cast<char*>(g_try_malloc (fs->blocksize));
        if (!p->buf)
        {
            g_free (p);
            g_free (file);
            g_free (f);
            return nullptr;
        }
        p->inode = inode;
        p->finished = FALSE;
        p->f_index = i;
        p->error_code = 0;
        p->pos = 0;
        p->size = delarray[i].size;
    }
    g_free (file);
    g_free (f);
    undelfs_usage++;
    return p;
}

int Undelfs::undelfs_close(void* vfs_info)
{
    auto *p = static_cast<undelfs_file *>(vfs_info);
    g_free (p->buf);
    g_free (p);
    undelfs_usage--;
    return 0;
}

int Undelfs::undelfs_dump_read(ext2_filsys param_fs, blk_t* blocknr, int blockcnt, void* Private)
{
    int copy_count;
    auto *p = static_cast<undelfs_file*>(Private);

    if (blockcnt < 0)
        return 0;

    if (*blocknr)
    {
        p->error_code = io_channel_read_blk (param_fs->io, *blocknr, 1, p->buf);
        if (p->error_code)
            return BLOCK_ABORT;
    }
    else
        memset (p->buf, 0, param_fs->blocksize);

    if (p->pos + (off_t) p->count < p->current)
    {
        p->finished = TRUE;
        return BLOCK_ABORT;
    }
    if (p->pos > p->current + param_fs->blocksize)
    {
        p->current += param_fs->blocksize;
        return 0;               /* we have not arrived yet */
    }

    /* Now, we know we have to extract some data */
    if (p->pos >= p->current)
    {

        /* First case: starting pointer inside this block */
        if (p->pos + (off_t) p->count <= p->current + param_fs->blocksize)
        {
            /* Fully contained */
            copy_count = p->count;
            p->finished = (p->count != 0);
        }
        else
        {
            /* Still some more data */
            copy_count = param_fs->blocksize - (p->pos - p->current);
        }
        memcpy (p->dest_buffer, p->buf + (p->pos - p->current), copy_count);
    }
    else
    {
        /* Second case: we already have passed p->pos */
        if (p->pos + (off_t) p->count < p->current + param_fs->blocksize)
        {
            copy_count = (p->pos + p->count) - p->current;
            p->finished = (p->count != 0);
        }
        else
        {
            copy_count = param_fs->blocksize;
        }
        memcpy (p->dest_buffer, p->buf, copy_count);
    }
    p->dest_buffer += copy_count;
    p->current += param_fs->blocksize;
    if (p->finished)
    {
        return BLOCK_ABORT;
    }
    return 0;
}

ssize_t Undelfs::undelfs_read(void* vfs_info, char* buffer, size_t count)
{
    auto *p = static_cast<undelfs_file *>(vfs_info);

    p->dest_buffer = buffer;
    p->current = 0;
    p->finished = FALSE;
    p->count = count;

    if (p->pos + (off_t) p->count > p->size)
    {
        p->count = p->size - p->pos;
    }
    int retval = ext2fs_block_iterate (fs, p->inode, 0, nullptr, undelfs_dump_read, p);
    if (retval)
    {
        message (D_ERROR, undelfserr, "%s", _("while iterating over blocks"));
        return -1;
    }
    if (p->error_code && !p->finished)
        return 0;
    p->pos = p->pos + (p->dest_buffer - buffer);
    return p->dest_buffer - buffer;
}

long Undelfs::undelfs_getindex(char *path)
{
    ext2_ino_t inode = atol(path);
    int i;

    for (i = 0; i < num_delarray; i++)
    {
        if (delarray[i].ino == inode)
            return i;
    }
    return -1;
}

int Undelfs::undelfs_stat_int (int inode_index, struct stat *buf)
{
    buf->st_dev = 0;
    buf->st_ino = delarray[inode_index].ino;
    buf->st_mode = delarray[inode_index].mode;
    buf->st_nlink = 1;
    buf->st_uid = delarray[inode_index].uid;
    buf->st_gid = delarray[inode_index].gid;
    buf->st_size = delarray[inode_index].size;
    buf->st_atime = delarray[inode_index].dtime;
    buf->st_ctime = delarray[inode_index].dtime;
    buf->st_mtime = delarray[inode_index].dtime;
#ifdef HAVE_STRUCT_STAT_ST_MTIM
    buf->st_atim.tv_nsec = buf->st_mtim.tv_nsec = buf->st_ctim.tv_nsec = 0;
#endif
    return 0;
}

int Undelfs::undelfs_lstat(const vfs_path_t* vpath, struct stat* buf)
{
    int inode_index;
    char *file, *f = nullptr;

    undelfs_get_path (vpath, &file, &f);
    if (file == nullptr)
    {
        g_free (f);
        return 0;
    }

    /* When called from save_cwd_stats we get an incorrect file and f here:
       e.g. incorrect                         correct
       path = "undel:/dev/sda1"          path="undel:/dev/sda1/401:1"
       file = "/dev"                     file="/dev/sda1"
       f    = "sda1"                     f   ="401:1"
       If the first char in f is no digit -> return error */
    if (!isdigit (*f))
    {
        g_free (file);
        g_free (f);
        return -1;
    }

    if (!ext2_fname || std::strcmp(ext2_fname, file))
    {
        g_free (file);
        g_free (f);
        message (D_ERROR, undelfserr, "%s", _("You have to chdir to extract files first"));
        return 0;
    }
    inode_index = undelfs_getindex(f);
    g_free (file);
    g_free (f);

    if (inode_index == -1)
        return -1;

    return undelfs_stat_int (inode_index, buf);
}

int Undelfs::undelfs_fstat(void* vfs_info, struct stat* buf)
{
    auto* p = static_cast<undelfs_file *>(vfs_info);

    return undelfs_stat_int (p->f_index, buf);
}

int Undelfs::undelfs_chdir(const vfs_path_t* vpath)
{
    char *file, *f = nullptr;

    undelfs_get_path (vpath, &file, &f);
    if (file == nullptr)
    {
        g_free (f);
        return (-1);
    }

    /* We may use access because ext2 file systems are local */
    /* this could be fixed by making an ext2fs io manager to use */
    /* our vfs, but that is left as an exercise for the reader */
    int fd = open (file, O_RDONLY);
    if (fd == -1)
    {
        message (D_ERROR, undelfserr, _("Cannot open file \"%s\""), file);
        g_free(f);
        g_free(file);
        return -1;
    }
    close(fd);
    g_free(f);
    g_free(file);
    return 0;
}

off_t Undelfs::undelfs_lseek (void* /*vfs_info*/, off_t /*offset*/, int /*whence*/)
{
    return -1;
}

vfsid Undelfs::undelfs_getid(const vfs_path_t* vpath)
{
    char *fname = nullptr, *fsname;

    undelfs_get_path(vpath, &fsname, &fname);
    gboolean ok = (fsname != nullptr);

    g_free(fname);
    g_free(fsname);

    return ok ? (vfsid) fs : nullptr;
}

gboolean Undelfs::undelfs_nothingisopen(vfsid /*id*/)
{
    return (undelfs_usage == 0);
}

void Undelfs::undelfs_free(vfsid /*id*/)
{
    undelfs_shutdown();
}

#ifdef ENABLE_NLS
int Undelfs::undelfs_init(struct vfs_class* me)
{
    undelfserr = _(undelfserr);
    return 1;
}
#endif