#!/bin/sh

#
# this assumes that gtkwave --prefix is the
# same directory hierarchy that gtkwave via
# jhbuild is installed to
#
# make sure you are in jhbuild-shell and
# that jhbuild is in your path
#

~/.local/bin/ige-mac-bundler gtkwave.bundle
