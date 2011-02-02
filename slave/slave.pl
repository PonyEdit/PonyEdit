# (C) 2010-2011 Mark George & Gary Pendergast. PonyEdit slave script.
use strict;

use Cwd;
use Fcntl ':mode';
use MIME::Base64;

use Data::Dumper;

our %buffers = ();
our $nextBufferId = 1;

# Open and unbuffer the log file
open( LOGFILE, '>>log.txt' );
select( ( select( LOGFILE ), $|=1 )[0] );

sub errlog
{
	print LOGFILE $_[0] . "\n";
}

#
#	Config & Definitions
#

our %dataTypes =
(
	#1 => 's',
	#2 => 'l',
	129 => 'v',
	130 => 'V',
);

use constant FILE_ISDIR =>    1;
use constant FILE_CANREAD =>  2;
use constant FILE_CANWRITE => 4;

#
#	Buffer class
#

{ package Buffer;
  use Digest::MD5 qw(md5_hex);
  use Encode qw(encode decode);
	sub new
	{
		my $self = {};
		bless( $self );
		return $self;
	}

	sub openFile
	{
		my ($self, $name) = @_;
		$self->{NAME} = $name;
		$self->{CLOSED} = 0;

		open( BUFFER_FILE, "<$name" );
		my @data = <BUFFER_FILE>;
		close( BUFFER_FILE );

		$self->{DATA} = join( '', @data );
		$self->{DATA} =~ s/\r\n/\n/g;
		$self->{DATA} = decode( 'UTF-8', $self->{DATA} );
	}

	sub change
	{
		my($self, $pos, $rem, $add) = @_;
		my $data = substr( $self->{DATA}, 0, $pos ) . $add . substr( $self->{DATA}, $pos + $rem, length( $self->{DATA} ) );
		$self->{DATA} = $data;
	}

	sub save
	{
		my $self = shift;
		open( BUFFER_FILE, '>' . $self->{NAME} ) or die "Failed to open $self->{NAME} for writing!\n";
		print BUFFER_FILE encode( 'UTF-8', $self->{DATA} );
		close( BUFFER_FILE );
	}

	sub checksum
	{
		my $self = shift;
		return md5_hex( encode( 'UTF-8', $self->{DATA} ) );
	}

	sub setData
	{
		my ($self, $data) = @_;
		$self->{DATA} = $data;
	}

	sub close
	{
		my $self = shift;
		$self->{DATA} = undef;
		$self->{NAME} = undef;
		$self->{CLOSED} = 1;
	}
}

#
#	DataBlock class
#

{ package DataBlock;
	sub new
	{
		my $self = {};
		bless( $self );
		return $self;
	}

	sub setData
	{
		my ($self, $data) = @_;
		$self->{DATA} = $data;
		$self->{CURSOR} = 0;
	}

	sub read
	{
		my ($self, $fmt) = @_;
		my @v = unpack( $fmt, substr( $self->{DATA}, $self->{CURSOR} ) );
		$self->{CURSOR} += length( pack( $fmt, @v ) );
		return @v;
	}

	sub readString
	{
		my $self = shift;
		my ($length) = $self->read( 'V' );
		my $v = substr( $self->{DATA}, $self->{CURSOR}, $length );
		$self->{CURSOR} += $length;
		return $v;
	}

	sub write
	{
		my ($self, $fmt, @args) = @_;
		$self->{DATA} .= pack( "$fmt", @args );
	}

	sub writeString
	{
		my ($self, $s) = @_;
		$self->{DATA} .= pack( 'V', length( $s ) ) . $s;
	}

	sub getData
	{
		my $self = shift;
		return $self->{DATA};
	}

	sub atEnd
	{
		my $self = shift;
		return $self->{CURSOR} >= length( $self->{DATA} );
	}

	sub readMessage
	{
		my $self = shift;
		my ($messageId, $bufferId, $length) = $self->read( 'vVV' );
		if( $length > length( $self->{DATA} ) - $self->{CURSOR} )
		{
			die( 'Faulty Message Header!' );
		}
		my %params = ();
		my $target = $self->{CURSOR} + $length;
		while( $self->{CURSOR} < $target )
		{
			my ($f, $t) = $self->read( 'CC' );
			my $d;
			if( exists( $dataTypes{$t} ) )
			{
				($d) = $self->read( $dataTypes{$t} );
			}
			elsif( $t == 3 )
			{
				$d = $self->readString();
			}
			$params{chr($f)} = $d;
		}
		if( $self->{CURSOR} != $target )
		{
			main::errlog( 'Warning: TLD message contents didn\'t match length in header!' );
			$self->{CURSOR} = $target;
		}
		return ($messageId, $bufferId, \%params);
	}
}

#
#	Message Handlers
#

