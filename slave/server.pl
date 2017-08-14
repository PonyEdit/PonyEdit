# (C) 2010-2011 Mark George & Gary Pendergast. PonyEdit slave script.
use strict;
use POSIX;
use Scalar::Util;
use IO::Select;
use Fcntl qw(:mode);
use Digest::MD5;
use List::Util qw(min);
use File::Path;
use Encode;
use utf8;

`stty -isig -icanon -onlcr`;

our %buffers = ();
our $nextBufferId = 1;
our %calls =
(
	'ls' => \&msg_ls,
	'open' => \&msg_open,
	'change' => \&msg_change,
	'save' => \&msg_save,
	'close' => \&msg_close,
	'mkdir' => \&msg_mkdir,
);

#	Open and unbuffer a logfile
open LOGFILE, '>>.ponyedit/error.log';
select((select(LOGFILE), $|=1)[0]);
sub errlog { print LOGFILE POSIX::strftime("[%Y-%m-%d %H:%M:%S] - ", localtime) . $_[0] . "\n"; }
$| = 1;

#	Startup information
print "Slave OK\n";
print json::encode({'~' => getcwd()}) . "\n";
print "\%-ponyedit-\%";

if ($ARGV[0] eq 'xfer')
{
	xferLoop();
}
else
{
	slaveLoop();
}

sub slaveLoop
{
	my $inBuffer = '';
	our $inputs = IO::Select->new();
	$inputs->add(\*STDIN);
	while (1)
	{
		my @ready = $inputs->can_read;
		foreach my $in (@ready)
		{
			my $retry = 1;
			while ($retry)
			{
				my $data;
				$retry = (sysread($in, $data, 2048) == 2048);
				$inBuffer .= $data;

				my @pieces = split("\n", $inBuffer, 2048);
				if (@pieces > 1)
				{
					$inBuffer = substr($inBuffer, rindex($inBuffer, "\n") + 1);
					for (my $i = 0; $i < (@pieces - 1); $i++)
					{
						eval
						{
							my $leftover = 0;
							my $command = unbin($pieces[$i], \$leftover);
							Encode::_utf8_on($command);
							slaveCommand($command); 1;
						}
						or do
						{
							errlog("Error handling slave command: $@\nCause: $pieces[$i]");
						};
					}
				}
			}
		}
	}
}

sub slaveCommand
{
	my ($line) = @_;
	my $message = json::decode($line);
	my $call = $calls{$message->{'c'}};
	my $reply;

	errlog("Handling: $line");

	if (defined $call)
	{
		$@ = undef;
		eval
		{
			my $buff;
			if (defined($message->{'b'}))
			{
				$buff = $buffers{$message->{'b'}};
				die "Invalid buffer id" if (!defined($buff));
			}

			$reply = $call->($message->{'p'}, $buff); 1;
		};
		if ($@)
		{
			$reply = {'error' => $@};
		};
	}
	else
	{
		$reply = {'error' => "Invalid method: $message->{'c'}"};
	}

	$reply->{'i'} = $message->{'i'};
	errlog("Replying: " . json::encode($reply));

	print json::encode($reply) . "\n";
}

sub expandPath
{
	$_[0] =~ s{ ^ ~ ( [^/]* ) } { $1 ? (getpwnam($1))[7] : ( $ENV{HOME} || $ENV{LOGDIR} || (getpwuid($>))[7] ) }ex;
	return $_[0];
}

sub msg_ls
{
	my ($p) = @_;
	my $hidden = $p->{'hidden'};
	my $dir = expandPath($p->{'dir'});
	my $entries = {};

	opendir DIR, $dir;
	while (my $filename = readdir DIR)
	{
		next if (!$hidden && $filename =~ /^\./);
		next if ($filename eq '.' || $filename eq '..');

		my ($dev,$ino,$mode,$nlink,$uid,$gid,$rdev,$size,
			$atime,$mtime,$ctime,$blksize,$blocks) = stat("$dir/$filename");

		my $flags =
			(S_ISDIR($mode) ? 'd' : '') .
			((-r "$dir/$filename") ? 'r' : '') .
			((-w "$dir/$filename") ? 'w' : '');

		$entries->{$filename} = {'f'=>$flags, 's'=>$size, 'm'=>$mtime};
	}
	close DIR;

	return {'error' => 'Permission denied', 'code' => 'perm'} if (scalar(keys %$entries) == 0 && !(-r $dir));
	return {'entries' => $entries};
}


sub msg_open
{
	my ($p) = @_;

	my $name = expandPath($p->{'file'});
	my $buff = Buffer->new();
	$buff->openFile($name);

	my $bufferId = $nextBufferId;
	$nextBufferId += 1;
	$buffers{$bufferId} = $buff;

	return {'writable' => (-w $name ?1:0), 'bufferId' => $bufferId, 'checksum' => $buff->checksum()};
}

#	change
sub msg_change
{
	my ($p, $buff) = @_;
	$buff->change($p->{'p'}, $p->{'d'} || 0, defined($p->{'a'}) ? $p->{'a'} : '');
	return {};
}

