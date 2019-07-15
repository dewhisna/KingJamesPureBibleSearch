#!/usr/bin/perl

use File::Basename;
use lib dirname (__FILE__);

use strict;
use CGI qw/:standard/;

our @services;

require "kjpbs_nodes.pl";

my ($status);


test_kjpbs_nodes();

#$status .= "<table border=\"1\">\n";
$status .= "<table style=\"border:1px solid #FF0000;\">\n";
for my $i ( 0 .. $#services-1 ) {
  $status .= "<tr>\n";
  $status .= "<td style=\"height:25; border:1px solid #00FF00;\"><br />" . $services[$i][1] . "<br /><br /></td>\n";

  if ($services[$i][0] != 0) {
    $status .= "<td style=\"height:25; border:1px solid #00FF00;\"><br /><b>In Use</b><br /><br /></td>";
  } else {
    $status .= "<td style=\"height:25; border:1px solid #00FF00;\"><br /><b>Available</b><br /><br /></td>";
  }
  $status .= "</tr>\n";
}
$status .= "</table>\n";

#Print Results
print "Content-type: text/html\n\n";
#print header;
#print start_html('King James Pure Bible Search VNC Service Status');
#print h1('King James Pure Bible Search VNC Service Status') . "\n";
print $status;
#print end_html;
exit;

