#include "neuroviewer.h"
#include <iostream>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <boost/uuid/uuid_io.hpp>
#include "neuro/neuro.h"
#include <functional>

using namespace std;
using namespace Basis;

static Basis::System* sys = nullptr;
using Color = point3d;

void drawSphere(GLUquadricObj* quadric, const point3d& p, const Color& color, float size)
{
	float hsize = size / 2.0;

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix(); // сохраняем мировую матрицу, чтобы не повлиять на другие объекты

	glColor3f(color.get<0>(), color.get<1>(), color.get<2>());
	glTranslatef(p.get<0>(), p.get<1>(), p.get<2>());

	gluSphere(quadric, hsize, 16, 16);

	glPopMatrix();
}

struct WindowData
{
	int width = 1024; /// текущая ширина окна приложения
	int height = 786; /// текущая высота окна приложения
};

static WindowData winData;

void windowPosCallback(GLFWwindow* window, int x, int y)
{
	//Mosaic::System* system = Mosaic::System::instance();
	//std::shared_ptr<MosaicSettingsImpl> settings = std::dynamic_pointer_cast<MosaicSettingsImpl>(system->settings());
	//if (!settings)
	//	return;

	//settings->winPosX = x;
	//settings->winPosY = y;
}

void windowSizeCallback(GLFWwindow* window, int width, int height)
{
	winData.width = width;
	winData.height = height;
}

void setProjection(GLFWwindow* window)
{
	int w = winData.width;
	int h = winData.height;
	if (h == 0)
		h = 1;
	GLfloat aspect = (GLfloat)w / (GLfloat)h;
	GLfloat minZ = -1.0;
	GLfloat maxZ = 1.0;

	glfwMakeContextCurrent(window);
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (w >= h)
		gluOrtho2D(minZ * aspect, maxZ * aspect, minZ, maxZ);
	else
		gluOrtho2D(minZ, maxZ, minZ / aspect, maxZ / aspect);
}

Polar::Polar()
{
}

Polar::Polar(double phi, double theta, double r)
{
	this->phi = phi;
	this->theta = theta;
	this->r = r;
}

struct NeuroViewer::Private
{
	ImVec4 clearColor = ImVec4(0.1f, 0.1f, 0.1f, 1.00f);
	ImVec4 buttonOnColor = ImVec4(78 / 256.0, 201 / 256.0, 176 / 256.0, 1.00f);
	ImVec4 buttonOffColor = ImVec4(10 / 256.0, 10 / 256.0, 10 / 256.0, 1.00f);
	GLFWwindow* mainWnd = nullptr;
	ViewingMode viewingMode = ViewingMode::List;
	shared_ptr<NeuroNet> activeNet = nullptr;
	shared_ptr<Trainer> selectedTrainer = nullptr; /// тренер, выбранный для просмотра и редактирования в данный момент

	double minDistance = 10;                /// минимальное расстояние от центра сцены в 3D
	double maxDistance = 1e3;               /// максимальное расстояние от центра сцены в 3D
	bool enableLighting = false;
	GLfloat ambientLight[4] = { 0.6f, 0.6f, 0.6f, 0.6f };
	GLfloat sunLight[4] = { 0.6f, 0.6f, 0.6f, 0.6f };
	GLfloat sunPosition[4] = { 0.0f, 0.0f, 100.0f, 1.0f };
	point3d lookFrom = { -100, -100, 100 };
	point3d lookAt = { 100, 100, 0 };
	point3d up = { 0, 0, 1 };
	GLUquadricObj* quadric = nullptr;

	Color inactiveNeuronColor = { 0.7, 0.7, 0.7 };
	Color activeNeuronColor = { 1.0, 0.7, 0.7 };
	Color inactiveLinkColor = { 0.3, 0.3, 0.3 };
	Color activeLinkColor = { 1.0, 0.2, 0.2 };

	Private() 
	{
		quadric = gluNewQuadric();
	}
};

NeuroViewer::NeuroViewer(Basis::System* sys) :
	Basis::Entity(sys),
	_p(std::make_unique<Private>())
{
	auto exe = addFacet<Basis::Executable>();
	if (exe)
		exe->setStepFunction(std::bind(&NeuroViewer::step, this));
}

NeuroViewer::~NeuroViewer()
{
}

bool NeuroViewer::init()
{
	return true;
}

void NeuroViewer::cleanup()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	if (_p->mainWnd)
		glfwDestroyWindow(_p->mainWnd);
	glfwTerminate();
}

