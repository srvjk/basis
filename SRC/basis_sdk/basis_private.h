#pragma once

#include "basis.h"
#include <map>
#include <boost/dll.hpp>

namespace Basis
{
	struct Module
	{
		std::string name;
		boost::dll::shared_library lib;
	};

	struct System::Private {
		/**
		* @brief ��������� ��������� ������.
		* @return ������ �� ��������� ��������� ������������ ������
		*/
		std::shared_ptr<Module> loadModule(const std::string& path);

		std::map<std::string, std::shared_ptr<Module>> modules; /// ����������� ������
	};
};