#	save
sub msg_save
{
	my ($p, $buff) = @_;

	my $s = $buff->checksum();
	my $c = $p->{'checksum'};
	die "Checksums do not match: $s vs $c\n" if (defined($c) && $c ne $s);

	$buff->save();
	return $p;
}

#	close
sub msg_close
{
	my ($p, $buff) = @_;
	delete $buffers{$buff->id};
	return {};
}

#	mkdir
sub msg_mkdir
{
	my ($p, $buff) = @_;
	my $dir = expandPath($p->{'dir'});

	eval { File::Path::make_path($dir) };
	return {'error' => 'Permission denied', 'code' => 'perm'} if ($@);

	return {};
}

#	Buffer class
{ package Buffer;
	use Encode qw(encode decode);
	use Digest::MD5 qw(md5_hex);

	sub new
	{
		my $id = shift;
		my $self = {'id' => $id};
		bless $self;
		return $self;
	}

	sub id { $_[0]->{'id'} }

	sub openFile
	{
		my ($self, $name) = @_;
		$self->{NAME} = $name;
		$self->{CLOSED} = 0;

		open BUFFER_FILE, "<$name";
		my @data = <BUFFER_FILE>;
		close BUFFER_FILE;

		$self->{DATA} = join( '', @data );
		$self->{DATA} =~ s/\r\n/\n/g;
		$self->{DATA} = decode('UTF-8', $self->{DATA});
	}

	sub change
	{
		my ($self, $pos, $rem, $add) = @_;
		my $data = substr($self->{DATA}, 0, $pos) . $add . substr($self->{DATA}, $pos + $rem, length($self->{DATA}));
		$self->{DATA} = $data;
	}

	sub save
	{
		my $self = shift;
		open(BUFFER_FILE, '>' . $self->{NAME}) or die "Failed to open $self->{NAME} for writing!\n";
		print BUFFER_FILE encode('UTF-8', $self->{DATA});
		close BUFFER_FILE;
	}

	sub checksum
	{
		my $self = shift;
		return md5_hex(encode('UTF-8', $self->{DATA}));
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

#	json class
{ package json;
	sub encode
	{
		my ($obj) = @_;
		return 'null' if (!defined($obj));

		my $ref = ref $obj;
		return '{' . join(',', map { _escape($_) . ':' . encode($obj->{$_}) } keys %$obj) . '}' if ($ref eq 'HASH');
		return '[' . join(',', map { encode($ref->{$_}) } @$obj) . ']' if ($ref eq 'ARRAY');
		return $obj if Scalar::Util::looks_like_number($obj);
		return _escape($obj);
	}

	sub decode
	{
		my ($str) = @_;
		my $index = 0;
		return _decode($str, \$index);
	}

	sub _escape
	{
		$_[0] =~ s/(["\\])/\\$1/g;
		$_[0] =~ s/\r/\\r/g;
		$_[0] =~ s/\n/\\n/g;
		$_[0] =~ s/\t/\\n/g;
		return "\"$_[0]\"";
	}

	sub _nextchr
	{
		my ($str, $idx) = @_;
		return substr($_[0], ${$_[1]}++, 1);
	}

	sub _nextnonwhite
	{
		my $c;
		do { $c = &_nextchr; } while ($c eq ' ' or $c eq "\t" or $c eq "\n");
		return $c;
	}

	sub _decode
	{
		my $c = &_nextnonwhite;
		if    ($c eq '{') { return &_object; }
		elsif ($c eq '[') { return &_array; }
		elsif ($c eq '"') { return &_string; }
		elsif ($c =~ /[0-9\-]/)
		{
			${$_[1]}--;
			return &_number;
		}
		else
		{
			${$_[1]}--;
			return &_word;
		}
	}

	sub _object
	{
		my $r = {};
		my $c;
		while (1)
		{
			$c = &_nextnonwhite;
			return $r if ($c eq '}');
			die "Expected: \" at ${$_[1]}\n" if ($c ne '"');

			my $key = &_string;
			die "Expected: : at ${$_[1]}\n" if (&_nextnonwhite ne ':');
			$r->{$key} = &_decode;

			$c = &_nextnonwhite;
			return $r if ($c eq '}');
			die "Expected: } or , at ${$_[1]}\n" if ($c ne ',');
		}
	}

	sub _array
	{
		my $r = [];
		while (1)
		{
			return $r if (&_nextnonwhite eq ']');
			${$_[1]}--;

			push @$r, &_decode;

			my $c = &_nextnonwhite;
			return $r if ($c eq ']');
			die "Expected: ] or , at ${$_[1]}\n" if ($c ne ',');
		}
	}

	sub _string
	{
		my $r = '';
		my $c;
		my $l = ${$_[1]};
		while (($c = &_nextchr) ne '"')
		{
			if ($c eq '\\')
			{
				$r .= substr($_[0], $l, ${$_[1]} - $l - 1);
				$c = &_nextchr;
				if    ($c eq 'b') { $r .= "\b"; }
				elsif ($c eq 'f') { $r .= "\f"; }
				elsif ($c eq 'n') { $r .= "\n"; }
				elsif ($c eq 'r') { $r .= "\r"; }
				elsif ($c eq 't') { $r .= "\t"; }
				elsif ($c eq 'u')
				{
					$r .= chr hex substr($_[0], ${$_[1]}, 4);
					${$_[1]} += 4;
				}
				else
				{
					$r .= $c;
				}
				$l = ${$_[1]};
			}
		}
		return $r . substr($_[0], $l, ${$_[1]} - $l - 1);
	}

	sub _number
	{
		pos($_[0]) = ${$_[1]};
		$_[0] =~ /\G([0-9\-.eE]+)/;
		${$_[1]} += length($1);

		return $1;
	}

	sub _word
	{
		pos($_[0]) = ${$_[1]};
		$_[0] =~ /\G([a-zA-Z]+)/;
		${$_[1]} += length($1);

		my $r = lc($1);
		return 1 if ($r eq 'true');
		return 0 if ($r eq 'false');
		return undef if ($r eq 'null');

		die "Invalid bareword: $r at ${$_[1]}\n";
	}
}

sub bin
{
	my ($data) = @_;

	my $reply = '';
	my $lastPos = 0;
	while ($data =~ m/[\x{03}\x{04}\x{08}\x{0A}\x{0D}\x{11}\x{13}\x{1D}\x{1E}\x{18}\x{1A}\x{1C}\x{7F}\x{FD}\x{FE}\x{FF}]/g)
	{
		$reply .= substr($data, $lastPos, pos($data) - $lastPos - 1) if ($lastPos + 1 < pos($data));
		$lastPos = pos($data);

		my $c = ord substr($data, $lastPos - 1, 1);

		if ($c == 10) { $reply .= chr(253); }
		elsif ($c == 13) { $reply .= chr(254); }
		elsif ($c == 253) { $reply .= chr(255) . 'A'; }
		elsif ($c == 254) { $reply .= chr(255) . 'B'; }
		elsif ($c == 255) { $reply .= chr(255) . 'C'; }
		else { $reply .= chr(255) . chr($c + 128); }
	}

	$reply .= substr($data, $lastPos);
	return $reply;
}

sub unbin_escape
{
	my ($c) = @_;

	if ($c < 128) { return chr($c + 188);  }
	else { return chr($c - 128); }
}

sub unbin
{
	my ($data, $remainingEscape) = @_;

	my $length = length $data;
	my $reply = '';
	my $lastPos = 0;

	if ($$remainingEscape)
	{
		$reply .= unbin_escape(ord substr($data, 0, 1));

		$lastPos++;
		pos($data) = $lastPos;

		$$remainingEscape = undef;
	}

	while ($data =~ m/[\x{FD}\x{FE}\x{FF}]/g)
	{
		$reply .=  substr($data, $lastPos, pos($data) - $lastPos - 1) if ($lastPos + 1 < pos($data));
		$lastPos = pos($data);

		my $c = ord substr($data, $lastPos - 1, 1);

		if ($c == 253) { $reply .= chr(10); }
		elsif ($c == 254) { $reply .= chr(13); }
		else
		{
			if ($lastPos >= $length)
			{
				$$remainingEscape = 1;
				return $reply;
			}

			$reply .= unbin_escape(ord substr($data, $lastPos, 1));

			$lastPos++;
			pos($data) = $lastPos;
		}
	}

	$reply .= substr($data, $lastPos);
	return $reply;
}

sub stream
{
	my ($size, $in, $out, $encode) = @_;
	my $remainingEscape;

	my $data;
	while ($size > 0)
	{
		my $want = min(2048, $size);

		my $read = read($in, $data, $want);

		die "Failed to read\n" if ($read < $want);
		$size -= $read;

		print $out ($encode ? bin($data) : unbin($data, \$remainingEscape));
	}
}

sub md5File
{
	my ($file) = @_;

	my $md5 = Digest::MD5->new;
	$md5->addfile($file);
	return $md5->hexdigest;
}

sub xferLoop
{
	my ($in, $size, $read);
	binmode(*STDOUT);
	while ($in = <STDIN>)
	{
		chomp $in;
		eval
		{
			my $c = chop $in;
			$in = expandPath($in);

			if ($c eq 'u')
			{
				#	Upload
				$size = <STDIN>;
				my $checksum = <STDIN>;
				chomp $size;
				chomp $checksum;

				open F, ">$in" or die "Denied\n";
				print "Ready\n";
				stream($size, *STDIN, *F, 0);
				close F;

				open F, $in or die "Denied\n";
				my $e = md5File(*F);
				die "Checksum error: '$e' vs '$checksum'\n" if ($e ne $checksum);
				close F;
			}
			else
			{
				#	Download
				die ($in . (-e $in ? " - Denied\n" : " - File not found\n")) if (!-r $in);
				$size = -s $in;
				open F, $in or die "Denied\n";
				print "$size," . md5File(*F) . "\n";
				seek F,0,0;
				stream($size, *F, *STDOUT, 1);
				close F;
			}
		};

		if ($@)
		{
			chomp $@;
			print "Error: $@\n" if $@;
		}
		else
		{
			print "OK\n";
		}
	}
}

# Do not delete the new line at the end of this file
