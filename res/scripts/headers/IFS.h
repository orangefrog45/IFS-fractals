#pragma once
#include "../includes/ScriptAPI.h"
#include "../headers/Ifs2D.h"
#include "../headers/Ifs3D.h"

extern "C" {
	using namespace ORNG;

	class IFS : public ScriptBase {
	public:
		IFS() = default;

		void OnCreate() override;

		void OnUpdate(float dt) override;

		void OnDestroy() override;

		void OnImGuiRender() override;

		void OnRender() override;

		void SaveVariant(const std::string& name);
	private:
		float m_brightness = 1.f;

		bool m_rendering_2d = true;

		Ifs3D m_ifs_3d;
		Ifs3D::Resources m_3d_ifs_resources;

		Ifs2D m_ifs_2d;
		Ifs2D::Resources m_2d_ifs_resources;

		Scene* mp_scene = nullptr;
		SceneEntity* p_cam = nullptr;
		RenderGraph m_render_graph;
		Framebuffer m_render_framebuffer;
	};
}
