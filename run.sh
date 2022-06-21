#!/bin/sh

EXEC="CHIP8"

function usage(){
	echo "-c Compile Chip8 with make
-d Delete all build files (including executable) with make clean
-y Compile with compiledb script for YCM syntax completion/checking
-k Force kill CHIP8 
-h This help menu

If you run this script without arguments, it will let you select a game and run the CHIP8 emulator. 
You must compile it with the -c flag first for it to work."
}


while getopts "cdyhkt" o; do
    case "${o}" in
		# compile
        c)
			make
			;;
		d) 
			make clean
			;;
		y)
			compiledb make
			;;
		k)
			killall -9 CHIP8
			echo "Force killed CHIP8"
			exit
			;;
		t)
			./$EXEC "games/TEST_OP"
			exit
			;;
		h)
			usage
			;;
		*)
			usage
			;;
    esac
done

if [ -z "$(ls "$EXEC")" ]; then
	usage
	echo "Exiting because the executable was not in the directory."
	exit
fi

GAMES_DIR="games"
ls "games" | nl

GAME_PATHS=("$GAMES_DIR"/*)

echo "Enter a number to select the ROM to run"
read input
((input=input-1))
RUN_PATH="${GAME_PATHS[$input]}"

echo $RUN_PATH

cmd="./$EXEC $RUN_PATH"

eval "$cmd"

