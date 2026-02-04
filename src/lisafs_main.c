//  lisafs_main.c
//	Part of lisafs.
//
//	Copyright © 2026 Christopher M. Hanson. All rights reserved.
//  See file COPYING for details.

#include "lisafs_main.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sysexits.h>

#include "lisafs.h"
#include "lisafs_commands.h"

#if LISAFS_ENABLE_DEV_COMMANDS
#include "lisafs_dev_commands.h"
#endif


LISAFS_SOURCE_BEGIN


const char *program_name = NULL;
const char *image_file_path = NULL;
const char *command_name = NULL;

lisafs_image *image = NULL;


typedef int (*lisafs_command_func)(int argc, char * _Nullable * _Nonnull argv);


struct lisafs_command {
    const char * const name;
    lisafs_command_func function;
    const char * const description;
} lisafs_commands[] = {
    { "extract",    lisafs_extract,     "extract path""\t- extract the file at the given Lisa path" },
    { "list",       lisafs_list,        "list" "\t"   "\t- list all files on the image" },

#if LISAFS_ENABLE_DEV_COMMANDS
    { "dumpblock",  lisafs_dumpblock,   "dumpblock n" "\t- hex dump raw block n" },
    { "dumppage",   lisafs_dumppage,    "dumppage n"  "\t- hex dump of raw page n" },
    { "fsinfo",     lisafs_fsinfo,      "fsinfo"      "\t- print filesystem info" },
    { "imageinfo",  lisafs_imageinfo,   "imageinfo"   "\t- print disk image info" },
    { "sfextract",  lisafs_sfextract,   "sfextract n" "\t- extract sfile n (using hints)" },
    { "sflist",     lisafs_sflist,      "sflist [-l]" "\t- list sfiles (using hints)", },
#endif

    { NULL, NULL },
};


void print_usage(void)
{
    fprintf(stderr, "Usage:" "\n");
    fprintf(stderr, " %s image-file <command> [args]" "\n", program_name);
    fprintf(stderr, " Commands are:" "\n");

    struct lisafs_command *command = NULL;
    const size_t lisafs_commands_count = sizeof(lisafs_commands) / sizeof(struct lisafs_command);
    for (int i = 0; i < lisafs_commands_count; i++) {
        command = &lisafs_commands[i];
        if (command->name == NULL) break;

        fprintf(stderr, "  %s" "\n", command->description);
    }
}


int main(int argc, char **argv)
{
    // Process whole-program arguments and determine the subcommand to use.

    program_name = argv[0];

    if (argc < 3) {
        fprintf(stderr, "%s: Insufficient arguments (%d)." "\n", program_name, argc);
        fprintf(stderr, "\n");
        print_usage();
        return EX_USAGE;
    }

    image_file_path = argv[1];
    command_name = argv[2];

    // Attach the disk image.

    image = lisafs_open(image_file_path);
    if (image == NULL) {
        const char *errstr = strerror(errno);
        fprintf(stderr, "%s: error opening image '%s': %s" "\n", program_name, image_file_path, errstr);
        print_usage();
        return EX_NOINPUT;
    }

    // Find and perform the subcommand and collect its result.

    struct lisafs_command *command = NULL;
    const size_t lisafs_commands_count = sizeof(lisafs_commands) / sizeof(struct lisafs_command);
    for (int i = 0; i < lisafs_commands_count; i++) {
        command = &lisafs_commands[i];
        if (command->name == NULL) {
            // At the end, no command.
            command = NULL;
            break;
        } else if (strcmp(command_name, command->name) == 0) {
            // Use this command.
            break;
        } else {
            // Try the next one.
        }
    }

    if (command == NULL) {
        fprintf(stderr, "%s: Unknown command '%s'" "\n", program_name, command_name);
        print_usage();
        return EX_USAGE;
    }

    lisafs_command_func function = command->function;

    int exitcode = (*function)(argc - 2, &argv[2]);

    // Detach the disk image.

    (void) lisafs_close(image);

    return exitcode;
}


LISAFS_SOURCE_END
