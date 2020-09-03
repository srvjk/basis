#include "basiscontrol.h"
#include <iostream>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <boost/uuid/uuid_io.hpp>

using namespace std;
using namespace Basis;

static Basis::System* sys = nullptr;

struct WindowData
{
	int width = 1024; /// текущая ширина окна приложения
	int height = 786; /// текущая высота окна приложения
};

static WindowData winData;

void windowPosCallback(GLFWwindow* window, int x, int y)
{
}

void windowSizeCallback(GLFWwindow* window, int width, int height)
{
	winData.width = width;
	winData.height = height;
}

struct BasisControl::Private
{
	ImVec4 clearColor = ImVec4(0.1f, 0.1f, 0.1f, 1.00f);
	ImVec4 buttonOnColor = ImVec4(78 / 256.0, 201 / 256.0, 176 / 256.0, 1.00f);
	ImVec4 buttonOffColor = ImVec4(10 / 256.0, 10 / 256.0, 10 / 256.0, 1.00f);
	GLFWwindow* mainWnd = nullptr;

	Private()
	{
	}
};

BasisControl::BasisControl(Basis::System* sys) :
	Basis::Entity(sys),
	_p(std::make_unique<Private>())
{
	auto exe = addFacet<Basis::Executable>();
	if (exe)
		exe->setStepFunction(std::bind(&BasisControl::step, this));
}

BasisControl::~BasisControl()
{
}

void BasisControl::step()
{
	if (!_p->mainWnd) {
		int res = glfwInit();
		if (res == GLFW_TRUE) {
			_p->mainWnd = glfwCreateWindow(1024, 768, "BasisControl", nullptr, nullptr);
			if (!_p->mainWnd) {
				glfwTerminate();
				return;
			}
		}
		else {
			std::cout << "BasisControl error: GLFW initialization failed with error " << res << std::endl;
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

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// all painting here
	glfwMakeContextCurrent(_p->mainWnd);
	showMainToolbar();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	glfwSwapBuffers(_p->mainWnd);
	glfwPollEvents();
}

void BasisControl::showMainToolbar()
{
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize;
	ImGui::SetNextWindowPos(ImVec2());
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));

	if (!ImGui::Begin("Control", 0, window_flags)) {
		ImGui::End();
		return;
	}

	ImVec4 color;

	// 'Exit' button
	color = _p->buttonOffColor;

	ImGui::PushStyleColor(ImGuiCol_Button, color);
	if (ImGui::Button("Exit")) {

	}
	ImGui::PopStyleColor();

	ImGui::End();
	ImGui::PopStyleColor();
}

void setup(Basis::System* s)
{
	std::cout << "BasisControl::setup()" << endl;

	sys = s;
	sys->registerEntity<BasisControl>();
}

