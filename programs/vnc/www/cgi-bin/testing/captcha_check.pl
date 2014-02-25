#!/usr/bin/perl

use CGI qw(:standard);
use Captcha::reCAPTCHA;

my $captcha = Captcha::reCAPTCHA->new;

my $challenge = param 'recaptcha_challenge_field';
my $response = param 'recaptcha_response_field';

# Verify Submission
my $result = $captcha->check_answer(
  "6Lf3Eu8SAAAAAH10tH_aKL-c6ZAakJj4oBsIjMiv", $ENV{'REMOTE_ADDR'},
  $challenge, $response
);

print header;
print start_html('King James Pure Bible Search VNC Service');
if ( $result->{is_valid} ) {
  print "<h1>Yes!</h1>\n";
} else {
  print "<h1>No</h1>\n";
}
print end_html;
