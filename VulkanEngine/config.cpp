#include "config.h"

std::vector<std::string> split(std::string line, std::string delimiter) {
	std::vector<std::string> split_line;
	std::string safe_line = line;

	int lastIndex = 0;
	int currentIndex = safe_line.find(delimiter, lastIndex);
	while (currentIndex != std::string::npos) {
		std::string splitString = safe_line.substr(lastIndex, currentIndex - lastIndex);
		split_line.push_back(splitString);
		lastIndex = currentIndex + 1;
		currentIndex = safe_line.find(delimiter, lastIndex);
	}

	split_line.push_back(safe_line.substr(lastIndex, std::string::npos));

	return split_line;
}