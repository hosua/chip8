output="CHIP8"
# compile objects
# g++ -I. -c chip8.cpp -o chip8.o
# g++ -I. -c cpu.cpp -o cpu.o 
# g++ -I. -c display.cpp -o display.o
# g++ -I. -c main.cpp -o main.o
# g++ *.o -o "$output" && ./"$output" "games/TEST_OP" # link obj files
# rm *.o # clean obj files

# lazy wildcard
g++ chip8.cpp -o "$output" && ./"$output" "../games/TEST_OP"
