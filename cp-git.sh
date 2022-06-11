read -p "Are you sure? (y/n) " yn

case $yn in 
	y ) yes | cp * -r ~/Desktop/Git/Github/chip8/;;
	n ) echo exiting...;
		exit;;
	* ) echo invalid response;
		exit 1;;
esac
