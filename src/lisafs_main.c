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
    const char * const example;
    const char * const description;
} lisafs_commands[] = {
    { "extract",    lisafs_extract,     "extract [-t|-b] path", "extract file at Lisa path, text or binary" },
    { "list",       lisafs_list,        "list",                 "list all files on the image" },

#if LISAFS_ENABLE_DEV_COMMANDS
    { "dumpblock",  lisafs_dumpblock,   "dumpblock n",          "hex dump raw block n" },
    { "dumppage",   lisafs_dumppage,    "dumppage n",           "hex dump of raw page n" },
    { "fsinfo",     lisafs_fsinfo,      "fsinfo",               "print filesystem info" },
    { "imageinfo",  lisafs_imageinfo,   "imageinfo",            "print disk image info" },
    { "sfextract",  lisafs_sfextract,   "sfextract n",          "extract sfile n (using hints)" },
    { "sflist",     lisafs_sflist,      "sflist [-l]",          "list sfiles (using hints)", },
#endif

    { NULL, NULL },
};


void print_usage(void)
{
    fprintf(stderr, "\n");
    fprintf(stderr, "Usage:" "\n");
    fprintf(stderr, " %s image-file <command> [args]" "\n", program_name);
    fprintf(stderr, " Commands are:" "\n");

    // Go through the commands twice to produce nicely-formatted columns.

    int max_name = 0, max_example = 0, max_description = 0;

    struct lisafs_command *command = NULL;
    const size_t lisafs_commands_count = sizeof(lisafs_commands) / sizeof(struct lisafs_command);

    for (int i = 0; i < lisafs_commands_count; i++) {
        command = &lisafs_commands[i];
        if (command->name == NULL) break;

        size_t name_len = strlen(command->name);
        size_t example_len = strlen(command->example);
        size_t description_len = strlen(command->description);

        if (name_len > max_name) max_name = (int)name_len;
        if (example_len > max_example) max_example = (int)example_len;
        if (description_len > max_description) max_description = (int)description_len;
    }

    for (int i = 0; i < lisafs_commands_count; i++) {
        command = &lisafs_commands[i];
        if (command->name == NULL) break;

        fprintf(stderr, "  %*s  %*s  %*s" "\n",
                -max_name, command->name,
                -max_example, command->example,
                -max_description, command->description);
    }
}


int main(int argc, char **argv)
{
    // Process whole-program arguments and determine the subcommand to use.

    program_name = argv[0];

    if (argc < 3) {
        fprintf(stderr, "%s: Insufficient arguments (%d)." "\n", program_name, argc);
        print_usage();
        return EX_USAGE;
    }

    image_file_path = argv[1];
    command_name = argv[2];

    const char *image_file_path_extension = strrchr(image_file_path, '.');
    if (   (image_file_path_extension == NULL)
        || (strncasecmp(image_file_path_extension, ".dc42", 5) != 0))
    {
        fprintf(stderr, "%s: Only DiskCopy 4.2 (.dc42) images are supported at this time." "\n", program_name);
        print_usage();
        return EX_USAGE;
    }

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
