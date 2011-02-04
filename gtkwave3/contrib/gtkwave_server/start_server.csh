#!/bin/csh
killall gtkwave_server ; ipcs -m | grep '^m ' | awk '{print "ipcrm -m "$2}' | sh
setenv GTKWAVE_COMM_DIR /tmp/gtkwave_server_comm
setenv GTKWAVE_SERVER_ID $USER
mkdir -p $GTKWAVE_COMM_DIR
gtkwave_server
