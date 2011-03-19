#
# Name		build.tcl
# Description	Build the user interface
# Author	Akos Polster
# Version	__EXPERIMENTAL__
# Revision	$Id$
#

##############################################################################
# Name		build
# Side Effects	Sets pentek(measure, graph, report)
#		Sets pdata

proc build {} {

    global pentek ptable

    # Decorate the toplevel

    wm title . "Péntek"
    wm iconname . "Péntek"

    # Create the toplevel's menu

    menu .m -tearoff 0

    .m add cascade -menu .m.file -label "File" -underline 0
    menu .m.file
    .m.file add command -label "Új" -underline 0 -command file.new
    .m.file add command -label "Beolvas" -underline 0 -command file.open
    .m.file add command -label "Elment" -underline 0 -command file.save
    .m.file add command -label "Elment, mint..." -underline 8 \
	-command file.saveAs
    .m.file add separator
    .m.file add command -label "Nyomtat..." -underline 0 -command file.print
    .m.file add separator
    .m.file add command -label "Kilép" -underline 0 -command file.exit

    .m add cascade -menu .m.measure -label "Mérés" -underline 0
    menu .m.measure
    .m.measure add command -label "Mér..." -underline 0
    .m.measure add command -label "Kiértékel..." -underline 0

    .m add cascade -menu .m.help -label "Segítség" -underline 0
    menu .m.help
    .m.help add command -label "Magamról..." -underline 0 -command about

    . configure -menu .m

    # Load the pixmaps
    set iconMinimize [image create pixmap -file $pentek(libDir)/minimize.xpm]
    set iconMaximize [image create pixmap -file $pentek(libDir)/maximize.xpm]
    set iconPinDown  [image create pixmap -file $pentek(libDir)/pindown.xpm]

    # Calculate a dark background color
    label .tmp
    set darkBg [darken [.tmp cget -background] 10]
    destroy .tmp

    # Create the main panes

    tixPanedWindow .p -orientation vertical

    foreach w $pentek(windowList) {

	set name [lindex $w 0]
	set title [lindex $w 1]

	.p add $name -min 35
	set pane [.p subwidget $name]
	pack [frame $pane.title -bd 0] -side top -fill x -anchor w -pady 5
	label $pane.title.l -bd 0 -relief raised -text $title -bg $darkBg
	pack $pane.title.l -anchor w -side left -fill both -expand y
	button $pane.title.max -bd 1 -relief raised -image $iconMaximize \
	    -highlightthickness 0 -command "maximize $name"
	button $pane.title.min -bd 1 -relief raised -image $iconMinimize \
	    -highlightthickness 0 -command "minimize $name"
	button $pane.title.float -bd 1 -relief raised -image $iconPinDown \
	    -highlightthickness 0 -command "float $name"
	pack $pane.title.float $pane.title.min $pane.title.max \
	    -side right -padx 0 -pady 0 -fill y

	set pentek($name) [.p subwidget $name]
    }

    pack .p -fill both -expand y

    # Create table for the measured data
    
    set m $pentek(measure)

    set topf [frame $m.f -bd 0]
    set botf [frame $m.b -bd 0]

    set vsb [scrollbar $topf.s]
    set hsb [scrollbar $botf.s -orient horizontal]

    set table [table $topf.t \
		   -cols 10 \
		   -rows 10 \
		   -titlerows 1 \
		   -titlecols 1 \
		   -colstretch all \
		   -rowstretch all \
		   -xscrollcommand "$hsb set" \
		   -yscrollcommand "$vsb set" \
		   -roworigin -1 \
		   -colorigin -1 \
		   -variable ptable]
    pack $vsb -fill y -expand y -side right
    pack $table -fill both -expand n -side left
    pack $hsb -fill x -expand y -side left

    set gap [frame $botf.g -bd 0 -width [winfo reqwidth $vsb]]
    pack $gap -side right

    pack $botf -fill x -side bottom
    pack $topf -fill both -expand y -side top

    $vsb configure -command [list $table yview]
    $hsb configure -command [list $table xview]

    for {set i 0} {$i < 10} {incr i} {
	set ptable(-1,$i) [format "%c" [expr 65+$i]]
    }
    for {set i 0} {$i < 10} {incr i} {
	set ptable($i,-1) [expr $i+1]
    }

    set ptable(0,0) zizi
    set ptable(1,1) zuzu

    puts [list [array get ptable]]
}

###############################################################################
# Name		minimize

proc minimize {name} {

    global pentek

    .p paneconfigure $name -size 35
}

###############################################################################
# Name		maximize

proc maximize {name} {

    global pentek

    foreach pair $pentek(windowList) {
	set this [lindex $pair 0]
	lappend sizes [.p panecget $this -size]
	if {$this != $name} {
	    catch {.p paneconfigure $this -size 35}
	}
    }

    set total 0
    foreach size $sizes {incr total $size}
    set newSize [expr $total - ([llength $pentek(windowList)] - 1) * 35]
    .p paneconfigure $name -size $newSize
}

###############################################################################
# Name		float

proc float {name} {

    global pentek
}

###############################################################################
# Name		about

proc about {} {
    tk_messageBox -message "Péntek: ELISA mérés és kiértékelés

Írta: Polster Ákos
Verzió: __EXPERIMENTAL__" -type ok -title "Péntek"
}

#
# End		build.tcl
#
