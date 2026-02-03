//  lisafs_commands.c
//  Part of lisafs.
//
//  Copyright © 2026 Christopher M. Hanson. All rights reserved.
//  See file COPYING for details.

#include "lisafs_commands.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sysexits.h>

#include "lisafs.h"


LISAFS_SOURCE_BEGIN


// MARK: - list command

int lisafs_list_print_directory(lisafs_directrec * _Nonnull directory, void * _Nullable context);
int lisafs_list_print_object(lisafs_objectrec * _Nonnull object, void * _Nullable context);
int lisafs_list_print_thread(lisafs_threadrec * _Nonnull thread, void * _Nullable context);
int lisafs_list_btree_entry_iterator(lisafs_directory_entry * _Nonnull entry, void * _Nullable context);
int lisafs_list_print_centry(lisafs_centry * _Nonnull centry, void * _Nullable context);
int lisafs_list_directory_entry_iterator(lisafs_centry * _Nonnull centry, void * _Nullable context);


int lisafs_list(int argc, char * _Nullable * _Nonnull argv)
{
    lisafs_fsversion fsversion = lisafs_get_fsversion(image);

    int iterate_err;
    if (fsversion == release3) {
        iterate_err = lisafs_iterate_btree_entries(image, lisafs_list_btree_entry_iterator, NULL);
    } else {
        iterate_err = lisafs_iterate_directory_entries(image, lisafs_list_directory_entry_iterator, NULL);
    }

    if (iterate_err == -1) {
        const char *errstr = strerror(errno);
        fprintf(stderr, "%s: error iterating %s: %s" "\n", program_name, image_file_path, errstr);
        return EX_DATAERR;
    }

    return EX_OK;
}


int lisafs_list_btree_entry_iterator(lisafs_directory_entry * _Nonnull entry, void * _Nullable context)
{
    switch (entry->header_only.etype) {
        case emptyentry:
        case killedentry:
        case removed:     return 0;

        case direntry:    return lisafs_list_print_directory(&entry->directory, context);

        case linkentry:
        case fileentry:
        case pipeentry:
        case ecentry:     return lisafs_list_print_object(&entry->object, context);
        case threadentry: return lisafs_list_print_thread(&entry->thread, context);
    }

    return 0;
}

int lisafs_list_print_directory(lisafs_directrec * _Nonnull directory, void * _Nullable context)
{
    char dirname[33] = {0};
    memcpy(dirname, directory->header.key.name, 32);

    fprintf(stdout, "D %*s- %*hd" "\n",
            -32, dirname,
            5, directory->nodeID);

    return 0;
}

int lisafs_list_print_object(lisafs_objectrec * _Nonnull object, void * _Nullable context)
{
    char objname[33] = {0};
    memcpy(objname, object->header.key.name, 32);

    char c;
    switch (object->header.etype) {
        case linkentry: c = 'L'; break;
        case fileentry: c = 'F'; break;
        case pipeentry: c = 'P'; break;
        case ecentry:   c = 'E'; break;
        default:        c = '?'; break;
    }

    fprintf(stdout, "%c %*s  %*hd %*d" "\n", c,
            -32, objname,
            5, object->sfile,
            10, object->size);

    return 0;
}

int lisafs_list_print_thread(lisafs_threadrec * _Nonnull thread, void * _Nullable context)
{
    char threadname[33] = {0};
    memcpy(threadname, &thread->myName[1], thread->myName[0]);

    if (threadname[0] == 0) {
        threadname[0] = '-';
        threadname[1] = '\0';
    }

    fprintf(stdout, "%s:" "\n", threadname);

    return 0;
}

int lisafs_list_directory_entry_iterator(lisafs_centry * _Nonnull centry, void * _Nullable context)
{
    switch (centry->cetype) {
        case linkentry:
        case fileentry:
        case pipeentry:
        case ecentry:
            return lisafs_list_print_centry(centry, context);

        default:
            // Skip all other types of entry.
            return 0;
    }
}

int lisafs_list_print_centry(lisafs_centry * _Nonnull centry, void * _Nullable context)
{
    char ename[33] = {0};
    memcpy(ename, &centry->name[1], centry->name[0]);

    char c;
    switch (centry->cetype) {
        case linkentry: c = 'L'; break;
        case fileentry: c = 'F'; break;
        case pipeentry: c = 'P'; break;
        case ecentry:   c = 'E'; break;
        default:        c = '?'; break;
    }

    lisafs_longint size;
    if (centry->cetype) {
        size = lisafs_get_sfile_size(image, centry->sfile);
    } else {
        size = 0;
    }
    if (size == -1) return -1;

    fprintf(stdout, "%c %*s  %*hd %*d" "\n", c,
            -32, ename,
            5, centry->sfile,
            10, size);

    return 0;
}


LISAFS_SOURCE_END
