#pragma once

#include <memory>
#include <functional>
#include <string>

class CommandReader
{
	struct Private;

public:
	CommandReader();
	~CommandReader();
	bool start();
	void stop();
	void addReceiver(std::function<void(const std::string&)> recvFunc);

private:
	void threadFunc();
	bool getLineAsync(std::istream& is, std::string& str, char delim = '\n');

private:
	std::unique_ptr<Private> _p;
};

class Console
{
	struct Private;

public:
	Console();
	~Console();
	bool start();
	void stop();

private:
	void threadFunc();

private:
	std::unique_ptr<Private> _p;
};