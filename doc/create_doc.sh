#!/bin/bash

asciidoc -a toc2 dlt_user_manual.txt
asciidoc -a toc2 dlt_design_specification.txt
asciidoc -a toc2 dlt_cheatsheet.txt
asciidoc -a toc2 dlt_filetransfer.txt

asciidoc -a toc2 dlt-daemon.1.txt
a2x --doctype manpage --format manpage dlt-daemon.1.txt
asciidoc -a toc2 dlt.conf.5.txt
a2x --doctype manpage --format manpage dlt.conf.5.txt
asciidoc -a toc2 dlt-convert.1.txt
a2x --doctype manpage --format manpage dlt-convert.1.txt
asciidoc -a toc2 dlt-receive.1.txt
a2x --doctype manpage --format manpage dlt-receive.1.txt
asciidoc -a toc2 dlt-system.1.txt
a2x --doctype manpage --format manpage dlt-system.1.txt
asciidoc -a toc2 dlt-system.conf.5.txt
a2x --doctype manpage --format manpage dlt-system.conf.5.txt

asciidoc -a toc2 dlt_book.txt

cd ..

asciidoc -a toc2 README.txt
asciidoc -a toc2 INSTALL.txt
asciidoc -a toc2 ReleaseNotes.txt

