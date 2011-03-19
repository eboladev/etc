#!/bin/sh
#\
exec tclsh "$0" ${1+"$@"}
puts "argv0: $argv0"
puts "argc: $argc"
puts "argv: [list $argv]"
