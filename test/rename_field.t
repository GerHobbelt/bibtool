#!/bin/perl -W
# =============================================================================
#  
#  This file is part of BibTool.
#  It is distributed under the GNU General Public License.
#  See the file COPYING for details.
#  
#  (c) 2015-2018 Gerd Neugebauer
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

rename_field.t - Test suite for BibTool rename.field.

=head1 SYNOPSIS

rename_field.t 

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
use BUnit;

#------------------------------------------------------------------------------
BUnit::run(name => 'rename_field_0',
    args	 => 'bib/x1.bib',
    resource 	 => 'rename.field={xyzzy = booktitle}',
    expected_out => <<__EOF__,

\@Manual{	  bibtool,
  title		= {BibTool},
  author	= {Gerd Neugebauer},
  year		= 2018
}
__EOF__
    expected_err => '' );

#------------------------------------------------------------------------------
BUnit::run(name => 'rename_field_1',
    args	 => 'bib/x1.bib',
    resource 	 => 'rename.field={title = booktitle}',
    expected_out => <<__EOF__,

\@Manual{	  bibtool,
  booktitle	= {BibTool},
  author	= {Gerd Neugebauer},
  year		= 2018
}
__EOF__
    expected_err => '' );

#------------------------------------------------------------------------------
BUnit::run(name => 'rename_field_2',
    args	 => 'bib/x1.bib',
    resource 	 => 'rename.field={Title = booktitle}',
    expected_out => <<__EOF__,

\@Manual{	  bibtool,
  booktitle	= {BibTool},
  author	= {Gerd Neugebauer},
  year		= 2018
}
__EOF__
    expected_err => '' );

#------------------------------------------------------------------------------
BUnit::run(name => 'rename_field_err_1',
    args	 => 'bib/x1.bib',
    resource 	 => 'rename.field={Title}',
    expected_out => <<__EOF__,

\@Manual{	  bibtool,
  title		= {BibTool},
  author	= {Gerd Neugebauer},
  year		= 2018
}
__EOF__
    expected_err => <<__EOF__,

Title
_____^
*** BibTool ERROR: Symbol expected.
__EOF__
    );


#------------------------------------------------------------------------------
BUnit::run(name => 'rename_field_err_2',
    args	 => 'bib/x1.bib',
    resource 	 => 'rename.field={Title booktitle xxx}',
    expected_out => <<__EOF__,

\@Manual{	  bibtool,
  title		= {BibTool},
  author	= {Gerd Neugebauer},
  year		= 2018
}
__EOF__
    expected_err => <<__EOF__,

Title booktitle xxx
________________^
*** BibTool ERROR: Unexpected characters at end of string.
__EOF__
    );

#------------------------------------------------------------------------------
BUnit::run(name => 'rename_field_10',
    args	 => 'bib/x1.bib',
    resource 	 => 'rename.field={Title = booktitle if author "Neu"}',
    expected_out => <<__EOF__,

\@Manual{	  bibtool,
  booktitle	= {BibTool},
  author	= {Gerd Neugebauer},
  year		= 2018
}
__EOF__
    expected_err => '' );

#------------------------------------------------------------------------------
BUnit::run(name => 'rename_field_11',
    args	 => 'bib/x1.bib',
    resource 	 => 'rename.field={Title = booktitle if $type "Book"}',
    expected_out => <<__EOF__,

\@Manual{	  bibtool,
  title		= {BibTool},
  author	= {Gerd Neugebauer},
  year		= 2018
}
__EOF__
    expected_err => '' );

1;
#------------------------------------------------------------------------------
# Local Variables: 
# mode: perl
# End: 
