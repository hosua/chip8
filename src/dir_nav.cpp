#include "dir_nav.h"

unsigned int depth = 0;

std::string SelectGame(std::string games_directory){
	std::vector<std::string> path_vect;
	for (const auto & entry : std::filesystem::recursive_directory_iterator(games_directory)){
		std::string entry_str = entry.path().string();
		// Keep track of directories by appending a '/' to the end of its name
		if (!std::filesystem::is_directory(entry))
			path_vect.push_back(entry_str);
		else {
			sort(path_vect.begin(), path_vect.end());
			path_vect.push_back(std::string(entry_str + "/"));
		}
	}

	// Enumerate directories/files
	int i = 1;
	for (auto itr = path_vect.begin(); itr != path_vect.end(); itr++, i++){
		// If  file
		if (itr->substr(itr->length()-1) != "/") {
			std::cout << i << ") " << itr->substr(itr->find_last_of("/")+1) << std::endl;	
		} else {
		// If a directory
			std::transform(itr->begin(), itr->end(), itr->begin(), ::toupper);
			std::cout << itr->substr(itr->substr(0, itr->size()-1).find_last_of("/")+1) << std::endl;
			// Don't count directory
			// i--; 
			path_vect.erase(itr);
		}
	}

	size_t num_games = path_vect.size();
	int input_selection = 0;
	while (input_selection < 1 || input_selection > num_games){
		std::cout << "Enter a number to select a game: " << std::endl;
		std::cin >> input_selection;
	}
	return path_vect[input_selection-1];
}

void SaveLastGamePlayed(std::string filename){
	std::fstream fstr;	
	fstr.open(filename, std::fstream::in | std::fstream::out | std::fstream::app);
	fstr.close();
}
