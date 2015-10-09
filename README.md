Simulate ACL evaluation.
==
Copyright (C) 2015 V. Atlidakis

COMS W4187 Fall 2015, Columbia University

## Project structure

Makefile:
include/parser.h:
include/f_ops.h:
src/parser.c:
src/f_ops.c:
src/main.c:
scripts/checkpatch.pl: Format checking script

## Notes - Conventions
Spaces at the beginning of file names are stripped, if more than one.

Attempting to create a file that already exists, produces an error.

Parsing is completely aborted upon errors caused by malformed lines.


## Questions
-What happens if user.group is recreated in another line for the same file?
-For sure I do delete, create stuff, right?

## TODO
-Review ACL tests
-ACL when no home folder for users should fail
-Cannot create ACL for user without home folder: This expains why I need
seperate lines for evaluasting "uesr.group" commandsd of user definition section
-Cannot delete parent folders with kids on top.
-home is not writable -  user should be able to create home folder
check ACLs on all previous components every time and assert READ?
-COMMAND SECTION ONLY: Inherits ACLs from parents when creted with NULL ACLs
-COMMAND SECTION ONLY: ACLs command overwrites the ACLs.
-Valgrid test
-Print errors commands section
-Implement COMMANDS:
.READ
.WRITE
.ACL
.CREATE
.DELETE
-we at least need some orerations section
