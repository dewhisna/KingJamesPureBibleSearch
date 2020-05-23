#!/usr/bin/perl

use strict;
use IO::Socket;

# Add the services to check here:
# [ STATUS, ServiceName, IPAddress, noVNCPort, VNCPort, UserID ]

our @services = (
  [ '0', 'KJPBS Node 01', 'localhost', '6080', '5900', 'pbsuser01' ],
  [ '0', 'KJPBS Node 02', 'localhost', '6081', '5901', 'pbsuser02' ],
  [ '0', 'KJPBS Node 03', 'localhost', '6082', '5902', 'pbsuser03' ],
  [ '0', 'KJPBS Node 04', 'localhost', '6083', '5903', 'pbsuser04' ],
  [ '0', 'KJPBS Node 05', 'localhost', '6084', '5904', 'pbsuser05' ],
  [ '0', 'KJPBS Node 06', 'localhost', '6085', '5905', 'pbsuser06' ],
  [ '0', 'KJPBS Node 07', 'localhost', '6086', '5906', 'pbsuser07' ],
  [ '0', 'KJPBS Node 08', 'localhost', '6087', '5907', 'pbsuser08' ],
  [ '0', 'KJPBS Node 09', 'localhost', '6088', '5908', 'pbsuser09' ],
  [ '0', 'KJPBS Node 10', 'localhost', '6089', '5909', 'pbsuser10' ],
  [ '0', 'KJPBS Node 11', 'localhost', '6090', '5910', 'pbsuser11' ],
  [ '0', 'KJPBS Node 12', 'localhost', '6091', '5911', 'pbsuser12' ],
  [ '0', 'KJPBS Node 13', 'localhost', '6092', '5912', 'pbsuser13' ],
  [ '0', 'KJPBS Node 14', 'localhost', '6093', '5913', 'pbsuser14' ],
  [ '0', 'KJPBS Node 15', 'localhost', '6094', '5914', 'pbsuser15' ],
  [ '0', 'KJPBS Node 16', 'localhost', '6095', '5915', 'pbsuser16' ],
  [ '0', 'KJPBS Node 17', 'localhost', '6096', '5916', 'pbsuser17' ],
  [ '0', 'KJPBS Node 18', 'localhost', '6097', '5917', 'pbsuser18' ],
  [ '0', 'KJPBS Node 19', 'localhost', '6098', '5918', 'pbsuser19' ],
  [ '0', 'KJPBS Node 20', 'localhost', '6099', '5919', 'pbsuser20' ],
  [ '0', 'KJPBS SPECIAL', 'localhost', '6100', '5920', 'kjpbs' ]
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
# (index) launch_node(resolution, bbluuid, runtime)
#
# Launches next available node using specified resolution and
#  Bible Database.  Returns index of the launched node or -1
#  if the launch failed (i.e. no available nodes):  Execution
#  is halted after the prescribed runtime:  Example args:
#	<resolution> : 1280x1024
#	<bbluuid> : 85D8A6B0-E670-11E2-A28F-0800200C9A66
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
  my $bbluuid = $Args[1];
  my $runtime = $Args[2];

  test_kjpbs_nodes();
  for my $i ( 0 .. $#services ) {
    if (($ndxLaunched == -1) && ($services[$i][0] == 0)) {
      $ndxLaunched = $i;
      my $user = $services[$i][5];
      my $VNCPort = $services[$i][4];
      my $noVNCPort = $services[$i][3];
#      system("sudo -u $user QWS_DEPTH=16 QWS_SIZE=$resolution $kjpbsApp -qws -display VNC:$ndxLaunched > /dev/null 2>&1 &");
      system("sudo -u $user $noVNCLaunch --vnc localhost:$VNCPort --listen $noVNCPort --user $user --kjpbs $kjpbsApp --res $resolution --bbluuid $bbluuid --instance $i --runtime $runtime  > /dev/null 2>&1 &");
    }
  }

  return $ndxLaunched;
}


1;