#	ls
sub msg_ls
{
	my( $buff, $params, $result ) = @_;
	my $d = $params->{'d'};
	opendir( DIR, $d );

	my $hits = 0;
	while( my $filename = readdir( DIR ) )
	{
		my ($dev,$ino,$mode,$nlink,$uid,$gid,$rdev,$size,
		$atime,$mtime,$ctime,$blksize,$blocks) = stat( "$d/$filename" );

		my $flags =
			(S_ISDIR($mode) ? FILE_ISDIR : 0) +
			(-r "$d/$filename" ? FILE_CANREAD : 0) +
			(-w "$d/$filename" ? FILE_CANWRITE : 0);

		$result->writeString( $filename );
		$result->write( 'CL', $flags, $size );
		$hits++;
	}
	close(DIR);

	die("Permission denied!\n") if ($hits == 0 && !(-r $d));
}

#	open
sub msg_open
{
	my ( $buff, $params, $result ) = @_;

	my $name = $params->{'f'};
	$buff = Buffer->new();
	$buff->openFile( $name );

	my $bufferId = $nextBufferId;
	$nextBufferId += 1;
	$buffers{$bufferId} = $buff;

	$result->write( 'C', (-w $name ? FILE_CANWRITE : 0) );
	$result->write( 'V', $bufferId );
	$result->writeString( $buff->checksum() );
}

#	change
sub msg_change
{
	my( $buff, $params, $result ) = @_;

	$buff->change( $params->{'p'}, $params->{'r'}, $params->{'a'} );
}

#	save
sub msg_save
{
	my( $buff, $params, $result ) = @_;

	my $s = $buff->checksum();
	if( $params->{'c'} ne $s )
	{
		die( "Checksums do not match: $s vs " . $params->{'c'} . "\n" );
	}
	$buff->save();
}

#	keepalive
sub msg_keepalive {}

#	pushcontent
sub msg_pushcontent
{
	my( $buff, $params, $result ) = @_;

	$buff->setData($params->{'d'});

	my $s = $buff->checksum();
	if ($params->{'c'} != $s)
	{
		die( "Checksums do not match: $s vs " . $params->{'c'} . "\n" );
	}

	if ($params->{'s'})
	{
		$buff->save();
	}
}

#	close
sub msg_close
{
	my( $buff, $params, $result ) = @_;

	errlog("Closing buffer.");
	$buff->close();
}

#	new
sub msg_new
{
	my( $buff, $params, $result ) = @_;

	my $opened = open( NEW_FILE, '>' . $params->{'f'} );
	if( !$opened )
	{
		die( "Could not save the file to that location.\n" );
	}

	print NEW_FILE $params->{'c'};
	close( NEW_FILE );
}

#	new directory
sub msg_new_dir
{
	my( $buff, $params, $result ) = @_;

	my $name = $params->{'l'} . '/' . $params->{'n'};

	my $created = mkdir( $name );
	if( !$created )
	{
		die( "Could not create directory.\n" );
	}
}

#
#	Message Definitions
#

our %messageDefs =
(
	1 => \&msg_ls,
	2 => \&msg_open,
	3 => \&msg_change,
	4 => \&msg_save,
	5 => \&msg_keepalive,
	6 => \&msg_pushcontent,
	7 => \&msg_close,
	8 => \&msg_new,
	9 => \&msg_new_dir
);

#
#	Main Guts
#

sub mainLoop
{
	while(1)
	{
		my $line = <STDIN>;
		chomp( $line );
		eval
		{
			$line = decode_base64( $line );
			1;
		}
		or do
		{
			errlog( 'Received some bogus input: ' . $line );
			next;
		};
		my $block = DataBlock->new();
		$block->setData( $line );
		while( ! $block->atEnd() )
		{
			my $reply = handleMessage( $block );
			my $reply_base64 = encode_base64( $reply->getData() );
			$reply_base64 =~ s/\r|\n//g;
			print "$reply_base64\n";
		}
	}
}

sub handleMessage
{
	my $message = shift;

	my ($messageId, $bufferId, $params) = $message->readMessage();

	errlog( "bufferId = $bufferId" );
	errlog( "messageId = $messageId" );
	errlog( 'Parameters = ' . Dumper( $params ) );

	my $result;
	eval
	{
		my $buff;
		if( $bufferId > 0 )
		{
			if( ! exists( $buffers{$bufferId} ) )
			{
				die( "Invalid BufferId: $bufferId" );
			}
			$buff = $buffers{$bufferId};
		}
		else
		{
			$buff = "";
		}

		if( ! exists( $messageDefs{$messageId} ) )
		{
			die( "Invalid messageId: $messageId" );
		}
		$result = DataBlock->new();
		$result->write( 'C', 1 );
		&{$messageDefs{$messageId}}($buff, $params, $result);

		if ($buff && $buff->{CLOSED})
		{
			delete $buffers{$buff};
		}

		1;
	}
	or do
	{
		my $e = $@;
		$e =~ s/\s+$//;
		errlog( "Error occurred: $e" );
		my $err = DataBlock->new();
		$err->write( 'C', 0 );
		$err->writeString( $e );
		return $err;
	};
	return $result;
}

errlog( '*************************************** Starting up *********************************************' );

#	Send the current working directory, which should be the user's home dir.
print '~=' . getcwd() . "%";

mainLoop();

1;
