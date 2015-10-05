#!/usr/bin/perl

use strict;
use CGI qw(:standard);
use Captcha::reCAPTCHA;

our @services;
require "kjpbs_nodes.pl";

my $captcha = Captcha::reCAPTCHA->new;

my $challenge = param 'recaptcha_challenge_field';
my $response = param 'recaptcha_response_field';
my $resolution = param 'resolution';
my $bbl = param 'bbl';

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
} elsif ($bbl eq '2') {
  $bbl = '12';
} elsif ($bbl eq '3') {
  $bbl = '8';
} elsif ($bbl eq '4') {
  $bbl = '13';
} elsif ($bbl eq '5') {
  $bbl = '16';
} elsif ($bbl eq '6') {
  $bbl = '7';
} else {
  $bbl = '1';
}

# Verify Submission
my $result = $captcha->check_answer(
  "6Lf3Eu8SAAAAAH10tH_aKL-c6ZAakJj4oBsIjMiv", $ENV{'REMOTE_ADDR'},
  $challenge, $response
);

my $nodeID;
my $launchURL;
my $datetimestring;

if ($result->{is_valid}) {
  $nodeID = launch_node($resolution, $bbl, '60m');

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
    print "<a href=\"http://vnc.purebiblesearch.com/status/\">Click here to see status</a><br>\n";
    print end_html;
  }
} else {
  print redirect("http://vnc.purebiblesearch.com/?resolution=$resolution&bbl=$bbl");
}

exit;
