#!/bin/sh

EXEC="CHIP8"

function usage(){
echo "-c Compile Chip8 with make
-d Delete all build files (including executable) with make clean
-g Run in debug mode with gdb
-e Exit
-k Force kill CHIP8 
-y Compile with compiledb script for YCM syntax completion/checking
-t Run optest
-h This help menu

If you run this script without arguments, it will let you select a game and run the CHIP8 emulator. 
You must compile it with the -c flag first for it to work."
}

use_gdb="false"

num_cycles=0
while getopts "cedyhktg:" o; do
    case "${o}" in
		# compile
        c) make ;;
		d) make clean ;;
		g) use_gdb="true" 
			num_cycles=${OPTARG}
			;;
		y) compiledb make ;;
		k) killall -9 CHIP8
			echo "Force killed CHIP8"
			;;
		e) exit ;;
		t) ./$EXEC "games/TEST_OP" exit ;;
		h) usage; exit ;;
		*) usage; exit ;;
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

if [ $use_gdb == "true" ]; then
	cmd="$cmd -d $num_cycles"
	cmd="gdb --args $cmd";
fi

eval "$cmd"

