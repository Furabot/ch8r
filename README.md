# ch8r
A CHIP-8 interpreter written in C++, with graphics and input handled by SDL2. Runs most CHIP-8 binaries in a playable state.

## Build Instructions
Run the following commands to compile and link on Windows
```
g++ -c main.cpp chip8.cpp -I"<Path to the include directory of SDL2>"

g++ -o main.exe main.o chip8.o -L"<Path to the lib directory of SDL2>" -lmingw32 -lSDL2main -lSDL2
```
## Run Instructions
Run this command in the terminal
```
./<Path to ch8r.exe> <Path to the CHIP-8 ROM>
```

Alternatively, drag-and-drop the CHIP-8 ROM onto the compiled binary.

## References
Special thanks to the following resources

[Laurence Muller's CHIP-8 guide](https://multigesture.net/articles/how-to-write-an-emulator-chip-8-interpreter)

[Cowgod's CHIP-8 Technical Reference](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM)

[Lazy Foo's SDL Tutorials](https://lazyfoo.net/tutorials/SDL)