void NeuroViewer::showMainToolbar()
{
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize;
	ImGui::SetNextWindowPos(ImVec2());
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));

	if (!ImGui::Begin("Magic Eye", 0, window_flags)) {
		ImGui::End();
		return;
	}

	ImVec4 color;

	// look for entities we know how to deal with
	for (auto iter = sys->entityIteratorNew(); iter.hasMore(); iter.next()) {
		auto ent = iter.value();
		auto net = ent->as<NeuroNet>();
		if (net) {
			color = _p->buttonOffColor;
			if (_p->activeNet) {
				if (_p->activeNet.get() == net.get())
					color = _p->buttonOnColor;
			}

			ImGui::PushStyleColor(ImGuiCol_Button, color);
			if (ImGui::Button(net->name().c_str())) {
				if (_p->activeNet.get() != net.get())
					_p->activeNet = net;
				else
					_p->activeNet = nullptr;
			}
			ImGui::PopStyleColor();
			ImGui::SameLine();
		}

		auto trainer = ent->as<Trainer>();
		if (trainer) {
			color = _p->buttonOffColor;
			if (trainer->isActive())
				color = _p->buttonOnColor;

			ImGui::PushStyleColor(ImGuiCol_Button, color);
			if (ImGui::Button(trainer->name().c_str()))
				trainer->setActive(!trainer->isActive());

			ImGui::PopStyleColor();
			ImGui::SameLine();

			if (trainer->isActive())
				_p->selectedTrainer = trainer;
		}
	}

	// 'Lighting' button
	color = _p->buttonOffColor;
	if (_p->enableLighting)
		color = _p->buttonOnColor;

	ImGui::PushStyleColor(ImGuiCol_Button, color);
	if (ImGui::Button("Lighting"))
		_p->enableLighting = !_p->enableLighting;
	ImGui::PopStyleColor();

	ImGui::End();
	ImGui::PopStyleColor();
}

void NeuroViewer::showLessons()
{
	if (!_p->selectedTrainer)
		return;

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize;
	ImGui::SetNextWindowPos(ImVec2());
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));

	if (!ImGui::Begin("Lessons", 0, window_flags)) {
		ImGui::End();
		return;
	}

	ImVec4 color;
	auto trainer = _p->selectedTrainer;
	for (auto iter = trainer->entityIteratorNew(); iter.hasMore(); iter.next()) {
		auto ent = iter.value();
		auto lesson = ent->as<Lesson>();
		if (lesson) {
			color = _p->buttonOffColor;
			if (lesson->active)
				color = _p->buttonOnColor;

			ImGui::PushStyleColor(ImGuiCol_Button, color);
			if (ImGui::Button(lesson->name().c_str()))
				lesson->active = !lesson->active;

			ImGui::PopStyleColor();
			ImGui::SameLine();
		}
	}

	ImGui::End();
	ImGui::PopStyleColor();
}

void drawLine(const point3d& p1, const point3d& p2, const Color& color, int width)
{
	glDisable(GL_LIGHTING);
	glLineWidth(1);

	glBegin(GL_LINES);

	glColor3f(color.get<0>(), color.get<1>(), color.get<2>());
	glVertex3d(p1.get<0>(), p1.get<1>(), p1.get<2>());
	glVertex3d(p2.get<0>(), p2.get<1>(), p2.get<2>());

	glEnd();
}

void drawAxes(double spread, bool singleColor)
{
	glDisable(GL_LIGHTING);
	glLineWidth(1);
	if (singleColor)
		glColor3f(1.0, 1.0, 1.0);

	glBegin(GL_LINES);

	// Ось X
	if (!singleColor)
		glColor3f(0.5, 0.0, 0.0); // красный
	glVertex3d(0.0, 0.0, 0.0);
	glVertex3d(spread, 0.0, 0.0);

	// Ось Y
	if (!singleColor)
		glColor3f(0.0, 0.5, 0.0); // зеленый
	glVertex3d(0.0, 0.0, 0.0);
	glVertex3d(0.0, spread, 0.0);

	// Ось Z
	if (!singleColor)
		glColor3f(0.0, 0.0, 0.5); // синий
	glVertex3d(0.0, 0.0, 0.0);
	glVertex3d(0.0, 0.0, spread);

	glEnd();
}

