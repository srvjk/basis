#pragma once

#include "basis.h"
#include <map>
#include <boost/dll.hpp>
#include <boost/function.hpp>
#include <functional>

namespace Basis
{
	struct Module
	{
		std::string name;
		boost::dll::shared_library lib;
		boost::function<void(Basis::System*)> setup_func;
	};

	struct Entity::Private
	{
		tid typeId;                             /// ��� ��������
		Entity* prototype = nullptr;            /// ������ �� ��������
		std::vector<std::shared_ptr<Entity>> facets; /// ����� ���� ��������
	};

	struct Executable::Private
	{
		std::function<void()> stepFunction = nullptr; /// �������, ���������� ������ step()
	};

	struct System::Private {
		/**
		* @brief ��������� ��������� ������.
		* @return ������ �� ��������� ��������� ������������ ������
		*/
		std::shared_ptr<Module> loadModule(const std::string& path);

		std::map<std::string, std::shared_ptr<Module>> modules;     /// ����������� ������
		std::map<tid, std::shared_ptr<FactoryInterface>> factories; /// ������� ���������
		std::vector<std::shared_ptr<Entity>> entities;              /// ��������
	};
};