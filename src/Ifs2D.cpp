#include "pch/pch.h"
#include "Ifs2D.h"

using namespace ORNG;

void Ifs2D::Init() {
	m_ifs_compute.SetPath(GL_COMPUTE_SHADER, "res/shaders/Ifs.comp");
	m_ifs_compute.AddVariant(0, {}, { "u_time_elapsed" });
	m_ifs_compute.AddVariant(1, { "CLEAR" }, {});

	m_ifs_transfer.SetPath(GL_COMPUTE_SHADER, "res/shaders/IfsTransfer.comp");
	m_ifs_transfer.AddVariant(0, {}, { "u_brightness" });

	m_res.Init({ Window::GetWidth(), Window::GetHeight() });
}

void Ifs2D::RenderIFS(Ifs2D::Resources& output, float brightness) {
	auto& spec = output.p_ifs_col_tex->GetSpec();
	glm::vec4 clear_val{ 0.f };
	//glClearTexSubImage(output.p_final_render_tex->GetTextureHandle(), 0, 0, 0, 0, spec.width, spec.height, 1, GL_RGBA, GL_FLOAT, &clear_val);

	m_ifs_compute.Activate(1);
	glBindImageTexture(0, output.p_ifs_density_tex->GetTextureHandle(), 0, false, 0, GL_READ_WRITE, GL_R32UI);
	glBindImageTexture(1, output.p_ifs_col_tex->GetTextureHandle(), 0, false, 0, GL_READ_WRITE, GL_R32UI);
	glDispatchCompute(spec.width / 8.f, spec.height / 8.f, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	m_ifs_compute.Activate(0);
	m_ifs_compute.SetUniform("u_time_elapsed", FrameTiming::GetTotalElapsedTime());
	glDispatchCompute(spec.width / 8.f, spec.height / 8.f, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	m_ifs_transfer.Activate(0);
	m_ifs_transfer.SetUniform("u_brightness", brightness);
	glBindImageTexture(0, output.p_ifs_density_tex->GetTextureHandle(), 0, false, 0, GL_READ_WRITE, GL_R32UI);
	glBindImageTexture(1, output.p_ifs_col_tex->GetTextureHandle(), 0, false, 0, GL_READ_WRITE, GL_R32UI);
	glBindImageTexture(2, output.p_output_tex->GetTextureHandle(), 0, false, 0, GL_READ_WRITE, GL_RGBA16F);
	glBindImageTexture(3, output.p_output_tex_8bpp->GetTextureHandle(), 0, false, 0, GL_READ_WRITE, GL_RGBA8);
	glDispatchCompute(spec.width / 8.f, spec.height / 8.f, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void Ifs2D::Resources::Init(glm::uvec2 resolution) {
	Texture2DSpec spec;
	spec.width = resolution.x;
	spec.height = resolution.y;
	spec.format = GL_RGBA;
	spec.internal_format = GL_RGBA16F;
	spec.storage_type = GL_FLOAT;

	Texture2DSpec uint_spec;
	uint_spec.width = resolution.x;
	uint_spec.height = resolution.y;
	uint_spec.format = GL_RED_INTEGER;
	uint_spec.internal_format = GL_R32UI;
	uint_spec.storage_type = GL_UNSIGNED_INT;

	p_output_tex = std::make_unique<Texture2D>("output_gamelayer");
	p_output_tex->SetSpec(spec);

	p_final_render_tex = std::make_unique<Texture2D>("ifs_render");
	p_final_render_tex->SetSpec(spec);

	spec.internal_format = GL_RGBA8;
	spec.storage_type = GL_UNSIGNED_BYTE;
	spec.generate_mipmaps = true;
	p_output_tex_8bpp = std::make_unique<Texture2D>("output_8bpp");
	p_output_tex_8bpp->SetSpec(spec);

	p_ifs_density_tex = std::make_unique<Texture2D>("ifs_density_tex");
	p_ifs_density_tex->SetSpec(uint_spec);
	p_ifs_col_tex = std::make_unique<Texture2D>("ifs_col_tex");
	p_ifs_col_tex->SetSpec(uint_spec);
}
