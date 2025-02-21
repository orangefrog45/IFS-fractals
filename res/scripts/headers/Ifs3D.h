#pragma once
#include "EngineAPI.h"
#include "rendering/Textures.h"
#include <glew-cmake/include/GL/glew.h>

using namespace ORNG;

class Ifs3D {
public:
	struct Resources {
		void Init(glm::uvec3 resolution) {
			Texture3DSpec uint_spec;
			uint_spec.width = resolution.x;
			uint_spec.height = resolution.y;
			uint_spec.layer_count = resolution.z;
			uint_spec.format = GL_RED_INTEGER;
			uint_spec.internal_format = GL_R32UI;
			uint_spec.storage_type = GL_UNSIGNED_INT;

			ifs_density_tex.SetSpec(uint_spec);
			ifs_col_tex.SetSpec(uint_spec);
		}

		ORNG::Texture3D ifs_density_tex{ "" };
		ORNG::Texture3D ifs_col_tex{ "" };
	};

	void Init() {
		m_ifs_compute.SetPath(GL_COMPUTE_SHADER, "res/shaders/Ifs3D.comp");
		m_ifs_compute.AddVariant(0, {}, { });
		m_ifs_compute.AddVariant(1, { "CLEAR" }, {});

		m_ifs_raymarch.SetPath(GL_COMPUTE_SHADER, "res/shaders/RaymarchVolume.comp");
		m_ifs_raymarch.AddVariant(0, {}, { });
	}

	void RenderIFS(Ifs3D::Resources& res) {
		const Texture3DSpec& spec = res.ifs_density_tex.GetSpec();
		glm::uvec3 resolution = glm::uvec3{ spec.width };

		m_ifs_compute.Activate(1);
		glBindImageTexture(0, res.ifs_density_tex.GetTextureHandle(), 0, true, 0, GL_READ_WRITE, GL_R32UI);
		glBindImageTexture(1, res.ifs_col_tex.GetTextureHandle(), 0, true, 0, GL_READ_WRITE, GL_R32UI);
		glDispatchCompute(glm::ceil(resolution.x / 4.f), glm::ceil(resolution.y / 4.f), glm::ceil(resolution.z / 4.f));
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		m_ifs_compute.Activate(0);
		glDispatchCompute(glm::ceil(resolution.x / 4.f), glm::ceil(resolution.y / 4.f), glm::ceil(resolution.z / 4.f));
		glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	}

	void RenderRaymarchedIfs(Ifs3D::Resources& res, Texture2D& output_tex) {
		auto& spec = output_tex.GetSpec();

		glBindImageTexture(1, output_tex.GetTextureHandle(), 0, false, 0, GL_WRITE_ONLY, spec.internal_format);
		GL_StateManager::BindTexture(GL_TEXTURE_3D, res.ifs_col_tex.GetTextureHandle(), GL_TEXTURE2);
		GL_StateManager::BindTexture(GL_TEXTURE_3D, res.ifs_density_tex.GetTextureHandle(), GL_TEXTURE3);

		m_ifs_raymarch.Activate(0);
		glDispatchCompute(glm::ceil(spec.width / 8.f), glm::ceil(spec.height / 8.f), 1.f);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
	}

private:
	ORNG::ShaderVariants m_ifs_compute;
	ORNG::ShaderVariants m_ifs_raymarch;
};