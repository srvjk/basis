#pragma once

#include "basis.h"
#include <boost/system/system_error.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#ifdef PLATFORM_WINDOWS
#  ifdef AQUACONTROLLER_LIB
#    define MODULE_EXPORT __declspec(dllexport)
#  else
#    define MODULE_EXPORT __declspec(dllimport)
#  endif
#else
#  define MODULE_EXPORT
#endif

class MODULE_EXPORT SerialReader
{
	struct Private;

	enum class ReadResult
	{
		InProgress,
		Success,
		Error,
		TimeoutExpired
	};

public:
	SerialReader();
	~SerialReader();
	void open(const std::string& devName);
	void close();
	bool isOpen() const;
	void setTimeout(const boost::posix_time::time_duration& t);
	void writeString(const std::string& s);
	bool readStringUntil(std::string& result, const std::string& delim = "\n");

private:
	void setupRead();
	void timeoutExpired(const boost::system::error_code& error);
	void readCompleted(const boost::system::error_code& error, const size_t bytesTransferred);

private:
	std::unique_ptr<Private> _p;
};

struct TimePoint
{
	float value;
	boost::posix_time::ptime time;

	TimePoint(double v, const boost::posix_time::ptime& t);
};

/// @brief Массив значений с метками времени.
class MODULE_EXPORT TimeValueArray : public Basis::Entity
{
	struct Private;

public:
	TimeValueArray(Basis::System* s);
	~TimeValueArray();
	void addValue(double value, const boost::posix_time::ptime& tm);
	const std::vector<TimePoint>& getData() const;

private:
	std::unique_ptr<Private> _p;
};

class MODULE_EXPORT AquaController : public Basis::Entity
{
	struct Private;

public:
	AquaController(Basis::System* s);
	~AquaController();
	void step();
	void reset();
	void readDataFromController();
	void sendCommand(const std::string& cmd);
	double getDoubleParam(const std::string& name, bool* ok = nullptr) const;
	int32_t getInt32Param(const std::string& name, bool* ok = nullptr) const;
	void switchFilter(bool on = true);
	bool isFilterOn() const;

private:
	void serialWorker();

private:
	std::unique_ptr<Private> _p;
};

/// @brief График массива значений.
class MODULE_EXPORT TimeValuePlot : public Basis::Entity
{
	struct Private;
	friend class AquaViewer;

public:
	TimeValuePlot(Basis::System* s);
	~TimeValuePlot();
	void setLineColor(int r, int g, int b);
	void setRect(float left, float top, float width, float height);
	void setValueRange(float minValue, float maxValue);
	void setMaxValuesVisible(int n);
	void drawTimeValues(const std::shared_ptr<TimeValueArray> arr);

private:
	std::unique_ptr<Private> _p;
};

class MODULE_EXPORT AquaViewer : public Basis::Entity
{
	struct Private;

public:
	AquaViewer(Basis::System* s);
	void step();

private:
	std::unique_ptr<Private> _p;
};

extern "C" MODULE_EXPORT void setup(Basis::System* s);