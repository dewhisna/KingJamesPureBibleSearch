#!/usr/bin/perl

use strict;
use CGI qw(:standard);
use Captcha::reCAPTCHA;

my $captcha = Captcha::reCAPTCHA->new;

print header;
print start_html('King James Pure Bible Search VNC Service');
print "<form action=\"/cgi-bin/captcha_check.pl\" method=\"post\">";
print $captcha->get_html("6Lf3Eu8SAAAAAHr_U6arZ0wXCbEreLNVTFDJC_tp");
print "<input type=\"submit\" value=\"submit\" />";
print "</form>";
print end_html;

