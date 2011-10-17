#!/bin/csh
setenv GTKWAVE_COMM_DIR /tmp/gtkwave_server_comm
setenv GTKWAVE_SERVER_ID $USER
gtkwave_client.pl /home/bybell/gtkwave/gtkwave3/examples/des.vzt /home/bybell/gtkwave/gtkwave3/examples/des.sav

