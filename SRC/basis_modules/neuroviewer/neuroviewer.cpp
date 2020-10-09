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

//void drawLabel(const point3d& p, const string& label)
//{
//	double offsetX = 5;
//	double offsetY = 5;
//
//	GLint viewport[4];
//	GLdouble modelViewMatrix[16];
//	GLdouble projectionMatrix[16];
//	GLdouble wx, wy, wz;
//
//	glGetIntegerv(GL_VIEWPORT, viewport);
//	glGetDoublev(GL_MODELVIEW_MATRIX, modelViewMatrix);
//	glGetDoublev(GL_PROJECTION_MATRIX, projectionMatrix);
//
//	gluProject(p.get<0>(), p.get<1>(), p.get<2>(), modelViewMatrix, projectionMatrix, viewport, &wx, &wy, &wz);
//	wy = viewport[3] - wy;
//
//	//painter->drawText(QPoint(wx + offsetX, wy - offsetY), sat.dispName);
//	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize;
//	ImGui::SetNextWindowPos(ImVec2(wx + offsetX, wy - offsetY));
//	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
//
//	if (!ImGui::Begin(label.c_str(), 0, window_flags)) {
//		ImGui::End();
//		return;
//	}
//
//	ImGui::Text(label.c_str());
//
//	ImGui::End();
//	ImGui::PopStyleColor();
//}

void drawLabel(Entity& ent)
{
	if (ent.name().empty())
		return;

	auto spatial = ent.as<Spatial>();
	if (!spatial)
		return;

	point3d p = spatial->position();

	double offsetX = 5;
	double offsetY = 5;

	GLint viewport[4];
	GLdouble modelViewMatrix[16];
	GLdouble projectionMatrix[16];
	GLdouble wx, wy, wz;

	glGetIntegerv(GL_VIEWPORT, viewport);
	glGetDoublev(GL_MODELVIEW_MATRIX, modelViewMatrix);
	glGetDoublev(GL_PROJECTION_MATRIX, projectionMatrix);

	gluProject(p.get<0>(), p.get<1>(), p.get<2>(), modelViewMatrix, projectionMatrix, viewport, &wx, &wy, &wz);
	wy = viewport[3] - wy;

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize;
	ImGui::SetNextWindowPos(ImVec2(wx + offsetX, wy - offsetY));
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));

	std::string idStr = boost::uuids::to_string(ent.id());
	if (!ImGui::Begin(idStr.c_str(), 0, window_flags)) {
		ImGui::End();
		return;
	}

	ImGui::Text(ent.name().c_str());

	ImGui::End();
	ImGui::PopStyleColor();
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

	double minDistance = 10;                /// минимальное расстояние от центра сцены в 3D
	double maxDistance = 1e3;               /// максимальное расстояние от центра сцены в 3D
	bool enableLighting = false;
	bool showAxes = false;
	GLfloat ambientLight[4] = { 0.6f, 0.6f, 0.6f, 0.6f };
	GLfloat sunLight[4] = { 0.6f, 0.6f, 0.6f, 0.6f };
	GLfloat sunPosition[4] = { 0.0f, 0.0f, 100.0f, 1.0f };
	point3d lookFrom = { -100, -100, 100 };
	point3d lookAt = { 100, 100, 0 };
	point3d up = { 0, 0, 1 };
	GLUquadricObj* quadric = nullptr;

	Color inactiveNeuronColor = { 0.7, 0.7, 0.7 };
	Color activeNeuronColor = { 1.0, 0.7, 0.7 };
	Color dummyLinkColor = { 0.1, 0.1, 0.1 };
	Color inactivePositiveLinkColor = { 0.2, 0.1, 0.1 };
	Color inactiveNegativeLinkColor = { 0.1, 0.1, 0.2 };
	Color activePositiveLinkColor = { 1.0, 0.4, 0.4 };
	Color activeNegativeLinkColor = { 0.4, 0.4, 1.0 };

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
	for (auto iter = sys->entityIterator(); iter.hasMore(); iter.next()) {
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
	}

	// 'Lighting' button
	color = _p->buttonOffColor;
	if (_p->enableLighting)
		color = _p->buttonOnColor;

	ImGui::PushStyleColor(ImGuiCol_Button, color);
	if (ImGui::Button("Lighting"))
		_p->enableLighting = !_p->enableLighting;
	ImGui::PopStyleColor();
	ImGui::SameLine();

	// 'Axes' button
	color = _p->buttonOffColor;
	if (_p->showAxes)
		color = _p->buttonOnColor;

	ImGui::PushStyleColor(ImGuiCol_Button, color);
	if (ImGui::Button("Axes"))
		_p->showAxes = !_p->showAxes;
	ImGui::PopStyleColor();
	ImGui::SameLine();

	if (_p->activeNet) {
		showActiveNetParams();
	}

	ImGui::End();
	ImGui::PopStyleColor();
}

void NeuroViewer::showStatistics()
{
	//ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize;
	ImGuiWindowFlags window_flags = 0;
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));

	if (!ImGui::Begin("Statistics", 0, window_flags)) {
		ImGui::End();
		return;
	}

	//ImGui::ColorEdit3("Act. excit. link: ");

	ImGui::End();
	ImGui::PopStyleColor();
}

