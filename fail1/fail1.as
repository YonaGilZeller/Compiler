; invalid dst
mov r3, #-5
; invalid label name
mov: stop
; invalid command
valid: hello #23
; invalid number (out of range)
prn #1000000
; invalid register
sub r0, r8
; invalid entry - label does not exist
.entry doesNotExist
; label is too long
ThisLabelIsLargerThan30Charsaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa: add #5, r4
; invalid label name
3label: .data 1, 2, 3
; no " at the end of a string
.string "abc
; no , between numbers
.data 1 2 3

