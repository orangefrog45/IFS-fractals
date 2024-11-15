#include "core/Application.h"
#include "GameLayer.h"
using namespace ORNG;

void main() {
	Application app;
	ApplicationData init_data{};
	init_data.disabled_modules = (ApplicationModulesFlags)(ApplicationModulesFlags::SCENE_RENDERER | ApplicationModulesFlags::AUDIO | ApplicationModulesFlags::PHYSICS);

	app.layer_stack.PushLayer(new GameLayer());
	app.Init(init_data);
}