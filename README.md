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
Any user appearing either in the user definitioin portion or in the operations
should have a dedicated home folder except for one case: when the user tries
to create his or her home folder for the first time.

Since to every file created in the user definition portion  we append an "*.* r"
ACL, any "READ user.group filename command" for a filename in the operations
portion will always be valid -- unless an explicit ACL update changing the ACLs
of a filename preceeds the READ command.

In the operations portion the commands "[CREATE|ACL] user.group filename" 
do not append an ACL "user.group rw" unlike what happens in the user
definition portion (Steve's Mail -- last night -- specifies that files created
in the user definition portion will have an ACL "user.group"). In order for ACLs
to be appended in the operations portion, there needs to be an explicit rule in
a seperate line that following the "[CREATE|ACL] user.group filename" line.

An "ACL user.group filename" command overwrites any existing ACLs of filename --
that is, it does not append new ACLs. (This convention simplifies things and
helps avoid confusion.)

To get rid of additional error messages (which are generaly useful) redirect
stderr to /dev/null/

## TODO
* ACL checks: Order & what superceeds what?
* Append the no permission ACL
* Format of Errors when OK of the same line appears

## Test
Type:
* make
* make valgrid (memory leak checks)
* cat tests/test.txt | ./main 2>/dev/null (small demo)
