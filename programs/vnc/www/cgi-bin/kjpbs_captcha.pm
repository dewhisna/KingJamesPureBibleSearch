package kjpbs_captcha;
use base 'CGI::Application';

use CGI::Application::Plugin::CAPTCHA;

sub setup
{
	my $self = shift;

	$self->run_modes([ qw/
		create
	/]);

	$self->captcha_config(
		{
			SECRET => 'RPtLzLKWdm2AU9BplrUBeO+re50',
			IMAGE_OPTIONS  => {
				width   => 200,
				height  => 100,
				lines   => 10,
				font    => "/usr/share/fonts/truetype/msttcorefonts/Arial.ttf",
				ptsize  => 24,
#				gd_font => "MediumBold",
				bgcolor => "#FFFF00",
			},
#			CREATE_OPTIONS   => [ 'normal', 'default' ],
			CREATE_OPTIONS   => [ 'ttf', 'rect' ],
			PARTICLE_OPTIONS => [ 1000 ],
			DEBUG => [ 0 ],
		}
	);
}

sub create
{
	my $self = shift;
	return $self->captcha_create;
}

1;	# end of kjpbs_captcha;

