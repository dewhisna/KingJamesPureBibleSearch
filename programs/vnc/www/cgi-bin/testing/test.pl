#!/usr/bin/perl

use strict;
use IO::Socket;
use CGI qw/:standard/;

my ($socket, $status);

# Add the services to check here:
# [ SERVICE NAME, IP ADDRESS, PORT, USERID, STATUS ]

my @services = (
  [ 'KJPBS User 01', 'vnc.purebiblesearch.com', '6080', 'pbsuser01', '0' ],
  [ 'KJPBS User 02', 'vnc.purebiblesearch.com', '6081', 'pbsuser02', '0' ],
  [ 'KJPBS User 03', 'vnc.purebiblesearch.com', '6082', 'pbsuser03', '0' ],
  [ 'KJPBS User 04', 'vnc.purebiblesearch.com', '6083', 'pbsuser04', '0' ],
  [ 'KJPBS User 05', 'vnc.purebiblesearch.com', '6084', 'pbsuser05', '0' ],
  [ 'KJPBS User 06', 'vnc.purebiblesearch.com', '6085', 'pbsuser06', '0' ],
  [ 'KJPBS User 07', 'vnc.purebiblesearch.com', '6086', 'pbsuser07', '0' ],
  [ 'KJPBS User 08', 'vnc.purebiblesearch.com', '6087', 'pbsuser08', '0' ],
  [ 'KJPBS User 09', 'vnc.purebiblesearch.com', '6088', 'pbsuser09', '0' ],
  [ 'KJPBS User 10', 'vnc.purebiblesearch.com', '6089', 'pbsuser10', '0' ],
  [ 'KJPBS User 11', 'vnc.purebiblesearch.com', '6090', 'pbsuser11', '0' ],
  [ 'KJPBS User 12', 'vnc.purebiblesearch.com', '6091', 'pbsuser12', '0' ],
  [ 'KJPBS User 13', 'vnc.purebiblesearch.com', '6092', 'pbsuser13', '0' ],
  [ 'KJPBS User 14', 'vnc.purebiblesearch.com', '6093', 'pbsuser14', '0' ],
  [ 'KJPBS User 15', 'vnc.purebiblesearch.com', '6094', 'pbsuser15', '0' ],
  [ 'KJPBS User 16', 'vnc.purebiblesearch.com', '6095', 'pbsuser16', '0' ],
  [ 'KJPBS User 17', 'vnc.purebiblesearch.com', '6096', 'pbsuser17', '0' ],
  [ 'KJPBS User 18', 'vnc.purebiblesearch.com', '6097', 'pbsuser18', '0' ],
  [ 'KJPBS User 19', 'vnc.purebiblesearch.com', '6098', 'pbsuser19', '0' ],
  [ 'KJPBS User 20', 'vnc.purebiblesearch.com', '6099', 'pbsuser20', '0' ],
  [ 'KJPBS SPECIAL', 'vnc.purebiblesearch.com', '6100', 'kjpbs', '0' ]
);


# Test:
for my $i ( 0 .. $#services ) {
  $status .= '<p>' . $services[$i][0] . " is ";
  $socket = IO::Socket::INET->new ( PeerAddr => $services[$i][1],
                                    PeerPort => $services[$i][2],
                                    Proto    => 'tcp',
                                    Type     => SOCK_STREAM );
  if ($socket) {
    $status .= "<b>In Use</b>";
    $services[$i][4] = 1;
    close($socket);
  } else {
    $status .= "<b>Available</b>";
    $services[$i][4] = 0;
  }
  $status .= "</p>\n";
}


#Print Results
print header;
print start_html('King James Pure Bible Search VNC Service Status');
print h1('King James Pure Bible Search VNC Service Status') . "\n";
print $status;
print end_html;
exit;

