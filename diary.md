## Date: 2024-12-22

These instructions will be put on hiatus
### Todos
- [ ] Complete instructions until 0x3e
- [ ] Finish all the add instructions

### Thoughts
After looking back at the code, I realised that it sucks. This is because of all the inconsistencies
in the functions, how the pc is incremented and the difference between addresses and data and stuff.

I will go back, write all the problems, probably raise issues and then refactor the code so that it
does not completely suck.

## Date: 2024-12-23

### Todos
- [x] Change Emulate8080p to Emulate
- [x] Convert all the parameters to foo *bar instead of foo* bar
- [x] Use fprintf for the error messages, e.g. for UnimplementedInstruction
- [x] Remove the AC flag for now, as it is not being used for the Space Invaders game. Make a note for
this so that it can be added later when required
- [x] Change make2ByteWord into combineBytesToWord
- [x] Change break2ByteWord into splitWordToBytes
- [x] For all functions that contain a and b for parameters or something similar like left and right, convert them into
highByte and lowByte
- [x] Get rid of the repeated break2ByteWord like break2ByteWordHL and make sure to replace those
functions with break2ByteWord
- [x] Change getMem into readMemoryAtRegPair, setPair to writeRegPairFromWord, setMem into
writeMemoryAtRegPair, twoRegAddition into addToRegPair
- [x] Delete the comment for inr and dnr that says its casted to 16 bit number
- [x] Change the dad fucntion (answer >> 16) | 1 to (answer >> 16) & 1
- [x] Replace stax with setMem(state, a, b, value) changed to the correct function name and arguments
- [x] Add breaks in the switch statement to prevent fall through
- [x] Lack of default case (Make one for unknown upcode)
- [x] Add bounds checking for readByte and writeByte
- [x] Remove the decrement for the pc in UnimplementedInstruction

## Date: 2024-12-24

### Thoughts
Seems like the code isn't as terrible as I thought. I've noted down the features that need to be
fixed in the 2024-12-23 entry and am working through them one at a time.

Update: All of the issues have been solved and finished


## Date: 2024-12-27

### Thoughts
Changed the move instructions in the switch statements from directly assigning the values in the
switch statement to calling a function.


## Date: 2024-12-28

### Todos
[x] Add state to instructions
[x] Complete all the opcodes until 0x3f

### Thoughts
Had a look over some of the documentation from 0x31 to 3f. Seems fine to implement. I added the
state to all the functions for consistency/future proofing

Update:
Refactoring required. Need to convert the instructions (e.g. lxi) to take a 16 bit word rather than
two registers which are then combined. This is so that the stack pointer works fine with these
instructions. Also need to change the instructions in the switch statement to combine register pairs
into word before calling their instructions. Spain without the S


## Date: 2024-12-30

### Thoughts
Does not look that bad to refactor, just need to change lxi, inx, dcx and dad

## Date: 2025-01-01

### Todos
[x] Make lxi for word
[x] Make inx for word
[x] Make dcx for word
[x] Make dad for word

### Thoughts

Refactored the code slightly, now there are functions that can take the 16 bit stack pointer and
carry out instructions that previously only took resgister pairs

Update: Finally completed the instructions until 0x3f.

## Date: 2025-01-02

### Thoughts
Finished all the arithmetic functions

## Date: 2025-01-05

### Thoughts
Finished all the opcodes until 0xff (YAY). I left certain instructions like the hardware input and
output opcodes so that I can write the code for them when i understand how to read the rom and also
how to output to the display.
