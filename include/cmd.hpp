#ifndef CMD_HPP
#define CMD_HPP

#include <string>
#include <array>
#ifdef WIN32
	
#else
	#include <cstdlib>
	#include <cstdio>
#endif //include

std::string execute(std::string command)
{
	std::array<char, 128> buffer;
    std::string result;

    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe)
        return "";
    while(fgets(buffer.data(), 128, pipe) != NULL) {
        result += buffer.data();
    }
    pclose(pipe);
	
	return result;
}

#endif //CMD_HPP