void NeuroViewer::showActiveNetParams()
{
	ImGuiWindowFlags window_flags = 0;
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));

	if (!ImGui::Begin("Net", 0, window_flags)) {
		ImGui::End();
		return;
	}

	ImVec4 color;

	// 'Pause' button
	color = _p->buttonOffColor;
	if (_p->activeNet->isPaused())
		color = _p->buttonOnColor;
	ImGui::PushStyleColor(ImGuiCol_Button, color);
	if (ImGui::Button("Pause"))
		_p->activeNet->pause();
	ImGui::PopStyleColor();
	ImGui::SameLine();

	// 'Resume' button
	color = _p->buttonOnColor;
	ImGui::PushStyleColor(ImGuiCol_Button, color);
	if (ImGui::Button("Resume"))
		_p->activeNet->resume();
	ImGui::PopStyleColor();

	// activation threshold slider
	double thr = _p->activeNet->activationThreshold();
	ImGui::InputDouble("net act. threshold", &thr, 0.01f, 1.0f, "%.2f");
	_p->activeNet->setActivationThreshold(thr);

	// spontaneous neuron activity checkbox
	bool spAct = _p->activeNet->isSpontaneousActivityEnabled();
	ImGui::Checkbox("spontan. act.", &spAct);
	_p->activeNet->enableSpontaneousActivity(spAct);

	// neuron editor
	{
		static const int bufSize = 256;
		static char buf[bufSize] = "";
		static shared_ptr<Neuron> selectedNeuron = nullptr;
		ImGui::InputText("", buf, bufSize);
		ImGui::SameLine();
		if (ImGui::Button("find")) {
			string name = string(buf);
			for (auto entIter = _p->activeNet->entityIterator(); entIter.hasMore(); entIter.next()) {
				auto ent = entIter.value();
				auto neuron = ent->as<Neuron>();
				if (!neuron)
					continue;
				if (neuron->name() == name) {
					selectedNeuron = neuron;
					break;
				}
			}
		}

		if (selectedNeuron) {
			ImGui::Text(selectedNeuron->name().c_str());
			double thr = selectedNeuron->activationThreshold();
			ImGui::InputDouble("neur. act. threshold", &thr, 0.01f, 1.0f, "%.2f");
			selectedNeuron->setActivationThreshold(thr);
		}
	}

	ImGui::End();
	ImGui::PopStyleColor();
}

void NeuroViewer::showLessons()
{
	shared_ptr<Trainer> trainer;
	for (auto iter = sys->entityIterator(); iter.hasMore(); iter.next()) {
		auto ent = iter.value();
		trainer = ent->as<Trainer>();
		if (trainer)
			break;
	}

	if (!trainer)
		return;

	//ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize;
	ImGuiWindowFlags window_flags = 0;
	//ImGui::SetNextWindowPos(ImVec2());
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));

	if (!ImGui::Begin("Lessons", 0, window_flags)) {
		ImGui::End();
		return;
	}

	list<string> lessons = trainer->listLessons();

	ImVec4 color;
	for (string lesson: lessons) {
		color = _p->buttonOffColor;
		if (lesson == trainer->activeLesson())
			color = _p->buttonOnColor;

		ImGui::PushStyleColor(ImGuiCol_Button, color);
		if (ImGui::Button(lesson.c_str()))
			trainer->setActiveLesson(lesson);

		ImGui::PopStyleColor();
		ImGui::SameLine();
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

	if (_p->showAxes)
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
	for (auto entIter = net->entityIterator(); entIter.hasMore(); entIter.next()) {
		auto ent = entIter.value();
		auto neuron = ent->as<Neuron>();
		if (!neuron)
			continue;

		auto spatial = ent->as<Spatial>();
		if (spatial) {
			Color color = _p->inactiveNeuronColor;
			if (neuron->outValue() > 0.9)
				color = _p->activeNeuronColor;

			point3d pt = spatial->position();
			drawSphere(_p->quadric, pt, color, 10);

			//if (_p->showNames)
			drawLabel(*ent.get());
		}
	}

	// links
	for (auto entIter = net->entityIterator(); entIter.hasMore(); entIter.next()) {
		auto ent = entIter.value();
		auto lnk = ent->as<Link>();
		if (!lnk)
			continue;
		if (!lnk->srcNeuron)
			continue;
		if (!lnk->dstNeuron)
			continue;

		Color color = _p->dummyLinkColor;
		if (lnk->active) {
			if (lnk->type == LinkType::Positive)
				color = _p->activePositiveLinkColor;
			else if (lnk->type == LinkType::Negative)
				color = _p->activeNegativeLinkColor;
		}
		else {
			if (lnk->type == LinkType::Positive)
				color = _p->inactivePositiveLinkColor;
			else if (lnk->type == LinkType::Negative)
				color = _p->inactiveNegativeLinkColor;
		}

		for (int i = 1; i < lnk->path.size(); ++i) {
			drawLine(lnk->path[i - 1], lnk->path[i], color, 1);
		}
	}
}

void NeuroViewer::step()
{
	if (!_p->mainWnd) {
		int res = glfwInit();
		if (res == GLFW_TRUE) {
			_p->mainWnd = glfwCreateWindow(1024, 768, "NeuroViewer", nullptr, nullptr);
			if (!_p->mainWnd) {
				std::cout << "NeuroViewer failed to create window" << std::endl;
				const char* errDescr;
				int errCode = glfwGetError(&errDescr);
				std::cout << "Error " << errCode << ": " << errDescr << std::endl;
				glfwTerminate();
				return;
			}
		}
		else {
			std::cout << "NeuroViewer error: GLFW initialization failed with error " << res << std::endl;
			const char* errDescr;
			int errCode = glfwGetError(&errDescr);
			std::cout << "Error " << errCode << ": " << errDescr << std::endl;
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
	showStatistics();
	showLessons();
	processKeyboardEvents();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	glfwSwapBuffers(_p->mainWnd);
	glfwPollEvents();
}

//double length(const point3d& p)
//{
//	double x = p.get<0>();
//	double y = p.get<1>();
//	double z = p.get<2>();
//	return sqrt(x * x + y * y + z * z);
//}

void normalize(point3d& p)
{
	bg::divide_value(p, Basis::length(p));
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

