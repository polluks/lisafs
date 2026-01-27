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
int lisafs_dumppage(int argc, char **argv);
int lisafs_fsinfo(int argc, char **argv);
int lisafs_imageinfo(int argc, char **argv);
int lisafs_sfextract(int argc, char **argv);


typedef int (*lisafs_command_func)(int argc, char **argv);

struct lisafs_command {
    const char * const name;
    lisafs_command_func function;
    const char * const description;
} lisafs_commands[] = {
    { "dumpblock",  lisafs_dumpblock,   "dumpblock n" "\t- hex dump raw block n" },
    { "dumppage",   lisafs_dumppage,    "dumppage n"  "\t- hex dump of raw page n" },
    { "fsinfo",     lisafs_fsinfo,      "fsinfo"      "\t- print filesystem info" },
    { "imageinfo",  lisafs_imageinfo,   "imageinfo"   "\t- print disk image info" },
    { "sfextract",  lisafs_sfextract,   "sfextrct n"  "\t- extract sfile n (using labels)" },
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


int lisafs_dumppage(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "%s: dumppage: insufficient arguments" "\n", program_name);
        print_usage();
        return EX_USAGE;
    }

    int32_t n = atol(argv[1]);

    // Read the page and its label.

    lisafs_page page = {0};

    int read_page_err = lisafs_image_read_page(image, n, &page);
    if (read_page_err != 0) {
        const char *errstr = strerror(errno);
        fprintf(stderr, "%s: error reading image '%s' block %d: %s" "\n", program_name, image_file_path, n, errstr);
        print_usage();
        return EX_DATAERR;
    }

    // Pretty-print the label.

    fprintf(stdout, "Page:\t"       "%d"    "\n", n);
    fprintf(stdout, "Label:"                "\n");
    fprintf(stdout, "  version:\t"  "%hd"   "\n", page.label.version);
    fprintf(stdout, "  flags:\t"    "0x%04hx" "\n", page.label.flags);
    fprintf(stdout, "  fileid:\t"   "%hd"   "\n", page.label.fileid);
    fprintf(stdout, "  dataused:\t" "%hd"   "\n", page.label.dataused);
    fprintf(stdout, "  abspage:\t"  "%d"    "\n", page.label.abspage);
    fprintf(stdout, "  relpage:\t"  "%d"    "\n", page.label.relpage);
    fprintf(stdout, "  fwdlink:\t"  "%d"    "\n", page.label.fwdlink);
    fprintf(stdout, "  bkwdlink:\t" "%d"    "\n", page.label.bkwdlink);

    // Produce formatted hex output.

    fprintf(stdout, "Data:" "\n");

    for (int b = 0; b < 512; b += 16) {
        fprintf(stdout, "%04x:\t", b);
        print_hex_bytes_line(&page.data[b], 16);
    }

    return EX_OK;
}

const char * timestr(lisafs_timestamp timestamp)
{
    static struct tm tm;
    static char buf[26];

    if (timestamp == 0) return "never";

    time_t time = lisafs_timestamp_to_time_t(timestamp);
    gmtime_r(&time, &tm);
    asctime_r(&tm, buf);
    char *newline = strchr(buf, '\n');
    if (newline) *newline = '\0';
    return buf;
}

