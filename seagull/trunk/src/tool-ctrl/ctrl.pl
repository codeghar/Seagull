#!/usr/bin/perl -w
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
# (c)Copyright 2006 Hewlett-Packard Development Company, LP.

#----------(      Purpose     )------------
# Sample implementation of the Seagull remote control interface which 
# uses http as protocol.
# This application parses a scenario file which can contain the following
# commands:
# SET RATE n      : equivalent to set call-rate n in Seagull
# WAIT n[M|S]     : wait n minutes or n seconds
# DUMP            : Dump available counters
# RAMP n IN o[M|S]: Go to scenario-rate n in o minutes or o seconds
# 
# A sample scenario could be:
# SET RATE 20 CPS
# WAIT 2S
# DUMP
# WAIT 2S
# DUMP
# SET RATE 40 CPS
# WAIT 2S
# DUMP
# WAIT 2S
# DUMP
# RAMP 100 IN 30S
# WAIT 10S
# DUMP
# WAIT 10S
# DUMP
# WAIT 10S
# DUMP
#
# Note: if you enhance this tool, please provide back any changes
#       on the Seagull-users mailing list (gull-users@lists.sourceforge.net)
#       Thanks!
#------------------------------------------

use strict;
use LWP::UserAgent;

#----------( Global variables )------------
our @G_scenario;
#------------------------------------------

#------------------------------------------
# FUNCTION:  readScenario
# PURPOSE:   Read a scenario file
# ARGS:      Scenario name
#------------------------------------------
sub readScenario {
  my ($myFile) = @_;

  open FILE, $myFile or return 0;
  @G_scenario = <FILE>;
  close(FILE);
  return 1;
}
    
#------------------------------------------
# FUNCTION:  sendRequest
# PURPOSE:   Send a request to remote controlled
#            Seagull
# ARGS:      Request
#            URI 
#------------------------------------------
sub sendRequest {
  my ($myRequest, $myUri) = @_;
  
  # User agent
  my $ua =
    LWP::UserAgent->new(
      agent => 'Seagull controller' );

  # Set a timeout for the request (in s)
  $ua->timeout(3);
    
  # Request 
  my $req = HTTP::Request->new( ${myRequest} => "${myUri}" );
   
  # Execute the request and receive response
  my $res = $ua->request($req);
  if (not $res->is_success) {
    print "Error while sending request $myRequest $myUri:\n  " . $res->status_line . "\n";
    return 0;
  } else {
    print $res->content;
  }
  return 1;
}

#------------------------------------------
# FUNCTION:  executeScenario
# PURPOSE:   execute the scenario
# ARGS:      Base URI
#------------------------------------------
sub executeScenario {
  my ($myBase) = @_;

  for(my $i=0; $i<$#G_scenario; $i++) {
    print ">" . $G_scenario[$i];
    if ($G_scenario[$i] =~ m/^[\#\']/) {
      # Comment: do nothing
    } elsif ($G_scenario[$i] =~ m/^SET RATE ([0-9]*)/) {
      sendRequest("PUT", $myBase . "seagull/command/rate?value=$1") || return 0;
    } elsif ($G_scenario[$i] =~ m/^WAIT ([0-9]*)([MS]*)/) {
      if ($2 eq "M") {
        sleep($1*60);
      } elsif ($2 eq "S") {
        sleep($1);
      } else {
        print "Don't know how to wait for $1$2\n";
	return 0;
      }
    } elsif ($G_scenario[$i] =~ m/^DUMP/) {
      sendRequest("GET", $myBase . "seagull/counters/all") || return 0;
    } elsif ($G_scenario[$i] =~ m/^RAMP ([0-9]*) IN ([0-9]*)([MS]*)/) {
      my $duration;
      if ($3 eq "M") {
        $duration = $2 * 60;
      } elsif ($3 eq "S") {
        $duration = $2;
      } else {
        print "Don't know how to ramp for $2$3\n";
	return 0;
      }
      sendRequest("PUT", $myBase . "seagull/command/ramp?value=$1&duration=$duration") || return 0;
    } else {
      print "Don't know how to execute scenario line '$G_scenario[$i]'\n";
      return 0;
    }
  }
  return 1;
}

#------------------------------------------
# FUNCTION:  Main program
#------------------------------------------
if ($#ARGV + 1 == 2) {
  readScenario ($ARGV[1]) || die ("Can't read file '$ARGV[1]': $!\n");
  executeScenario($ARGV[0]) || die ("Can't execute scenario\n");
} else {
  print "USAGE: $0 <seagull URI> <scenario file>\n";
  print "EXAMPLE: $0 http://127.0.0.1:8088/ scenario.txt\n";
}

