#ifndef DIR_NAV_H
#define DIR_NAV_H

#include <iostream>
#include <vector>
#include <list>
#include <filesystem>
#include <algorithm>

#define DEFAULT_GAMES_DIR "GAMES"

#ifdef _WIN32
#define DIR_SEP "\\"
#else
#define DIR_SEP "/"
#endif // _WIN32

std::string SelectGame(std::string games_directory);

#endif // DIR_NAV_H
