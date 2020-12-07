#include "consolerw.h"
#include <thread>
#include <iostream>
#include <atomic>
#include <boost/signals2.hpp>
#include <boost/algorithm/string.hpp>
//#include <conio.h>

using namespace std;

struct CommandReader::Private
{
	std::unique_ptr<thread> thr = nullptr;
	atomic_bool shouldStop{false};
	boost::signals2::signal<void(string command)> sendCommand;
};

CommandReader::CommandReader() :
	_p(make_unique<Private>())
{
}

CommandReader::~CommandReader()
{
}

void CommandReader::threadFunc()
{
	_p->shouldStop = false;
	string cmd;
	cout << ">";
	while (!_p->shouldStop) {
		if (getLineAsync(cin, cmd)) {
			// We have to do some minimal parsing right here because we need to process 'quit' command,
			// since there is no easy and portable way to stop reading from std::cin programmatically.

			vector<string> lst;
			boost::split(lst, cmd, [](char c) {return c == ' '; });

			if (lst.empty())
				return;

			for (string str : lst) {
				boost::trim(str);
				boost::to_lower(str);
			}

			if (lst.at(0) == "quit" && lst.size() == 1) {
				_p->shouldStop = true;
			}

			_p->sendCommand(cmd);

			cout << ">";
		}
	}
}

bool CommandReader::getLineAsync(std::istream& is, std::string& str, char delim)
{
	static std::string lineSoFar;
	char inChar;
	int charsRead = 0;
	bool lineRead = false;
	str = "";

	do {
		inChar = is.get();
		if (inChar == EOF)
			return false;

		if (inChar == delim) {
			str = lineSoFar;
			lineSoFar = "";
			lineRead = true;
		}
		else {
			lineSoFar.append(1, inChar);
		}
	} while (!lineRead && !_p->shouldStop);

	return lineRead;
}

bool CommandReader::start()
{
	if (_p->thr)
		return true;

	_p->thr = make_unique<thread>([=] { threadFunc(); });
}

void CommandReader::stop()
{
	if (!_p->thr)
		return;

	_p->shouldStop = true;
	_p->thr->join();

	return;
}

void CommandReader::addReceiver(function<void(const string&)> recvFunc)
{
	_p->sendCommand.connect(recvFunc);
}
