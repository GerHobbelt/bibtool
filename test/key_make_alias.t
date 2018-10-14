#!/usr/bin/env perl
# =============================================================================
#  
#  This file is part of BibTool.
#  It is distributed under the GNU General Public License.
#  See the file COPYING for details.
#  
#  (c) 2011-2018 Gerd Neugebauer
#  
#  Net: gene@gerd-neugebauer.de
#  
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2, or (at your option)
#  any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
#*=============================================================================

=head1 NAME

key_make_alias.t - Test suite for BibTool key.make.alias.

=head1 SYNOPSIS

key_make_alias.t 

=head1 DESCRIPTION

This module contains some test cases. Running this module as program
will run all test cases and print a summary for each. Optionally files
*.out and *.err are left if the expected result does not match the
actual result.

=head1 OPTIONS

none 

=head1 AUTHOR

Gerd Neugebauer

=cut

use strict;
use BUnit;use warnings;



#------------------------------------------------------------------------------
BUnit::run(name  => 'key_make_alias_1',
    bib		 => <<__EOF__,
\@article{xyzzy, author={A.U.Thor}, title="The Title"}
__EOF__
    resource  => <<__EOF__,
key.generation=on
key.format=short
key.make.alias=on
__EOF__
    expected_out => <<__EOF__,

\@Article{	  thor:title,
  author	= {A.U.Thor},
  title		= "The Title"
}
\@ALIAS{xyzzy	= thor:title }
__EOF__
    expected_err => '' );

#------------------------------------------------------------------------------
BUnit::run(name  => 'key_make_alias_2',
    bib		 => <<__EOF__,
\@article{thor:title, author={A.U.Thor}, title="The Title"}
__EOF__
    resource  => <<__EOF__,
key.generation=on
key.format=short
key.make.alias=on
__EOF__
    expected_out => <<__EOF__,

\@Article{	  thor:title,
  author	= {A.U.Thor},
  title		= "The Title"
}
__EOF__
    expected_err => '' );

#------------------------------------------------------------------------------
BUnit::run(name  => 'key_make_alias_3',
    bib		 => <<__EOF__,
\@article{, author={A.U.Thor}, title="The Title"}
__EOF__
    resource  => <<__EOF__,
key.generation=on
key.format=short
key.make.alias=on
quiet=on
__EOF__
    expected_out => <<__EOF__,

\@Article{	  thor:title,
  author	= {A.U.Thor},
  title		= "The Title"
}
__EOF__
    expected_err => '' );

#------------------------------------------------------------------------------
BUnit::run(name         => 'key_make_alias_4',
	   resource	=> <<__EOF__ ,
key.base = digit
key.format=short
key.make.alias=on
__EOF__
	   bib 	        => <<__EOF__,
\@Misc{		  xxx,
  author	= "A. U. Thor"
}
\@Misc{		  xxx,
  author	= "A. U. Thor"
}
__EOF__
	   expected_out => <<__EOF__ );

\@Misc{		  thor,
  author	= "A. U. Thor"
}

\@Misc{		  thor*1,
  author	= "A. U. Thor"
}
\@ALIAS{xxx	= thor }
\@ALIAS{xxx	= thor*1 }
__EOF__

1;
#------------------------------------------------------------------------------
# Local Variables: 
# mode: perl
# End: 
