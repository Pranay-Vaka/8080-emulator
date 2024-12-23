## Date: 22-12-2024

These instructions will be put on hiatus
### Todos
- [ ] Complete instructions until 0x3e
- [ ] Finish all the add instructions

### Thoughts
After looking back at the code, I realised that it sucks. This is because of all the inconsistencies
in the functions, how the pc is incremented and the difference between addresses and data and stuff.

I will go back, write all the problems, probably raise issues and then refactor the code so that it
does not completely suck.

## Date: 23-12-2024

### Issues
- [ ] Change Emulate8080p to Emulate
- [ ] Convert all the parameters to foo *bar instead of foo* bar
- [ ] Use fprintf for the error messages, e.g. for UnimplementedInstruction
- [ ] Make sure to decrement pc in UnimplementedInstruction and then output the pc
- [ ] Remove the AC flag for now, as it is not being used for the Space Invaders game. Make a note for
this so that it can be added later when required
- [ ] Change make2ByteWord into combineBytesToWord
- [ ] Change break2ByteWord into splitWordToBytes
- [ ] For all functions that contain a and b for parameters or something similar like left and right, convert them into
highByte and lowByte
- [ ] Get rid of the repeated break2ByteWord like break2ByteWordHL and make sure to replace those
functions with break2ByteWord
- [ ] Change getMem into readMemoryAtRegPair, setPair to writeRegPairFromWord, setMem into
writeMemoryAtRegPair, twoRegAddition into addToRegPair
- [ ] Add a check for INCREMENT_FLAGS in inx and dcx
- [ ] Delete the comment for inr and dnr that says its casted to 16 bit number
- [ ] Change the dad fucntion (answer >> 16) | 1 to (answer >> 16) & 1
- [ ] Replace stax with setMem(state, a, b, value) changed to the correct function name and arguments
- [ ] Add breaks in the switch statement to prevent fall through
- [ ] Lack of default case (Make one for unknown upcode)
- [ ] Add bounds checking for readByte and writeByte
- [ ] Remove the decrement for the pc in UnimplementedInstruction
