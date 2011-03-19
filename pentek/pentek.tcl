#
# Name		pentek.tcl
# Description	Mesure and display Elisa data
# Author	Akos Polster
# Version	2.0.0
# Revision	$Id: pentek.tcl,v 1.1 1997/07/16 17:06:45 akos Exp akos $
#

# Globals

global argc argv argv0 env auto_path pentek

set pentek(libDir) [file dirname $argv0]
set pentek(windowList) {
    {measure	"Mérés"}
    {graph	"Grafikon"}
    {report	"Riport"}
}

# Load the packages I need
package require Tix
package require Tktable

# Append my directory to the auto-path
lappend auto_path $pentek(libDir)

# Build the user interface
build

#
# End		pentek.tcl
#
