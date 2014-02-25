#!/usr/bin/perl

use strict;
use CGI qw(:standard);

our @services;
require "kjpbs_nodes.pl";

my $nodeID = launch_node('1280x1024', '1', '60m');

my $launchURL;

if ($nodeID > -1) {
  $launchURL = "http://vnc.purebiblesearch.com:" . $services[$nodeID][3];
  $launchURL .= "/vnc.html?host=vnc.purebiblesearch.com&port=" . $services[$nodeID][3];
  $launchURL .= "&true_color=1&autoconnect=1&cursor=0";

  sleep 5;
  print redirect($launchURL);
} else {
  print header;
  print start_html('King James Pure Bible Search VNC Service');
  print "<p>Sorry, all King James Pure Bible Search VNC Sessions are in use.</p>\n<br><br>\n";
  print "<a href=\"http://vnc.purebiblesearch.com/cgi-bin/status.pl\">Click here to see status</a><br>\n";
  print end_html;
}

exit;

