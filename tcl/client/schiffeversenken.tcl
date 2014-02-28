 #! /bin/sh
# the next line restarts using wish \
exec `which wish` $0 ${1+"$@"}

if {0} {
  -------------------------------------
  Schiffe versenken Client
  -------------------------------------
  by jannis_achstetter@web.de
  released under GPLv2
  -------------------------------------
}

wm title . "Schiffe versenken"
wm geometry . =420x350

image create photo wasser -file "wasser.gif"
image create photo wasser-hit -file "wasser-hit.gif"
image create photo schiff-neu -file "schiff-neu.gif"
image create photo schiff-hit -file "schiff-hit.gif"
image create photo schiff-down -file "schiff-down.gif"

labelframe .eigenes -text "Eigenes Spielfeld"
labelframe .fremdes -text "Gegnerisches Spielfeld"
labelframe .gamechat -text "Chat des Spielraums"
labelframe .mainchat -text "Globaler Chatraum"

grid .eigenes .fremdes -sticky nsew
grid .gamechat .mainchat -sticky nsew

for {set i 97} {$i < 107} {incr i} {
  for {set j 0} {$j < 10} {incr j} {
    label .eigenes.[format "%c" $i]$j -image wasser
  }
}

for {set i 97} {$i < 107} {incr i} {
  grid .eigenes.[format "%c" $i]0 .eigenes.[format "%c" $i]1 .eigenes.[format "%c" $i]2 .eigenes.[format "%c" $i]3 .eigenes.[format "%c" $i]4 .eigenes.[format "%c" $i]5 .eigenes.[format "%c" $i]6 .eigenes.[format "%c" $i]7 .eigenes.[format "%c" $i]8 .eigenes.[format "%c" $i]9 -sticky nsew
}

for {set i 97} {$i < 107} {incr i} {
  for {set j 0} {$j < 10} {incr j} {
    label .fremdes.[format "%c" $i]$j -image wasser
  }
}

for {set i 97} {$i < 107} {incr i} {
  grid .fremdes.[format "%c" $i]0 .fremdes.[format "%c" $i]1 .fremdes.[format "%c" $i]2 .fremdes.[format "%c" $i]3 .fremdes.[format "%c" $i]4 .fremdes.[format "%c" $i]5 .fremdes.[format "%c" $i]6 .fremdes.[format "%c" $i]7 .fremdes.[format "%c" $i]8 .fremdes.[format "%c" $i]9 -sticky nsew
}

listbox .gamechat.list -width 27 -height 5
entry .gamechat.entry -width 27
grid .gamechat.list
grid .gamechat.entry

listbox .mainchat.list -width 27 -height 5
entry .mainchat.entry -width 27
grid .mainchat.list
grid .mainchat.entry
