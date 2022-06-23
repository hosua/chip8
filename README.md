# Issues
opcode test looks like it works, but it shows OK opcodes when they aren't implemented yet too.
Keyboard input not yet implemented

# How to use

If you do wish to use this currently broken emulator...

From the ``run.sh`` script help menu:
```
-c Compile Chip8 with make
-d Delete all build files (including executable) with make clean
-y Compile with compiledb script for YCM syntax completion/checking
-k Force kill CHIP8 
-h This help menu
```

If you run this script without arguments, it will let you select a game from the ``games`` directory and run the CHIP8 emulator. 
You must compile it with the ``-c`` flag first for it to work.

**This is still a work in progress**

If you cannot use the ``run.sh`` script for whatever reason, you can compile the program with ``make``,
and then run it with ``./CHIP8 "games/IBM"``, to run the IBM test ROM for example.

Currently, the IBM test ROM works, some games partially work, but obviously aren't playable until I figure out keyboard input.

Some opcodes are still broken/not implemented yet, keyboard input still also needs to be done as well.

To render the graphics, I am using SDL 2.0.

![SDL](images/SDL.png)

# Resources used
[devernay](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM)

[tobiasvl](https://tobiasvl.github.io/blog/write-a-chip-8-emulator/)

[austinmorlan](https://austinmorlan.com/posts/chip8_emulator/)

[codeslinger](http://www.codeslinger.co.uk/pages/projects/chip8.html)

[multigesture](https://multigesture.net/articles/how-to-write-an-emulator-chip-8-interpreter/)

[w&jdevschool](https://blog.wjdevschool.com/blog/video-game-console-emulator/)

[test-rom](https://github.com/corax89/chip8-test-rom)


