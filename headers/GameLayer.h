#pragma once
#include "EngineAPI.h"

namespace ORNG {
	class GameLayer : public Layer {
	public:
		void OnInit() override;
		void Update() override;
		void OnRender() override;
		void OnShutdown() override {};
		void OnImGuiRender() override;

		void SaveVariant(const std::string& name);


		struct Resources {
			void Init(glm::uvec2 resolution);

			std::unique_ptr<Texture2D> p_output_tex = nullptr;
			std::unique_ptr<Texture2D> p_output_tex_8bpp = nullptr;

			std::unique_ptr<Texture2D> p_ifs_density_tex = nullptr;
			std::unique_ptr<Texture2D> p_ifs_col_tex = nullptr;
		};

		void RenderIFS(GameLayer::Resources& output);

	private:
		ShaderVariants* mp_ifs_compute = nullptr;
		ShaderVariants* mp_ifs_transfer = nullptr;

		Resources m_res;
	};

}