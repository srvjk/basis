#include "basis.h"
#include "basis_private.h"
#include <boost/filesystem.hpp>
#include <boost/dll.hpp>
#include <iostream>

using namespace Basis;
using namespace std;

namespace fs = boost::filesystem;

Entity::Entity() : _p(make_unique<Private>())
{
}

Entity::~Entity()
{
}

void Entity::setTypeId(tid typeId)
{
	_p->typeId = typeId;
}

tid Entity::typeId() const
{
	return _p->typeId;
}

Entity* Entity::addFacet(tid protoTypeId)
{
	System* system = System::instance(); // TODO ����� ���������� ��� ��������

	// ���������� ������ �������� �������� ���� protoTypeId (������� �����������, ��� ��� �����������)
	shared_ptr<Entity> prototype = system->findEntity([protoTypeId](Entity* ent) -> bool {
		return ((ent->typeId() == protoTypeId) && !ent->hasPrototype());
	});

	// ���� �������� �� ����������, �������� ������� ���
	if (!prototype)
		prototype = system->newEntity(protoTypeId);
	if (!prototype)
		return nullptr; // �� ������� ������� ��������

	// ����� �� �� ���������, ��� ����� ����� ��� �� ����������,
	// ��������� ��������� ����� ��������� ������ ������ ���� (��������,
	// ������� ����� ���� ����������� ����� ���� ��������)
	auto newFct = System::instance()->newEntity(prototype.get());
	if (!newFct)
		return nullptr;

	_p->facets.push_back(newFct);
	return newFct.get();
}

Entity* Entity::addFacet(Entity* prototype)
{
	// ����� �� �� ���������, ��� ����� ����� ��� �� ����������,
	// ��������� ��������� ����� ��������� ������ ������ ���� (��������,
	// ������� ����� ���� ����������� ����� ���� ��������)
	auto newFct = System::instance()->newEntity(prototype);
	if (!newFct)
		return nullptr;

	_p->facets.push_back(newFct);
	return newFct.get();
}

bool Entity::hasPrototype() const
{
	return (_p->prototype != nullptr);
}

Executable::Executable() : _p(make_unique<Private>())
{
}

Executable::~Executable()
{
}

void Executable::step()
{
	if (_p->stepFunction)
		_p->stepFunction();
}

void Executable::setStepFunction(std::function<void()> func)
{
	_p->stepFunction = func;
}

System::System() : _p(make_unique<Private>())
{
	// ����������� ��������� ���������
	registerEntity<Executable>();
}

System::~System()
{
}

int System::loadModules(const string& path, bool recursive)
{
	fs::path p(path);
	if (!fs::exists(p)) {
		cout << path << "path does not exist: " << path << endl;
		return 0;
	}

	if (fs::is_regular_file(p)) {
		if (_p->loadModule(p.generic_string())) {
			return 1;
		}
	}
	else if (fs::is_directory(p)) {
		int res = 0;
		for (fs::directory_entry& entry : fs::directory_iterator(p)) {
			if (recursive) {
				res += loadModules(entry.path().generic_string(), recursive);
			}
			else {
				if (_p->loadModule(entry.path().generic_string()))
					res++;
			}
		}

		return res;
	}

	return 0;
}

bool System::isEntityRegistered(tid typeId) const
{
	if (_p->factories.count(typeId) > 0)
		return true;

	return false;
}

int64_t System::entityTypesCount() const
{
	return _p->factories.size();
}

shared_ptr<Entity> System::createEntity(tid typeId)
{
	auto iter = _p->factories.find(typeId);
	if (iter == _p->factories.end())
		return nullptr;

	FactoryInterface* factory = iter->second.get();
	if (!factory)
		return nullptr;

	shared_ptr<Entity> ent = shared_ptr<Entity>(factory->newEntity());

	return ent;
}

shared_ptr<Entity> System::newEntity(tid typeId)
{
	shared_ptr<Entity> ent = findEntity([typeId](Entity* ent) -> bool {
		return ((ent->typeId() == typeId) && !ent->hasPrototype());
	});
	if (ent)
		return ent; 

	ent = createEntity(typeId);
	if (ent)
		_p->entities.push_back(ent);

	return ent;
}

std::shared_ptr<Entity> System::newEntity(Entity* prototype)
{
	if (!prototype)
		return nullptr;

	shared_ptr<Entity> ent = createEntity(prototype->typeId());
	if (!ent)
		return nullptr;

	ent->_p->prototype = prototype;
	_p->entities.push_back(ent);

	return ent;
}

std::shared_ptr<Entity> System::findEntity(std::function<bool(Entity*)> match)
{
	for (auto i = _p->entities.begin(); i != _p->entities.end(); ++i) {
		if (match(i->get()))
			return *i;
	}

	return nullptr;
}

std::vector<std::shared_ptr<Entity>> System::findEntities(std::function<bool(Entity*)> match)
{
	vector<shared_ptr<Entity>> result;
	for (auto i = _p->entities.begin(); i != _p->entities.end(); ++i) {
		if (match(i->get()))
			result.push_back(*i);
	}

	return result;
}

bool System::addFactory(FactoryInterface* f)
{
	_p->factories[f->typeId()] = std::shared_ptr<FactoryInterface>(f);

	return true;
}

std::shared_ptr<Module> System::Private::loadModule(const std::string& path)
{
	shared_ptr<Module> retval = nullptr;

	cout << "loading '" << path << "'" << endl;

	fs::path p(path);
	if (!fs::exists(p)) {
		cout << "cannot load module: path '" << path << "' not exists" << endl;
		return retval;
	}
	if (!fs::is_regular_file(p)) {
		cout << "cannot load module: path '" << path << "' is not a file" << endl;
		return retval;
	}

	// �������� �� DLL:
	try {
		boost::dll::library_info info(path, true);
	}
	catch (boost::exception& e) {
		cout << path << " is not a module" << endl;
		return retval;
	}

	string moduleName = p.stem().generic_string();

	if (modules.find(moduleName) != modules.end())
		cout << "already loaded: " << moduleName << endl;

	auto module = make_shared<Module>();
	module->name = moduleName;

	try {
		module->lib = boost::dll::shared_library(path);
		module->setup_func = module->lib.get<void(Basis::System*)>("setup");
	}
	catch (boost::system::system_error& err) {
		cout << err.what() << endl;
		return retval;
	}

	module->setup_func(System::instance());
	modules[moduleName] = module;

	cout << "module '" << module->name << "' successfully loaded." << endl;

	return module;
}

