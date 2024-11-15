#include "GameLayer.h"

using namespace ORNG;

void GameLayer::OnInit() {
	mp_ifs_compute = &Renderer::GetShaderLibrary().CreateShaderVariants("res/shaders/Ifs.comp");
	mp_ifs_compute->SetPath(GL_COMPUTE_SHADER, "res/shaders/Ifs.comp");
	mp_ifs_compute->AddVariant(0, {}, {"u_time_elapsed"});
	mp_ifs_compute->AddVariant(1, {"CLEAR"}, {});

	mp_ifs_transfer = &Renderer::GetShaderLibrary().CreateShaderVariants("res/shaders/IfsTransfer.comp");
	mp_ifs_transfer->SetPath(GL_COMPUTE_SHADER, "res/shaders/IfsTransfer.comp");
	mp_ifs_transfer->AddVariant(0, {}, {});

	Texture2DSpec spec;
	spec.width = Window::GetWidth();
	spec.height = Window::GetHeight();
	spec.format = GL_RGBA;
	spec.internal_format = GL_RGBA16F;
	spec.storage_type = GL_FLOAT;

	Texture2DSpec uint_spec;
	uint_spec.width = Window::GetWidth();
	uint_spec.height = Window::GetHeight();
	uint_spec.format = GL_RED_INTEGER;
	uint_spec.internal_format = GL_R32UI;
	uint_spec.storage_type = GL_UNSIGNED_INT;

	mp_output_tex = std::make_unique<Texture2D>("output_gamelayer");
	mp_output_tex->SetSpec(spec);
	mp_ifs_tex = std::make_unique<Texture2D>("ifs_tex");
	mp_ifs_tex->SetSpec(uint_spec);

}

void GameLayer::OnRender() {
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	mp_ifs_compute->Activate(1);
	glBindImageTexture(0, mp_ifs_tex->GetTextureHandle(), 0, false, 0, GL_READ_WRITE, GL_R32UI);
	glDispatchCompute(Window::GetWidth() / 8.f, Window::GetHeight() / 8.f, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	mp_ifs_compute->Activate(0);
	mp_ifs_compute->SetUniform("u_time_elapsed", FrameTiming::GetTotalElapsedTime());
	glDispatchCompute(Window::GetWidth() / 8.f, Window::GetHeight() / 8.f, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	mp_ifs_transfer->Activate(0);
	glBindImageTexture(0, mp_ifs_tex->GetTextureHandle(), 0, false, 0, GL_READ_WRITE, GL_R32UI);
	glBindImageTexture(1, mp_output_tex->GetTextureHandle(), 0, false, 0, GL_READ_WRITE, GL_RGBA16F);
	glDispatchCompute(Window::GetWidth() / 8.f, Window::GetHeight() / 8.f, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);


	Renderer::GetFramebufferLibrary().UnbindAllFramebuffers();
	glClearColor(1.f, 0.f, 0.f, 1.f);
	GL_StateManager::DefaultClearBits();
	GL_StateManager::BindTexture(GL_TEXTURE_2D, mp_output_tex->GetTextureHandle(), GL_TEXTURE1, true);
	Renderer::GetShaderLibrary().GetQuadShader().ActivateProgram();
	glViewport(0, 0, Window::GetWidth(), Window::GetHeight());
	Renderer::DrawQuad();
}