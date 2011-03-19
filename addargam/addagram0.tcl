proc buildChain {level base chain} {
  global sorted
  global maxLevel
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
  if {$longestLength > $originalLength} {printChain $longestChain}
  return $longestChain
} 

proc good {long short} {
  set l [lindex $long 1]
  set s [lindex $short 1]
  set len [llength $l]
  for {set i 0} {$i < $len} {incr i} {
    if {[string equal $s [lreplace $l $i $i]]} {
      puts "  $short vs. $long: ok"
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

puts "reading"
set f [open WORD.LST]
set words [split [read -nonewline $f] "\n"]
close $f

puts "sorting"
set maxLevel 3
foreach w $words {
  set len [string length $w]
  if {$len > $maxLevel} {set maxLevel $len}
  set s [lsort [split $w ""]]
  lappend sorted($len) [list $w $s [join $s ""]]
}

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
foreach word $sorted(3) {
  set chain [buildChain 4 $word [list $word]]
  printChain $chain
}

