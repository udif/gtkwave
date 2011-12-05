#!/bin/sh

#
# this assumes that gtkwave --prefix is the
# same directory hierarchy that gtkwave via
# jhbuild is installed to
#
# make sure you are in jhbuild-shell and
# that jhbuild is in your path
#

# ~/.local/bin/ige-mac-bundler gtkwave.bundle
~/.local/bin/gtk-mac-bundler gtkwave.bundle

#
# this is a bit of a hack as it seems ige-mac-bundler
# should already do this...
#
gdk-pixbuf-query-loaders | \
sed 's#/Users/.*loaders/#@executable_path/../Resources/lib/gdk-pixbuf-2.0/loaders/#' \
> ~/Desktop/gtkwave.app/Contents/Resources/etc/gtk-2.0/gdk-pixbuf.loaders

