Simulate ACL evaluation.
==
Copyright (C) 2015 V. Atlidakis

COMS W4187 Fall 2015, Columbia University

## Project structure

* Makefile: Makefile
* include/parser.h: Header file for parsing methods
* include/f_ops.h: Header file for file operation methods
* src/parser.c: Implements parser methods
* src/f_ops.c: Implements file operation methods
* src/main.c: Implements main loop for parsing of simulation
* scripts/checkpatch.pl: Format checking script
* tests/test.txt: A demo test file

## Notes - Conventions
Any user appearing either in the user definition portion should have a
dedicated home folder except for one case: when the user tries to create his or
her home folder for the first time.

In the operations portion the commands "[CREATE|ACL] user.group filename"
do not append an ACL "user.group rw" unlike what happens in the user
definition portion (Steve's Mail -- last night -- specifies that files created
in the user definition portion will have an ACL "user.group"). In order for
ACLs to be appended in the operations portion, there needs to be an explicit
rule in a separate line that following the "[CREATE|ACL] user.group filename"
line.

An "ACL user.group filename" command overwrites any existing ACLs of filename --
that is, it does not append new ACLs. (This convention simplifies things and
helps avoid confusion.)

## Error
* Verbose error messages are printed in stderr. To filter them out just redirect
stderr (fd:2) to /dev/null
* Errors for user.group command are printed along with the corresponding line number
for both the user definition and the operations portion.
* Errors for command are printed along with the corresponding command number.
* Note that if an error occurs in "user.group" portion of a ACL or CREATE
  comand, the lines following the error are dropped -- but the ones preceeding
  the error are resulting to valid ACLs.

## TODO
* ACL checks: What superceeds what? Append no permission ACL; where does rw go?
* Format of Errors when OK of the same line appears
*  ACL - and only - in ACL set command. <-- Error

## Run
* make build
* make test (small demo using tests/test.txt)
* cat custom_test_file | make exec (test using custom file)
* make valgrid (memory leak checks)

## Debug
* make DEBUG=1
* make demo (small demo using tests/test.txt)
