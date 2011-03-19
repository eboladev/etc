proc binout {x} {
    for {set i 15} {$i >= 0} {incr i -1} {
	puts -nonewline [expr ($x & (1 << $i))? 1: 0]
    }
    puts ""
}

