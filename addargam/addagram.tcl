proc buildChain {level base chain} {
  global sorted
  global maxLevel
  global longestAll
  if {$level > $maxLevel} {
    return $chain
  }
  set longestChain $chain
  set longestLength [llength $chain]
  set originalLength $longestLength
  set nextLevel [expr {$level + 1}]
  foreach word $sorted($level) {
    if {[good $word $base]} {
      set testChain $chain
      lappend testChain $word
      set newChain [buildChain $nextLevel $word $testChain]
      set newLength [llength $newChain]
      if {$newLength > $longestLength} {
        set longestLength $newLength
        set longestChain $newChain
      }
    }
  }
  if {$longestLength >= $longestAll} {
    printChain $longestChain
    set longestAll $longestLength
  }
  return $longestChain
} 

proc good {long short} {
  set variations [lindex $long 3]
  set s [lindex $short 2]
  foreach try $variations {
    if {[string equal $s $try]} {
      # puts "  [lindex $short 0] vs. [lindex $long 0]: o.k."
      return 1
    }
  }
  return 0
}

proc printChain {c} {
  foreach e $c {
    puts -nonewline "[lindex $e 0] "
  }
  puts ""
}

global sorted
global maxLevel
global longestAll

puts "reading"
set f [open WORD.LST]
set words [split [read -nonewline $f] "\n"]
close $f
set total [llength $words]

puts "sorting"
set maxLevel 3
set cnt 0
set last 0
foreach w $words {
  set len [string length $w]
  if {$len > $maxLevel} {set maxLevel $len}
  set s [lsort [split $w ""]]
  set variations {}
  for {set i 0} {$i < $len} {incr i} {
    lappend variations [join [lreplace $s $i $i] ""]
  }
  lappend sorted($len) [list $w $s [join $s ""] $variations]
  incr cnt
  set percent [expr {($cnt * 100) / $total}]
  if {$percent > $last} {
    set last $percent
    puts -nonewline "\r${percent}%"
  }
}
puts ""

if {$maxLevel == 3} {
  puts "No words with more than 3 letters"
  exit 1
}

puts "sorting-sorting"
for {set i 3} {$i <= $maxLevel} {incr i} {
  if {[info exist sorted($i)]} {
    # set sorted($i) [lsort -index 2 $sorted($i)]
    set sorted($i) [lsort -unique -index 2 $sorted($i)]
  } else {
    set sorted($i) {}
  }
}

puts "playing"
set longestChain {}
set longestAll 0

foreach word $sorted(3) {
  set chain [buildChain 4 $word [list $word]]
  set len [llength $chain]
  if {$len >= $longestAll} {
    set longestAll $len
    puts $chain
  }
}

