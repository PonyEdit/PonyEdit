#!/usr/bin/env perl

use strict;
use warnings;
use Term::ANSIColor;

my $failing = 0;
my $failed = 0;

while (<>) {
	if ( /^PASS/ ) {
		$failing = 0;
		print color( "green" );
	} elsif ( /^FAIL/ ) {
		$failing = $failed = 1;
		print color( "red" );
	} elsif ( /^Totals/ ) {
		$failing = 0;
		print $failed ? color( "red" ) : color( "green" );
	}
	print $_;
	if ( ! $failing ) {
		print color( "reset" );
	}
}

exit $failed;
