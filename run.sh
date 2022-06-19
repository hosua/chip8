# make
# ./CHIP8 "games/TEST_OP"
# ./CHIP8 "games/CONNECT4"

EXEC="CHIP8"

GAMES_DIR="games"
ls "games" | nl

GAME_FILES=("$GAMES_DIR"/*)
GAME_PATHS=()

echo "Enter a number to select the ROM to run"
read input
((input=input-1))
RUN_PATH="${GAME_FILES[$input]}"

echo $RUN_PATH

cmd="./$EXEC $RUN_PATH"

eval "$cmd"