#define OFFSET(a,b) ((void *)&(a->b) - (void *)a)
#define DEFPRINT(s,m, f) \
    fprintf(stdout, "" #m ":\t" f " (%td)" "\n", s->m, OFFSET(s, m))

int lisafs_fsinfo(int argc, char **argv)
{
    lisafs_mf_loader_loader_header *header = lisafs_image_get_loader_header(image);
    assert(header != NULL);

    fprintf(stdout, "*** Loader Loader Header (block 0)" "\n");
    fprintf(stdout, "JMP:\t\t"          "0x%08x (%td)"  "\n", header->jmp, OFFSET(header, jmp));
    fprintf(stdout, "Boot ID:\t"        "0x%04hx (%td)" "\n", header->boot_id, OFFSET(header, boot_id));
    fprintf(stdout, "Loader Vers:\t"    "0x%04hx (%td)" "\n", header->ldr_version, OFFSET(header, ldr_version));
    fprintf(stdout, "Global Size:\t"    "0x%04hx (%td)" "\n", header->globalsize, OFFSET(header, globalsize));
    fprintf(stdout, "Code Size:\t"      "0x%04hx (%td)" "\n", header->codesize, OFFSET(header, codesize));
    fprintf(stdout, "PC Offset:\t"      "0x%04hx (%td)" "\n", header->pc_offset, OFFSET(header, pc_offset));
    fprintf(stdout, "Block 0 Offset:\t" "%d (%td)"      "\n", header->fs_block0, OFFSET(header, fs_block0));
    fprintf(stdout, "\n");

    lisafs_mddf *mddf = lisafs_image_get_mddf(image);
    assert(mddf != NULL);

    fprintf(stdout, "*** Media Description Data File (block %d)" "\n", header->fs_block0);
    fprintf(stdout, "Version:\t\t"          "%s (%td)" "\n", lisafs_fsversion_string(mddf->fsversion), OFFSET(mddf, fsversion));
    fprintf(stdout, "Volume ID:\t\t"        "0x%08x:%08x (%td)""\n", mddf->volid.a, mddf->volid.b, OFFSET(mddf, volid));
    fprintf(stdout, "Volume Number:\t\t"    "0x%04hx (%td)" "\n", mddf->volnum, OFFSET(mddf, volnum));
    fprintf(stdout, "Volume Name:\t\t"      "'%s' (%td)" "\n", lisafs_image_get_volname(image), OFFSET(mddf, volname));
    fprintf(stdout, "Volume Password:\t"    "'%s' (%td)" "\n", lisafs_image_get_password(image), OFFSET(mddf, password));
    DEFPRINT(mddf, init_machine_id, "%d");
    DEFPRINT(mddf, master_machine_id, "%d");
    fprintf(stdout, "Date Created:\t\t"     "%s (%td)" "\n", timestr(mddf->DT_created), OFFSET(mddf, DT_created));
    fprintf(stdout, "Date Copy Created:\t"  "%s (%td)" "\n", timestr(mddf->DT_copy_created), OFFSET(mddf, DT_copy_created));
    fprintf(stdout, "Date Copied:\t\t"      "%s (%td)" "\n", timestr(mddf->DT_copied), OFFSET(mddf, DT_copied));
    fprintf(stdout, "Date Scavenged:\t\t"   "%s (%td)" "\n", timestr(mddf->DT_scavenged), OFFSET(mddf, DT_scavenged));
    DEFPRINT(mddf, copy_thread, "%d");
    DEFPRINT(mddf, firstblock, "%d");
    DEFPRINT(mddf, lastblock, "%d");
    DEFPRINT(mddf, lastfspage, "%d");
    DEFPRINT(mddf, blockcount, "%d");
    DEFPRINT(mddf, blocksize, "%hd");
    DEFPRINT(mddf, datasize, "%hd");
    DEFPRINT(mddf, cluster_size, "%hd");
    fprintf(stdout, "MDDF Address:\t\t"     "%u (%td)" "\n", mddf->MDDFaddr, OFFSET(mddf, MDDFaddr));
    fprintf(stdout, "MDDF Size:\t\t"        "%hu (%td)" "\n", mddf->MDDFsize, OFFSET(mddf, MDDFsize));
    fprintf(stdout, "Bitmap Address:\t\t"   "%u (%td)" "\n", mddf->bitmap_addr, OFFSET(mddf, bitmap_addr));
    fprintf(stdout, "Bitmap Size:\t\t"      "%u bits (%td)" "\n", mddf->bitmap_size, OFFSET(mddf, bitmap_size));
    fprintf(stdout, "Bitmap Bytes:\t\t"     "%hu (%td)" "\n", mddf->bitmap_bytes, OFFSET(mddf, bitmap_bytes));
    fprintf(stdout, "Bitmap Pages:\t\t"     "%hu (%td)" "\n", mddf->bitmap_pages, OFFSET(mddf, bitmap_pages));
    fprintf(stdout, "S-list Address:\t\t"   "%u (%td)" "\n", mddf->slist_addr, OFFSET(mddf, slist_addr));
    fprintf(stdout, "S-list Packing:\t\t"   "%hu (%td)" "\n", mddf->slist_packing, OFFSET(mddf, slist_packing));
    fprintf(stdout, "S-list Blocks:\t\t"    "%hu (%td)" "\n", mddf->slist_block_count, OFFSET(mddf, slist_block_count));
    fprintf(stdout, "First File:\t\t"       "%hu (%td)" "\n", mddf->first_file, OFFSET(mddf, first_file));
    fprintf(stdout, "Empty File:\t\t"       "%hu (%td)" "\n", mddf->empty_file, OFFSET(mddf, empty_file));
    fprintf(stdout, "Max Files:\t\t"        "%hu (%td)" "\n", mddf->maxfiles, OFFSET(mddf, maxfiles));
    DEFPRINT(mddf, hintsize, "%hd");
    DEFPRINT(mddf, leader_offset, "%hd");
    DEFPRINT(mddf, leader_pages, "%hd");
    DEFPRINT(mddf, flabel_offset, "%hd");
    DEFPRINT(mddf, unusedi1, "%hd");
    DEFPRINT(mddf, map_offset, "%hd");
    DEFPRINT(mddf, map_size, "%hd");
    fprintf(stdout, "File Count:\t\t"       "%hu (%td)" "\n", mddf->filecount, OFFSET(mddf, filecount));
    DEFPRINT(mddf, freestart, "%d");
    DEFPRINT(mddf, unusedl1, "%d");
    DEFPRINT(mddf, freecount, "%d");
    fprintf(stdout, "Root S-file:\t\t"      "%hu (%td)" "\n", mddf->rootsnum, OFFSET(mddf, rootsnum));
    fprintf(stdout, "Root Maximum:\t\t"     "%hu entries (%td)" "\n", mddf->rootmaxentries, OFFSET(mddf, rootmaxentries));
    DEFPRINT(mddf, mountinfo, "%hd");
    fprintf(stdout, "Overmount Stamp:\t"    "0x%08x:%08x (%td)""\n", mddf->overmount_stamp.a, mddf->overmount_stamp.b, OFFSET(mddf, overmount_stamp));
    fprintf(stdout, "Pmem Machine ID:\t"    "%d (%td)" "\n", mddf->pmem_id, OFFSET(mddf, pmem_id));
    for (int i = 0; i < 32; i++) {
        if ((i % 8) == 0) fprintf(stdout, "pmem[%d]:\t\t", i);
        fprintf(stdout, "0x%04hx ", mddf->pmem[i]);
        if ((i % 8) == 7) fprintf(stdout, "(%td)\n", OFFSET(mddf, pmem[i - 7]));
    }
    DEFPRINT(mddf, vol_scavenged, "%hd");
    DEFPRINT(mddf, tbt_copied, "%hd");
    DEFPRINT(mddf, smallmap_offset, "%hd");
    DEFPRINT(mddf, hentry_offset, "%hd");
    fprintf(stdout, "Backup Volume ID:\t"   "0x%08x:%08x (%td)""\n", mddf->backup_volid.a, mddf->backup_volid.b, OFFSET(mddf, backup_volid));
    DEFPRINT(mddf, flabel_size, "%hd");
    DEFPRINT(mddf, fs_overhead, "%hd");
    DEFPRINT(mddf, result_scavenge, "%hd");
    DEFPRINT(mddf, boot_code, "%hd");
    DEFPRINT(mddf, boot_environ, "%hd");
    DEFPRINT(mddf, oem_id, "%d");
    DEFPRINT(mddf, root_page, "%d");
    DEFPRINT(mddf, tree_depth, "%hd");
    DEFPRINT(mddf, node_id, "%hd");
    DEFPRINT(mddf, vol_seq_no, "%hd");
    DEFPRINT(mddf, vol_mounted, "%hd");

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


int lisafs_sfextract(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "%s: sfextract: insufficient arguments" "\n", program_name);
        print_usage();
        return EX_USAGE;
    }

    long input_fileid = atol(argv[1]);
    if ((input_fileid < 0) || (input_fileid > 32767)) {
        fprintf(stderr, "%s: sfextract: invalid file ID %ld" "\n", program_name, input_fileid);
        print_usage();
        return EX_USAGE;
    }
    const lisafs_fileid fileid = input_fileid;

    lisafs_mddf *mddf = lisafs_image_get_mddf(image);
    assert(mddf != NULL);

    // Do a linear search through the volume to find the first page of
    // the requested s-file.

    lisafs_page page;
    FILE *output = NULL;

    const lisafs_paddr pcount = mddf->lastfspage;
    for (lisafs_paddr i = 0; i < pcount; i++) {
        int read_err = lisafs_image_read_page(image, i, &page);
        if (read_err == -1) goto error;

        // If we found the first page, use its data to create the output
        // file and then record where the next page is.

        if ((page.label.fileid == fileid) && (page.label.bkwdlink == -1)) {
            char sfname[32] = {0};
            snprintf(sfname, 32, "sf.%hd", fileid);
            output = fopen(sfname, "wb");
            if (output == NULL) goto error;

            size_t written = fwrite(page.data, page.label.dataused, 1, output);
            if (written != 1) goto error;

            break;
        }
    }

    if (output != NULL) {
        // Go through all the pages in order and write them to the output.

        while (page.label.fwdlink != -1) {
            int read_err = lisafs_image_read_page(image, page.label.fwdlink, &page);
            if (read_err == -1) goto error;

            size_t written = fwrite(page.data, page.label.dataused, 1, output);
            if (written != 1) goto error;
        }

        fclose(output);
        output = NULL;
    } else {
        fprintf(stderr, "%s: sfextract: file ID %d not found" "\n", program_name, fileid);
    }

    return EX_OK;

error:
    return EX_DATAERR;
}
