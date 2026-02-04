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


// MARK: - extract command

int lisafs_extract(int argc, char * _Nullable * _Nonnull argv)
{
    lisafs_path *lisa_path = NULL;
    uint8_t *buf = NULL;
    int err;

    if (argc < 2) {
        fprintf(stderr, "%s %s: insufficient arguments (%d)" "\n", program_name, command_name, argc);
        print_usage();
        err = EX_USAGE;
        goto error;
    }

    char *path = argv[argc - 1];;
    enum output_style {
        preferred = 0,
        binary,
        text,
    } output_style = preferred;

    for (int i = 1; i < (argc - 1); i++) {
        if (strncmp("-t", argv[i], 2) == 0) {
            output_style = text;
        } else if (strncmp("-b", argv[i], 2) == 0) {
            output_style = binary;
        } else {
            fprintf(stderr, "%s %s: unknown flag '%s'" "\n", program_name, command_name, argv[i]);
            print_usage();
            err = EX_USAGE;
            goto error;
        }
    }

    lisa_path = lisafs_path_from_string(path);
    if (lisa_path == NULL) {
        const char *errstr = strerror(errno);
        fprintf(stderr, "%s %s: error decomposing path '%s': %s" "\n", program_name, command_name, path, errstr);
        err = EX_DATAERR;
        goto error;
    }

    // Determine whether to prefer text or binary based on extension.

    if (output_style == preferred) {
        const char *last_component = lisa_path->components[lisa_path->component_count - 1];
        const char *last_component_extension = strrchr(last_component, '.');
        if (   (last_component_extension != NULL)
            && (strncasecmp(last_component_extension, ".text", 5) == 0))
        {
            output_style = text;
        } else {
            output_style = binary;
        }
    }

    lisafs_fileid lisa_file = lisafs_lookup_sfile(image, lisa_path);
    if (lisa_file == -1) {
        const char *errstr = strerror(errno);
        fprintf(stderr, "%s %s: error accessing file at path '%s': %s" "\n", program_name, command_name, path, errstr);
        err = EX_NOINPUT;
        goto error;
    }

    size_t buf_size = lisafs_get_sfile_size(image, lisa_file);
    if (buf_size == -1) {
        const char *errstr = strerror(errno);
        fprintf(stderr, "%s %s: error finding size of file at path '%s': %s" "\n", program_name, command_name, path, errstr);
        err = EX_OSERR;
        goto error;
    }

    buf = calloc(sizeof(uint8_t), buf_size);
    if (buf == NULL) {
        fprintf(stderr, "%s %s: cannot allocate %zd byte buffer for file at path '%s'" "\n", program_name, command_name, buf_size, path);
        err = EX_OSERR;
        goto error;
    }

    int read_err = lisafs_read_sfile(image, lisa_file, buf, buf_size);
    if (read_err == -1) {
        const char *errstr = strerror(errno);
        fprintf(stderr, "%s %s: error reading file at path '%s': %s" "\n", program_name, command_name, path, errstr);
        err = EX_IOERR;
        goto error;
    }

    char outname[33];
    strncpy(outname, lisa_path->components[lisa_path->component_count - 1], 33);
    char *slash = strchr(outname, '/');
    while (slash != NULL) {
        *slash = '-';
        slash = strchr(&slash[1], '/');
    }

    FILE *outfile = fopen(outname, "wb");
    if (outfile == NULL) {
        const char *errstr = strerror(errno);
        fprintf(stderr, "%s %s: error opening output file '%s': %s" "\n", program_name, command_name, outname, errstr);
        err = EX_CANTCREAT;
        goto error;
    }

    // By default, write out the file contents unadulterated. But if
    // the user passes -t, also convert the file to a modern UNIX-style
    // text format instead of Lisa's weird "tab-compressed 1KB page
    // with 1KB leader page" format.

    if (output_style == binary) {
        size_t items_written = fwrite(buf, buf_size, 1, outfile);
        if (items_written != 1) {
            const char *errstr = strerror(errno);
            fprintf(stderr, "%s %s: error writing output file '%s': %s" "\n", program_name, command_name, outname, errstr);
            err = EX_IOERR;
            goto error;
        }
    } else {
        if ((buf_size < 1024) || ((buf_size % 1024) != 0)) {
            fprintf(stderr, "%s %s: file '%s' size %zd is wrong for text" "\n", program_name, command_name, path, buf_size);
            err = EX_DATAERR;
            goto error;
        }

        // Start at the 1024th byte, skipping the first "page."

        bool saw_tabcomp = false;
        for (size_t i = 1024; i < buf_size; i++) {
            uint8_t ch = buf[i];
            switch (ch) {
                case 0x00:
                case 0xff:
                    // Skip NUL and DEL characters, they're padding and EOF.
                    break;

                case 0x0D:
                    // Turn CR into LF.
                    fputc('\n', outfile);
                    break;

                case 0x10:
                    // Set a flag when we see a "tab compression" char.
                    saw_tabcomp = true;
                    break;

                default:
                    if (saw_tabcomp) {
                        // Insert spaces.
                        for (int j = 0; j < (ch - 0x20); j++) {
                            fputc(' ', outfile);
                        }
                        saw_tabcomp = false;
                    } else {
                        fputc(ch, outfile);
                    }
                    break;
            }
        }
    }

    fclose(outfile);
    outfile = NULL;

    lisafs_path_free(lisa_path);
    lisa_path = NULL;

    free(buf);
    buf = NULL;

    return EX_OK;

error:
    lisafs_path_free(lisa_path);
    lisa_path = NULL;

    free(buf);
    buf = NULL;

    return err;
}


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
        fprintf(stderr, "%s %s: error iterating %s: %s" "\n", program_name, command_name, image_file_path, errstr);
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

    fprintf(stdout, "-%s:" "\n", threadname);

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
