# lisafs

The `lisafs` tool allows read-only access to Lisa filesystems on Disk
Copy 4.2 disk images.

It currently implements two subcommands, `list` and `extract`, as well
as a number of "development-time" commands for assisting with
understanding the data structures used by the Lisa filesystem.

Both Lisa OS 2.0 (flat) and 3.0 (hierarchical) filesystems are
supported, and Lisa OS 1.0 filesystems *may* work as well though are
currently untested.


## Usage

To use `lisafs`, build it and then invoke it with your disk image as the
first argument, followed by the desired subcommand and any arguments.
For example:

```sh
$ lisafs 688-0036_Pascal_Workshop_3.0_1.dc42 list
Volume (3.0): Workshop Pascal 1 3.0
-:
F apin/syslib.obj                       5      89600
F font.heur                            24       1536
F font.lib                              6       5746
F installwsphrase                       7      17965
F Intrinsic.Lib                         8       1536
F iospaslib.obj                         9      24576
F system.bt_Priam Disk                 13      11264
F system.bt_Profile                    14      11776
F system.cdd                           15       1536
F system.cd_2 Port Card                16       1024
F system.cd_Archive Tape               17       4096
F system.cd_Console                    18       5120
F system.cd_Priam Card                 19       2048
F system.cd_Priam Disk                 20       3584
F system.cd_Profile                    21       5632
F system.cd_Sony                       22       3584
F system.IUDirectory                   25       8192
F system.lld                           10      10240
F system.os                            11     141824
F system.shell                         12      17408
F SYSTEM.UNPACK                        23       1024
```


### list

The `list` subcommand takes no arguments, and prints the volume name and
version of the filesystem on the supplied disk image followed
recursively by the contents of each directory on the volume, starting
with `-` (which his how Lisa spells the root directory).

The contents of a directory are distinguished between files and
directories by a leading F or D followed by their filename. The third
column contains the file or directory ID, and the final column contains
the size of the file.


### extract

The `extract` subcommand takes a Lisa filesystem path and copies the
file at that path out of the image. For example, this command:

```sh
$ lisafs 688-0036_Pascal_Workshop_3.0_1.dc42 extract -system.os
```

will extract the binary file at `-system.os` from the Workshop Pascal
3.0 disk #1 image to a local `system.os` binary file.

Paths on Lisa use `-` as their component separator; supplying a leading
`-` to indicate the root directory is optional. Since `/` is a valid
filename character on Lisa but `-` is not, extracting a file will swap
`/` and `-`; e.g. the file at Lisa path `-apin/syslib.obj` will be
extracted to a file named `apin-syslib.obj`.

Paths on Lisa are also case-insensitive. Currently, the case used to
specify a path is what's used in creating the output file, not the case
used in the Lisa filesystem.


## Lisa Text Files

Text files on Lisa use carriage return rather than linefeed characters
as their line separator, and are also divided into 1KB "pages" padded
with 0x00 bytes, with a leading metadata page and optional trailing 0xFF
byte after their last character.

Furthermore, text files on Lisa use a space-compression mechanism
whereby a sequence of spaces can be introduced by an ASCII 16 byte
followed by a the number of spaces to generate plus 32. (That is, 8
spaces is represented by ASCII 16 followed by ASCII 40.)

To accommodate these on modern UNIX-derived platforms, the `extract`
subcommand will transparently convert files whose name ends in `.text`
(case-insenstive) to drop the leading page, elide any 0x00 or 0xFF
padding bytes, and expand any compressed space sequences. The bytes are
otherwise uninterpreted.

This behavior can be controlled by passing the `-b` or `-t` flag to the
`extract` subcommand: Passing `-b` (for "binary") will cause the file to
be extracted as-is, while passing `-t` will cause the file to be treated
as text to convert even if it doesn't have a `.text` extension.
