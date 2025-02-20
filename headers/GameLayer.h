#pragma once
#include "EngineAPI.h"
#include "Ifs2D.h"
#include "Ifs3D.h"


namespace ORNG {
	class GameLayer : public Layer {
	public:
		void OnInit() override;
		void Update() override;
		void OnRender() override;
		void OnShutdown() override {};
		void OnImGuiRender() override;

		void SaveVariant(const std::string& name);
	private:
		float m_brightness = 1.f;

		bool m_rendering_2d = true;

		Ifs3D m_ifs_3d;
		Ifs3D::Resources m_3d_ifs_resources;

		Ifs2D m_ifs_2d;
		Ifs2D::Resources m_2d_ifs_resources;

		Scene m_scene;
		SceneEntity* p_cam = nullptr;
		RenderGraph m_render_graph;
		Framebuffer m_render_framebuffer;
	};
}