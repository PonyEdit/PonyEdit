#!/usr/bin/perl
use strict;
use warnings;

our $userPassword = 'HY@I6buut2vugu53jhvtr4juVJU#CY45uh25';
our $encryptedPassword = crypt($userPassword, 'bananas');

#######################################

sub createUser
{
	my ($login) = @_;
	`useradd -m -p $encryptedPassword $login` if (!getpwnam($login));
}

#######################################

createUser('ponytest_a');

