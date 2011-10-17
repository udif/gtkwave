#!/usr/bin/perl -w

use strict;

use POSIX qw(mkfifo);

die "usage: gtkwave_client.pl FILENAME SAVEFILE\n"if (@ARGV != 2);

my $GTKWAVE_COMM_DIR= $ENV{'GTKWAVE_COMM_DIR'};

die "Environmental variable GTKWAVE_COMM_DIR is not defined.\n" if (!$GTKWAVE_COMM_DIR);

die "GTKWAVE_COMM_DIR $GTKWAVE_COMM_DIR not writable: won't be able to create FIFOs.\n" 
  if (! -w $GTKWAVE_COMM_DIR);

my $GTKWAVE_SERVER_ID= $ENV{'GTKWAVE_SERVER_ID'};

die "Environmental variable GTKWAVE_SERVER_ID is not defined.\n" if (!$GTKWAVE_SERVER_ID);

my $REQUESTS_NAME= "$GTKWAVE_COMM_DIR/requests.$GTKWAVE_SERVER_ID";
my $REQUEST_NAME= "$GTKWAVE_COMM_DIR/request.$GTKWAVE_SERVER_ID.$$";
my $RESPONSE_NAME= "$GTKWAVE_COMM_DIR/response.$GTKWAVE_SERVER_ID.$$";

unless (-e $REQUESTS_NAME) { mkfifo($REQUESTS_NAME, 0600); }
mkfifo($REQUEST_NAME, 0600) || die "could not create $REQUEST_NAME: $!\n";
mkfifo($RESPONSE_NAME, 0600) || die "could not create $RESPONSE_NAME: $!\n";

print "sending initial request on $REQUESTS_NAME: PID $$\n";
open(REQUESTS, ">$REQUESTS_NAME") || die "could not open $REQUESTS_NAME: $!\n";
print REQUESTS "$$\n";
close REQUESTS;

print "sending search request for $ARGV[0] on $REQUEST_NAME\n";
open(REQUEST, ">$REQUEST_NAME") || die "could not open $REQUEST_NAME: $!\n";
print REQUEST <<END_REQUEST;
--dump
$ARGV[0]
--save
$ARGV[1]
END_REQUEST
close REQUEST;

print "reading response from $RESPONSE_NAME\n";
open(RESPONSE, "$RESPONSE_NAME") || die "could not open $RESPONSE_NAME: $!\n";
while (<RESPONSE>) { print; }
close RESPONSE;

unlink $REQUEST_NAME, $RESPONSE_NAME;





