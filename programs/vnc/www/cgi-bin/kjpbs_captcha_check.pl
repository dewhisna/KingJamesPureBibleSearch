#!/usr/bin/perl

use File::Basename;
use lib dirname (__FILE__);

use strict;
use CGI qw(:standard);
use kjpbs_captcha;

our @services;
require "kjpbs_nodes.pl";

my $captcha = kjpbs_captcha->new;

my $challenge = cookie('hash');
my $response = param 'verify';
my $resolution = param 'resolution';
my $bbl = param 'bbl';
my $bible = '85D8A6B0-E670-11E2-A28F-0800200C9A66';

# Limit uncontrolled input:
if ($resolution eq '1024x768') {
} elsif ($resolution eq '1280x720') {
} elsif ($resolution eq '1280x1024') {
} elsif ($resolution eq '1440x810') {
} elsif ($resolution eq '1920x1080') {
} else {
  $resolution = '1280x1024';
}

# Limit uncontrolled input and remap Bible indexes accordingly:
if ($bbl eq '1') {
  # KJV
  $bible = '85D8A6B0-E670-11E2-A28F-0800200C9A66';
} elsif ($bbl eq '2') {
  # KJV1611
  $bible = '8D999FF0-D650-11E3-9C1A-0800200C9A66';
} elsif ($bbl eq '3') {
  # RV1865mv20180504
  $bible = '28A10630-5728-11E8-B566-0800200C9A66';
} elsif ($bbl eq '4') {
  # RVG2010_20150120
  $bible = '925744F0-6B1B-11E5-A837-0800200C9A66';
} elsif ($bbl eq '5') {
  # GERLUT1545
  $bible = 'D7376840-C75F-11E3-9C1A-0800200C9A66';
} else {
  # KJV (default)
  $bible = '85D8A6B0-E670-11E2-A28F-0800200C9A66';
}

# Verify Submission
my $result = $captcha->captcha_verify($challenge, $response);

my $nodeID;
my $launchURL;
my $datetimestring;

if ($result) {
  $nodeID = launch_node($resolution, $bible, '60m');

  if ($nodeID > -1) {
    $launchURL = "http://vnc.purebiblesearch.com:" . $services[$nodeID][3];
    $launchURL .= "/vnc.html?host=vnc.purebiblesearch.com&port=" . $services[$nodeID][3];
    $launchURL .= "&true_color=1&autoconnect=1&cursor=0";

    sleep 5;
    print redirect($launchURL);
  } else {
    open (MYFILE, '>>/var/www/data/kjpbs/server_full.txt');
    $datetimestring = gmtime();
    print MYFILE "$datetimestring\n";
    close (MYFILE);
    print header;
    print start_html('King James Pure Bible Search VNC Service');
    print "<p>Sorry, all King James Pure Bible Search VNC Sessions are in use.</p>\n<br><br>\n";
    print "<a href=\"//vnc.purebiblesearch.com/status/\">Click here to see status</a><br>\n";
    print end_html;
  }
} else {
  print redirect("//vnc.purebiblesearch.com/?resolution=$resolution&bbl=$bbl");
}

exit;
