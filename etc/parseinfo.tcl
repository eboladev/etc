proc parseInfo {text ret} {
  upvar ret info
  set map {"&lt;" "<" "&gt;" ">"} ;# Incomplete, but you can get the idea
  set inDescription 0
  foreach line [split $text "\n"] {
    if {[string equal "<description>" $line]} {
      set inDescription 1
    } elseif {[string equal "</description>" $line]} {
      set inDescription 0
    } elseif {$inDescription} {
      append info(<description>) [string map $map $line] "\n"
    } elseif {[regexp {(^<[a-z]+>)(.*)(</[a-zA-Z]+>$)} $line all key val end]} {
        set info($key) [string map $map $val]
    }
  }
}

set text "<?xml version=\"1.0\"?>
<tclpackage>
<name>base64</name>
<author>Stephen Uhler, Brent Welch</author>
<copyright>Copyright (c) 1998-200 by Scriptics Corporation</copyright>
<description>
Encode/decode base64 for a string.
The decoder was done for exmh by Chris Garrigues.
And now:
Let's see how the mapping works: &lt; and &gt;
</description>
</tclpackage>"

parseInfo $text ret
parray ret
