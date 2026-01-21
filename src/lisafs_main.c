//  lisafs_main.c
//	Part of LisaFilesystem.
//
//	Copyright © 2026 Base Hit Ventures, LLC. All rights reserved.

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>

#include "lisafs.h"


const char *program_name;
const char *image_file_path;
const char *command_name;


int lisafs_ls(int argc, char **argv);


typedef int (*lisafs_command_func)(int argc, char **argv);

struct lisafs_command {
    const char *name;
    lisafs_command_func function;
} lisafs_commands[] = {
    { "ls", lisafs_ls },
    { NULL, NULL },
};


void print_usage(void)
{
    fprintf(stderr, "Usage:" "\n");
    fprintf(stderr, " %s image-file <command> [args]" "\n", program_name);
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
        fprintf(stderr, "%s: Unknown command '%s" "\n", program_name, command_name);
        print_usage();
        return EX_USAGE;
    }

    lisafs_command_func function = command->function;

    // Attach the disk image.

    // TODO: Implement image attach.

    // Perform the subcommand and return its result.

    return (*function)(argc - 2, &argv[2]);
}


int lisafs_ls(int argc, char **argv)
{
    // TODO: Implement ls command.

    return EX_OK;
}