void NeuroViewer::drawScene()
{
	glClearColor(0.0, 0.0, 0.0, 0.0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	glViewport(0, 0, (GLsizei)winData.width, (GLsizei)winData.height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, (GLfloat)winData.width / (GLfloat)winData.height,
		_p->minDistance, _p->maxDistance);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	gluLookAt(
		_p->lookFrom.get<0>(), _p->lookFrom.get<1>(), _p->lookFrom.get<2>(),
		_p->lookAt.get<0>(), _p->lookAt.get<1>(), _p->lookAt.get<2>(),
		_p->up.get<0>(), _p->up.get<1>(), _p->up.get<2>()
	);

	//if (s->showAxes)
		drawAxes(1000, false);

	if (_p->enableLighting) {
		glEnable(GL_LIGHTING);
		glLightfv(GL_LIGHT1, GL_AMBIENT, _p->ambientLight);
		glLightfv(GL_LIGHT1, GL_DIFFUSE, _p->sunLight);
		glLightfv(GL_LIGHT1, GL_POSITION, _p->sunPosition);
		glEnable(GL_LIGHT1);
	}
	else {
		glDisable(GL_LIGHTING);
	}

	drawActiveNet();
}

void NeuroViewer::drawActiveNet()
{
	if (!_p->activeNet)
		return;

	shared_ptr<NeuroNet> net = _p->activeNet;

	// neurons
	for (auto entIter = net->entityIteratorNew(); entIter.hasMore(); entIter.next()) {
		auto ent = entIter.value();
		auto neuron = ent->as<Neuron>();
		if (!neuron)
			continue;

		auto spatial = ent->as<Spatial>();
		if (spatial) {
			Color color = _p->inactiveNeuronColor;
			if (neuron->value() > 0.9)
				color = _p->activeNeuronColor;

			drawSphere(_p->quadric, spatial->position(), color, 10);
		}
	}

	// links
	for (auto entIter = net->entityIteratorNew(); entIter.hasMore(); entIter.next()) {
		auto ent = entIter.value();
		auto lnk = ent->as<Link>();
		if (!lnk)
			continue;
		if (!lnk->srcNeuron)
			continue;
		if (!lnk->dstNeuron)
			continue;

		point3d p1;
		point3d p2;
		auto spatial = lnk->srcNeuron->as<Spatial>();
		p1 = spatial->position();
		spatial = lnk->dstNeuron->as<Spatial>();
		p2 = spatial->position();

		Color color = _p->inactiveLinkColor;
		if (lnk->active)
			color = _p->activeLinkColor;
		drawLine(p1, p2, color, 1);
	}


	//auto entIter = net->entityIterator();
	//while (!entIter->finished()) {
	//	auto ent = entIter->value();
	//	if (ent->typeId() == TYPEID(Neuron)) {
	//		auto spatial = ent->as<Basis::Spatial>();
	//		if (spatial) {
	//			Color color = _p->inactiveNeuronColor;
	//			if (neuron->value() > 0.9)
	//				color = _p->activeNeuronColor;

	//			drawSphere(_p->quadric, spatial->position(), color, 10);
	//		}
	//	}

	//	neurIter->next();
	//}

	//int i = 0;
	//for (auto entPtr = cont->entities(); entPtr->finished() == false; entPtr->next()) {
	//	shared_ptr<Entity> ent = entPtr->value();
	//	if (ent->typeId() != TYPEID(Neuron))
	//		continue;

	//	auto neuron = static_pointer_cast<Neuron>(ent);
	//	auto spatPtr = neuron->facets(TYPEID(Basis::Spatial));
	//	if (!spatPtr)
	//		continue;

	//	shared_ptr<Basis::Spatial> spat = static_pointer_cast<Basis::Spatial>(spatPtr->value());
	//	if (!spat)
	//		continue;

	//	Color color = _p->inactiveNeuronColor;
	//	if (neuron->value() > 0.9)
	//		color = _p->activeNeuronColor;

	//	drawSphere(_p->quadric, spat->position(), color, 10);
	//	++i;
	//}
}

void NeuroViewer::step()
{
	if (!_p->mainWnd) {
		if (glfwInit()) {
			_p->mainWnd = glfwCreateWindow(1024, 768, "NeuroViewer", nullptr, nullptr);
			if (!_p->mainWnd) {
				glfwTerminate();
				return;
			}
		}
		else {
			std::cout << "Error: GLFW initialization failed!" << std::endl;
			return;
		}

		glfwMakeContextCurrent(_p->mainWnd);

		glfwSetWindowSizeCallback(_p->mainWnd, windowSizeCallback);
		glfwSetWindowPosCallback(_p->mainWnd, windowPosCallback);

		glfwSwapInterval(1);
		glewInit();
		ImGui::CreateContext();

		if (!ImGui_ImplGlfw_InitForOpenGL(_p->mainWnd, true)) {
			printf("Error: could not initialize Gui renderer");
			glfwTerminate();
			return;
		}
		if (!ImGui_ImplOpenGL3_Init()) {
			printf("Error: could not initialize Gui renderer (OpenGL 3)");
			ImGui_ImplGlfw_Shutdown();
			glfwTerminate();
			return;
		}

		ImGuiIO& io = ImGui::GetIO();

		if (!_p->mainWnd)
			return; // не удалось создать окно
	}

	setProjection(_p->mainWnd);

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// all painting here
	drawScene();
	showMainToolbar();
	showLessons();
	processKeyboardEvents();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	glfwSwapBuffers(_p->mainWnd);
	glfwPollEvents();
}

double length(const point3d& p)
{
	double x = p.get<0>();
	double y = p.get<1>();
	double z = p.get<2>();
	return sqrt(x * x + y * y + z * z);
}

void normalize(point3d& p)
{
	bg::divide_value(p, length(p));
}

Polar rectangularToPolar(const point3d& p)
{
	double x = p.get<0>();
	double y = p.get<1>();
	double z = p.get<2>();

	double phi = 0;
	double theta = 0;
	double r = sqrt(x * x + y * y + z * z);

	if (x == 0 && y == 0)
		phi = 0;
	else
		phi = atan2(y, x);

	if (z == 0 && r == 0)
		theta = 0;
	else
		theta = atan2(z, sqrt(x * x + y * y));

	return Polar(phi, theta, r);
}

point3d polarToRectangular(const Polar& p)
{
	point3d res;
	res.set<0>(p.r * cos(p.theta) * cos(p.phi));
	res.set<1>(p.r * cos(p.theta) * sin(p.phi));
	res.set<2>(p.r * sin(p.theta));

	return res;
}

void NeuroViewer::rotateCameraAroundViewpointLR(double angle)
{
	point3d v = _p->lookFrom;
	bg::subtract_point(v, _p->lookAt); // то же, что -(_p->lookAt - _p->lookFrom)

	Polar p = rectangularToPolar(v);
	p.phi += angle;
	double cyc = p.phi / PI2;
	if (cyc > 1.0)
		p.phi -= floor(cyc) * PI2;
	if (cyc < -1.0)
		p.phi += floor(abs(cyc)) * PI2;

	v = polarToRectangular(p);
	bg::multiply_value(v, -1);

	_p->lookFrom = _p->lookAt;
	bg::subtract_point(_p->lookFrom, v);
}

void NeuroViewer::rotateCameraAroundViewpointUD(double angle)
{
	point3d v = _p->lookFrom;
	bg::subtract_point(v, _p->lookAt); // то же, что -(_p->lookAt - _p->lookFrom)

	Polar p = rectangularToPolar(v);
	p.theta += angle;
	if (p.theta > PIdiv2)
		p.theta = PIdiv2;
	if (p.theta < -PIdiv2)
		p.theta = -PIdiv2;
	v = polarToRectangular(p);
	bg::multiply_value(v, -1);

	_p->lookFrom = _p->lookAt;
	bg::subtract_point(_p->lookFrom, v);
}


void NeuroViewer::zoom(double dist)
{
	point3d v = _p->lookAt;
	bg::subtract_point(v, _p->lookFrom);
	normalize(v);
	bg::multiply_value(v, dist);
	bg::add_point(_p->lookFrom, v);
}

void NeuroViewer::processKeyboardEvents()
{
	ImGuiIO& io = ImGui::GetIO();

	double rotAngle = 0.01; // радианы
	double dist = 10;

	for (int i = 0; i < IM_ARRAYSIZE(io.KeysDown); ++i) {
		if (ImGui::IsKeyPressed(i)) {
			if (io.KeyShift) { // Shift
				rotAngle = 0.1;
				dist = 100;
			}

			switch (i) {
			case 262: // стрелка вправо
			case 326:
				rotateCameraAroundViewpointLR(-rotAngle);
				break;
			case 263: // стрелка влево
			case 324:
				rotateCameraAroundViewpointLR(rotAngle);
				break;
			case 264: // стрелка вниз
			case 322:
				rotateCameraAroundViewpointUD(-rotAngle);
				break;
			case 265: // стрелка вверх
			case 328:
				rotateCameraAroundViewpointUD(rotAngle);
				break;
			case 61: // знак +
			case 334:
				zoom(dist);
				break;
			case 45: // знак -
			case 333:
				zoom(-dist);
				break;
			}
		}
	}
}

void setup(Basis::System* s)
{
	std::cout << "NeuroViewer::setup()" << endl;

	sys = s;
	sys->registerEntity<NeuroViewer>();
	//auto ent = sys->container()->newPrototype<NeuroViewer>();
	//ent->setName("MagicEye");
}

