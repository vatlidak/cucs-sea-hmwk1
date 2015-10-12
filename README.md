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
Attempting to create a file that already exists, produces an error.

Parsing is completely aborted upon errors caused by malformed lines.

Any user appearing either in the user definitioin portion or in the operations
should have a dedicated home folder except for one case: when the user tries
to create his or her home folder for the first time.

Since to every file created in the user definition portion  we append an "*.* r"
ACL, any "READ user.group filename command" for a filename in the operations
portion will always be valid -- unless an explicit ACL update changing the ACLs
of a filename preceeds the READ command.

Also regarding the following two sentences: "A file that does not belong
to anyone will allow only reads but no writes" and "if you create a file with
NULL ACL, it inherits the ACL of its parent".  The only two files that possibly
do not belong to anyone will be "/tmp" and "/home".  Any other file will be
created either in the user definition portion (line: user.group filename) or
in the operations portion (line: CREATE user.group filename -- "followed by a
mandatory ACL for the file"), and therefore in the current spec it is impossible
to create a file that belongs to noone.

In the operations portion the commands "[CREATE|ACL] user.group filename" 
do not append an ACL "user.group rw" unlike what happens in the user
definition portion (Steve's Mail -- last night -- specifies that files created
in the user definition portion will have an ACL "user.group"). In order for ACLs
to be appended in the operations portion, there needs to be an explicit rule in
a seperate line that following the "[CREATE|ACL] user.group filename" line.


An "ACL user.group filename" command overwrites any existing ACLs of filename --
that is, it does not append new ACLs. (This convention simplifies things and
helos avoid confusion.)

## TODO
-ACL checks: Order & what superceeds what?
-Append the no permission ACL
-SPECs: parsing of line ok when all components found
-SPECs: parsing of line ok when all components found
-Format of Errors when OK of the same line appears

## Tests
valgrind
checkpatch
