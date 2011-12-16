## -*- makefile -*-
##

xdgdatadir=@XDGDATADIR@
desktopdir=$(xdgdatadir)/applications
desktop_DATA = \
	x-gtkwave-extension-ae2.desktop x-gtkwave-extension-aet.desktop \
	x-gtkwave-extension-evcd.desktop x-gtkwave-extension-fst.desktop \
	x-gtkwave-extension-ghw.desktop x-gtkwave-extension-gtkw.desktop \
	x-gtkwave-extension-lx2.desktop x-gtkwave-extension-lxt2.desktop \
	x-gtkwave-extension-lxt.desktop x-gtkwave-extension-vcd.desktop \
	x-gtkwave-extension-vzt.desktop

if FDO_MIME
install-data-hook:
	$(UPDATE_DESKTOP_DATABASE)

uninstall-hook:
	$(UPDATE_DESKTOP_DATABASE)
endif
