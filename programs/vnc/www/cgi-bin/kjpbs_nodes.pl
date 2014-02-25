#!/usr/bin/perl

use strict;
use IO::Socket;

# Add the services to check here:
# [ STATUS, ServiceName, IPAddress, noVNCPort, VNCPort, UserID ]

our @services = (
  [ '0', 'KJPBS Node 01', 'vnc.purebiblesearch.com', '6080', '5900', 'pbsuser01' ],
  [ '0', 'KJPBS Node 02', 'vnc.purebiblesearch.com', '6081', '5901', 'pbsuser02' ],
  [ '0', 'KJPBS Node 03', 'vnc.purebiblesearch.com', '6082', '5902', 'pbsuser03' ],
  [ '0', 'KJPBS Node 04', 'vnc.purebiblesearch.com', '6083', '5903', 'pbsuser04' ],
  [ '0', 'KJPBS Node 05', 'vnc.purebiblesearch.com', '6084', '5904', 'pbsuser05' ],
  [ '0', 'KJPBS Node 06', 'vnc.purebiblesearch.com', '6085', '5905', 'pbsuser06' ],
  [ '0', 'KJPBS Node 07', 'vnc.purebiblesearch.com', '6086', '5906', 'pbsuser07' ],
  [ '0', 'KJPBS Node 08', 'vnc.purebiblesearch.com', '6087', '5907', 'pbsuser08' ],
  [ '0', 'KJPBS Node 09', 'vnc.purebiblesearch.com', '6088', '5908', 'pbsuser09' ],
  [ '0', 'KJPBS Node 10', 'vnc.purebiblesearch.com', '6089', '5909', 'pbsuser10' ],
  [ '0', 'KJPBS Node 11', 'vnc.purebiblesearch.com', '6090', '5910', 'pbsuser11' ],
  [ '0', 'KJPBS Node 12', 'vnc.purebiblesearch.com', '6091', '5911', 'pbsuser12' ],
  [ '0', 'KJPBS Node 13', 'vnc.purebiblesearch.com', '6092', '5912', 'pbsuser13' ],
  [ '0', 'KJPBS Node 14', 'vnc.purebiblesearch.com', '6093', '5913', 'pbsuser14' ],
  [ '0', 'KJPBS Node 15', 'vnc.purebiblesearch.com', '6094', '5914', 'pbsuser15' ],
  [ '0', 'KJPBS Node 16', 'vnc.purebiblesearch.com', '6095', '5915', 'pbsuser16' ],
  [ '0', 'KJPBS Node 17', 'vnc.purebiblesearch.com', '6096', '5916', 'pbsuser17' ],
  [ '0', 'KJPBS Node 18', 'vnc.purebiblesearch.com', '6097', '5917', 'pbsuser18' ],
  [ '0', 'KJPBS Node 19', 'vnc.purebiblesearch.com', '6098', '5918', 'pbsuser19' ],
  [ '0', 'KJPBS Node 20', 'vnc.purebiblesearch.com', '6099', '5919', 'pbsuser20' ],
  [ '0', 'KJPBS SPECIAL', 'vnc.purebiblesearch.com', '6100', '5920', 'kjpbs' ]
);


my $kjpbsApp = "/var/www/KingJamesPureBibleSearch/KJVCanOpener/app/KingJamesPureBibleSearch";
my $noVNCLaunch = "/var/www/noVNC/utils/launch_kjpbs.sh";

my ($socket);

#
# test_kjpbs_nodes()
#
sub test_kjpbs_nodes {
  for my $i ( 0 .. $#services ) {
    $socket = IO::Socket::INET->new ( PeerAddr => $services[$i][2],
                                      PeerPort => $services[$i][3],
                                      Proto    => 'tcp',
                                      Type     => SOCK_STREAM );
    if ($socket) {
      $services[$i][0] = 1;
      close($socket);
    } else {
      $services[$i][0] = 0;
    }
  }
}

#
# (index) launch_node(resolution, bbl, runtime)
#
# Launches next available node using specified resolution and
#  Bible Database.  Returns index of the launched node or -1
#  if the launch failed (i.e. no available nodes):  Execution
#  is halted after the prescribed runtime:  Example args:
#	<resolution> : 1280x1024
#	<bbl> : 1
#	<runtime> : 3600s
#
# (Repeats node test prior to launching)
#
sub launch_node {
  my ($ndxLaunched, @Args, $nArgs);

  $ndxLaunched = -1;
  @Args = @_;
  $nArgs = scalar(@_);
  if ($nArgs != 3) {
    return $ndxLaunched;
  }
  
  my $resolution = $Args[0];
  my $bbl = $Args[1];
  my $runtime = $Args[2];

  test_kjpbs_nodes();
  for my $i ( 0 .. $#services ) {
    if (($ndxLaunched == -1) && ($services[$i][0] == 0)) {
      $ndxLaunched = $i;
      my $user = $services[$i][5];
      my $VNCPort = $services[$i][4];
      my $noVNCPort = $services[$i][3];
#      system("sudo -u $user QWS_DEPTH=16 QWS_SIZE=$resolution $kjpbsApp -qws -display VNC:$ndxLaunched > /dev/null 2>&1 &");
      system("sudo -u $user $noVNCLaunch --vnc localhost:$VNCPort --listen $noVNCPort --user $user --kjpbs $kjpbsApp --res $resolution --bbl $bbl --instance $i --runtime $runtime  > /dev/null 2>&1 &");
    }
  }

  return $ndxLaunched;
}


1;
