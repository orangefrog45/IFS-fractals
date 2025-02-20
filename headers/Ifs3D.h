#pragma once
#include "EngineAPI.h"
#include "rendering/Textures.h"

class Ifs3D {
public:
	void Init();

	struct Resources {
		void Init(glm::uvec3 resolution);

		ORNG::Texture3D ifs_density_tex{ "" };
		ORNG::Texture3D ifs_col_tex{""};
	};

	void RenderIFS(Resources& output);

	void RenderRaymarchedIfs(Resources& res, ORNG::Texture2D& output_tex);

private:
	ORNG::ShaderVariants m_ifs_compute;
	ORNG::ShaderVariants m_ifs_raymarch;
};