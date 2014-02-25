#!/usr/bin/perl

use strict;
use CGI qw/:standard/;

our @services;

require "kjpbs_nodes.pl";

my ($status);


test_kjpbs_nodes();

$status .= "<ul>\n";
for my $i ( 0 .. $#services ) {
  $status .= '<li>' . $services[$i][1] . " is ";

  if ($services[$i][0] != 0) {
    $status .= "<b>In Use</b>";
  } else {
    $status .= "<b>Available</b>";
  }
  $status .= "</li>\n";
}
$status .= "</ul>\n";

#Print Results
print header;
print start_html('King James Pure Bible Search VNC Service Status');
print h1('King James Pure Bible Search VNC Service Status') . "\n";
print $status;
print end_html;
exit;

