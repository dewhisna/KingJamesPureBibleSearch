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

if ($resolution eq '') {
  $resolution = '1280x1024';
}

if ($bbl eq '') {
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
    print "<a href=\"http://vnc.purebiblesearch.com/status.html\">Click here to see status</a><br>\n";
    print end_html;
  }
} else {
  print redirect("http://vnc.purebiblesearch.com/?resolution=$resolution&bbl=$bbl");
}

exit;
