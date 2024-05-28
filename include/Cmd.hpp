#ifndef CMD_HPP
#define CMD_HPP

#include "Process.hpp"

class Cmd
{
	public:
		Cmd()
		{
			restart();
		}
		void restart()
		{
			process.startProcess((char*)"cmd.exe");
			Sleep(150);
			process.readFromProcess();
		}
		std::string execute(std::string command)
		{
			process.writeToProcess(command + '\n');
			std::string res = process.readFromProcess();

			int index = res.find('\n');
			if(index != -1 && index != ((int)res.size()-1))
				res = res.substr(index+1);
			index = res.rfind('\n');
			if(index != -1)
				res = res.substr(0, index);

			return res;
		}
		void close()
		{
			process.closeProcess();
		}
	private:
		Process process;
};

#endif // !CMD_HPP
