//  lisafs_main.c
//	Part of LisaFilesystem.
//
//	Copyright © 2026 Base Hit Ventures, LLC. All rights reserved.

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>

#include "image_dc42.h"
#include "lisafs.h"


const char *program_name = NULL;
const char *image_file_path = NULL;
const char *command_name = NULL;

lisafs_image *image = NULL;


int lisafs_dumpblock(int argc, char **argv);
int lisafs_imageinfo(int argc, char **argv);
int lisafs_ls(int argc, char **argv);


typedef int (*lisafs_command_func)(int argc, char **argv);

struct lisafs_command {
    const char * const name;
    lisafs_command_func function;
} lisafs_commands[] = {
    { "dumpblock", lisafs_dumpblock },
    { "imageinfo", lisafs_imageinfo },
    { "ls", lisafs_ls },
    { NULL, NULL },
};


void print_usage(void)
{
    fprintf(stderr, "Usage:" "\n");
    fprintf(stderr, " %s image-file <command> [args]" "\n", program_name);
    fprintf(stderr, " Commands are:" "\n");
    fprintf(stderr, "  dumpblock n" "\t- hex dump raw block n" "\n");
    fprintf(stderr, "  imageinfo"   "\t- print disk image info" "\n");
    fprintf(stderr, "  ls [path]"   "\t- list directory at path or root" "\n");
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

    image = lisafs_image_open(image_file_path);
    if (image == NULL) {
        const char *errstr = strerror(errno);
        fprintf(stderr, "%s: error opening image '%s': %s" "\n", program_name, image_file_path, errstr);
        print_usage();
        return EX_NOINPUT;
    }

    // Perform the subcommand and collect its result.

    int exitcode = (*function)(argc - 2, &argv[2]);

    // Detach the disk image.

    (void) lisafs_image_close(image);

    return exitcode;
}


void print_hex_bytes_line(const uint8_t * const bytes, size_t n)
{
    assert(n <= 16);

    // Print hex (and pad if necessary)

    for (size_t i = 0; i < n; i++) {
        fprintf(stdout, "%02x ", bytes[i]);
    }

    if (n < 16) {
        for (size_t i = 0; i < (16 - n); i++) {
            fprintf(stdout, "   ");
        }
    }

    // Print divider

    fprintf(stdout, "| ");

    // Print text

    for (size_t i = 0; i < n; i++) {
        char ch = (isascii(bytes[i]) && isprint(bytes[i])) ? bytes[i] : '.';
        fprintf(stdout, "%c", ch);
    }

    // Print trailing newline.

    fprintf(stdout, "\n");
}


int lisafs_dumpblock(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "%s: dumpblock: insufficient arguments" "\n", program_name);
        print_usage();
        return EX_USAGE;
    }

    int32_t n = atol(argv[1]);

    // Read the block and its tag.

    uint8_t block[512] = {0};
    uint8_t tag[12] = {0};

    int read_block_err = lisafs_image_read_block(image, n, block, tag);
    if (read_block_err != 0) {
        const char *errstr = strerror(errno);
        fprintf(stderr, "%s: error reading image '%s' block %d: %s" "\n", program_name, image_file_path, n, errstr);
        print_usage();
        return EX_DATAERR;
    }

    // Produce formatted hex output.

    fprintf(stdout, "Block:\t%d" "\n", n);
    fprintf(stdout, "Tag:\t");
    print_hex_bytes_line(tag, 12);
    fprintf(stdout, "Data:" "\n");

    for (int b = 0; b < 0x200; b += 16) {
        fprintf(stdout, "%04x:\t", b);
        print_hex_bytes_line(&block[b], 16);
    }

    return EX_OK;
}


int lisafs_imageinfo(int argc, char **argv)
{
    image_dc42 *raw_image = lisafs_image_get_raw_image(image);
    assert(raw_image != NULL);

    image_dc42_header *header = image_dc42_get_header(raw_image);
    assert(header != NULL);

    const char * const name = image_dc42_get_name(raw_image);
    assert(name != NULL);

    fprintf(stdout, "Name:\t\t"     "'%s'" "\n", name);
    fprintf(stdout, "Type:\t\t"     "DiskCopy 4.2" "\n");
    fprintf(stdout, "Data size:\t"  "%u (%u)" "\n", header->data_size, header->data_size / 512);
    fprintf(stdout, "Tag size:\t"   "%u (%u)" "\n", header->tag_size, header->tag_size / 12);
    fprintf(stdout, "Encoding:\t"   "%s" "\n", image_dc42_get_encoding_name(header->encoding));
    fprintf(stdout, "Format:\t\t"   "0x%02x" "\n", header->format.gcr);
    fprintf(stdout, "Magic:\t\t"    "0x%04x" "\n", header->magic_number);

    return EX_OK;
}


int lisafs_ls(int argc, char **argv)
{
    // TODO: Implement ls command.

    return EX_OK;
}
