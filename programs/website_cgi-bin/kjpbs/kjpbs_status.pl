#!/usr/bin/perl

use strict;
use IO::Socket;

my $socket = IO::Socket::INET->new( PeerAddr => 'vnc.purebiblesearch.com',
                                    PeerPort => '80',
                                    Proto    => 'tcp',
                                    Type     => SOCK_STREAM );

print "Content-type: text/html\n\n";

if ($socket) {
  close($socket);
  print "(Status: <b>Online And Ready!</b>)";
} else {
  print "(Status: <b>OFFLINE! Try Back Later</b>)";
}
exit;
