#!/usr/bin/perl -w

use File::Basename;
use lib dirname (__FILE__);

use kjpbs_captcha;

my $app = kjpbs_captcha->new();

$app->run();


