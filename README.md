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

An error caused by a line in the user definition section aborts further parsing.

## Questions
-what happens if I recreate the file that exists in another line?
