#include "GameLayer.h"
#include "core/Input.h"
#include "util/ExtraUI.h"
#include "stb/stb_image_write.h"
#include "util/util.h"

using namespace ORNG;

void GameLayer::OnInit() {
	mp_ifs_compute = &Renderer::GetShaderLibrary().CreateShaderVariants("res/shaders/Ifs.comp");
	mp_ifs_compute->SetPath(GL_COMPUTE_SHADER, "res/shaders/Ifs.comp");
	mp_ifs_compute->AddVariant(0, {}, {"u_time_elapsed"});
	mp_ifs_compute->AddVariant(1, {"CLEAR"}, {});

	mp_ifs_transfer = &Renderer::GetShaderLibrary().CreateShaderVariants("res/shaders/IfsTransfer.comp");
	mp_ifs_transfer->SetPath(GL_COMPUTE_SHADER, "res/shaders/IfsTransfer.comp");
	mp_ifs_transfer->AddVariant(0, {}, {});

	m_res.Init({ Window::GetWidth(), Window::GetHeight() });
}

void GameLayer::Resources::Init(glm::uvec2 resolution) {
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


void GameLayer::RenderIFS(struct GameLayer::Resources& output) {
	auto& spec = output.p_ifs_col_tex->GetSpec();

	mp_ifs_compute->Activate(1);
	glBindImageTexture(0, output.p_ifs_density_tex->GetTextureHandle(), 0, false, 0, GL_READ_WRITE, GL_R32UI);
	glBindImageTexture(1, output.p_ifs_col_tex->GetTextureHandle(), 0, false, 0, GL_READ_WRITE, GL_R32UI);
	glDispatchCompute(spec.width / 8.f, spec.height / 8.f, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	mp_ifs_compute->Activate(0);
	mp_ifs_compute->SetUniform("u_time_elapsed", FrameTiming::GetTotalElapsedTime());
	glDispatchCompute(spec.width / 8.f, spec.height / 8.f, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	mp_ifs_transfer->Activate(0);
	glBindImageTexture(0, output.p_ifs_density_tex->GetTextureHandle(), 0, false, 0, GL_READ_WRITE, GL_R32UI);
	glBindImageTexture(1, output.p_ifs_col_tex->GetTextureHandle(), 0, false, 0, GL_READ_WRITE, GL_R32UI);
	glBindImageTexture(2, output.p_output_tex->GetTextureHandle(), 0, false, 0, GL_READ_WRITE, GL_RGBA16F);
	glBindImageTexture(3, output.p_output_tex_8bpp->GetTextureHandle(), 0, false, 0, GL_READ_WRITE, GL_RGBA8);
	glDispatchCompute(spec.width / 8.f, spec.height / 8.f, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
}

void GameLayer::Update() {
	if (Input::IsKeyPressed('f'))
		Renderer::GetShaderLibrary().ReloadShaders();
}

void GameLayer::OnRender() {
	if (!Input::IsKeyPressed('g'))
		return;

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	RenderIFS(m_res);

	Renderer::GetFramebufferLibrary().UnbindAllFramebuffers();
	glClearColor(1.f, 0.f, 0.f, 1.f);
	GL_StateManager::DefaultClearBits();
	GL_StateManager::BindTexture(GL_TEXTURE_2D, m_res.p_output_tex->GetTextureHandle(), GL_TEXTURE1, true);
	Renderer::GetShaderLibrary().GetQuadShader().ActivateProgram();
	glViewport(0, 0, Window::GetWidth(), Window::GetHeight());
	Renderer::DrawQuad();
}

void GameLayer::SaveVariant(const std::string& name) {
	stbi_flip_vertically_on_write(true);
	std::string output_dir = BASE_DIR "/configs/" + name;
	if (FileExists(output_dir)) {
		ORNG_CORE_ERROR("Failed to save variant '{}', variant with name already exists", name);
		return;
	}
	else {
		Create_Directory(output_dir);
	}

	GameLayer::Resources res;
	constexpr glm::uvec2 RESOLUTION = { 8192, 8192 };
	res.Init(RESOLUTION);
	RenderIFS(res);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	res.p_output_tex_8bpp->GenerateMips();

	auto& spec = res.p_output_tex_8bpp->GetSpec();

	std::vector<uint8_t> pixels;
	for (int i = 0; i < 4; i++) {
		glm::uvec2 mip_res = { spec.width / pow(2, i) , spec.height / pow(2, i)};

		const size_t NUM_PIXELS = mip_res.x * mip_res.y;
		pixels.resize(NUM_PIXELS * 4);
		glGetTextureImage(res.p_output_tex_8bpp->GetTextureHandle(), i, GL_RGBA, GL_UNSIGNED_BYTE, NUM_PIXELS * 4, pixels.data());

		std::string output_image_path = output_dir + "/" + name + std::to_string(i) + ".png";
		// Write out image preview of variant
		stbi_write_png(output_image_path.c_str(), mip_res.x, mip_res.y, 4, pixels.data(), 0);
	}


	constexpr auto SHADER_PATH = BASE_DIR "/res/shaders/Ifs.comp";
	// Write out shader file for variant
	FileCopy(SHADER_PATH, output_dir + "/Ifs.comp");
}

void GameLayer::OnImGuiRender() {
	ImGui::SetNextWindowPos({ 0, 0 });
	if (ImGui::Begin("Menu", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		static std::string save_name;
		ImGui::InputText("Save name", &save_name);

		if (ImGui::Button("Save")) {
			SaveVariant(save_name);
		}
	}

	ImGui::End();
}
