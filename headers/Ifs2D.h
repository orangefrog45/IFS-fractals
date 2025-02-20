#pragma once
#include "EngineAPI.h"
#include "rendering/Textures.h"

class Ifs2D {
public:
	struct Resources {
		void Init(glm::uvec2 resolution);

		std::unique_ptr<ORNG::Texture2D> p_output_tex = nullptr;
		std::unique_ptr<ORNG::Texture2D> p_output_tex_8bpp = nullptr;

		std::unique_ptr<ORNG::Texture2D> p_ifs_density_tex = nullptr;
		std::unique_ptr<ORNG::Texture2D> p_ifs_col_tex = nullptr;

		std::unique_ptr<ORNG::Texture2D> p_final_render_tex = nullptr;

	};

	void Init();

	void RenderIFS(Resources& output, float brightness);
private:
	ORNG::ShaderVariants m_ifs_compute;
	ORNG::ShaderVariants m_ifs_transfer;

	Resources m_res;
};