#
# Name		darken.tcl
# Description	Darken a color by a given amount
# Author	Akos Polster
# Version	2.0.0
# Revision	$Id: darken.tcl,v 1.1 1997/07/16 18:36:39 akos Exp $
#

proc darken {color percent} {
    set RGB   [winfo rgb . $color]
    set red   [lindex $RGB 0]
    set green [lindex $RGB 1]
    set blue  [lindex $RGB 2]
    set red   [expr "$red   - ($red   * $percent)/100"]
    set green [expr "$green - ($green * $percent)/100"]
    set blue  [expr "$blue  - ($blue  * $percent)/100"]
    set red   [expr "($red   > 0)? (($red   < 65535)? $red:   65535): 0"]
    set green [expr "($green > 0)? (($green < 65535)? $green: 65535): 0"]
    set blue  [expr "($blue  > 0)? (($blue  < 65535)? $blue:  65535): 0"]
    return [format "#%4.4x%4.4x%4.4x" $red $green $blue]
}

#
# End		darken.tcl
#
