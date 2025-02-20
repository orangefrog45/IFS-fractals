#include "GameLayer.h"
#include "core/Input.h"
#include "util/ExtraUI.h"
#include "stb/stb_image_write.h"
#include "util/util.h"
#include "components/ComponentSystems.h"
#include "components/systems/SceneUBOSystem.h"

using namespace ORNG;

constexpr unsigned IFS_3D_RESOLUTION = 400;

void GameLayer::OnInit() {
	m_render_framebuffer.Init();

	m_2d_ifs_resources.Init({Window::GetWidth(), Window::GetHeight()});
	m_ifs_2d.Init();

	m_3d_ifs_resources.Init({ IFS_3D_RESOLUTION, IFS_3D_RESOLUTION, IFS_3D_RESOLUTION });
	m_ifs_3d.Init();

	m_scene.AddSystem(new CameraSystem(&m_scene));
	m_scene.AddSystem(new SceneUBOSystem(&m_scene));
	m_scene.LoadScene();
	p_cam = &m_scene.CreateEntity("cam");
	p_cam->AddComponent<CameraComponent>()->MakeActive();

	m_render_graph.AddRenderpass<BloomPass>();
	m_render_graph.SetData("PPS", &m_scene.post_processing);
	m_render_graph.SetData("Scene", &m_scene);
	m_render_graph.SetData("OutCol", &*m_2d_ifs_resources.p_final_render_tex);
	m_render_graph.SetData("BloomInCol", &*m_2d_ifs_resources.p_output_tex);
	m_render_graph.Init();
}


void GameLayer::Update() {
	float dt = FrameTiming::GetTimeStep() * 0.001f;
	auto* p_cam_comp = p_cam->GetComponent<CameraComponent>();
	auto* p_transform = p_cam->GetComponent<TransformComponent>();
	p_cam_comp->aspect_ratio = (float)Window::GetWidth() / Window::GetHeight();

	if (!m_rendering_2d) {
		m_scene.Update(FrameTiming::GetTimeStep());

		static glm::ivec2 mouse_lock_pos;
		if (Window::GetInput().IsMouseClicked(1)) {
			mouse_lock_pos = Window::GetInput().GetMousePos();
		}

		if (Window::GetInput().IsMouseDown(1)) {
			glm::vec3 move{ 0, 0, 0 };
			move += p_transform->forward * (float)Window::GetInput().IsKeyDown('w');
			move -= p_transform->forward * (float)Window::GetInput().IsKeyDown('s');
			move += p_transform->right * (float)Window::GetInput().IsKeyDown('d');
			move -= p_transform->right * (float)Window::GetInput().IsKeyDown('a');
			move += glm::vec3{ 0, 1, 0 } *(float)Window::GetInput().IsKeyDown(32); // space
			move -= glm::vec3{ 0, 1, 0 } *(float)Window::GetInput().IsKeyDown(340); // shift

			glm::vec2 delta_mouse = Window::GetInput().GetMousePos() - mouse_lock_pos;
			glm::quat y_rot = glm::angleAxis(-delta_mouse.x * dt, glm::vec3{ 0, 1, 0 });
			glm::quat x_rot = glm::angleAxis(-delta_mouse.y * dt, p_transform->right);
			glm::vec3 fwd = y_rot * x_rot * p_transform->forward;

			if (Window::GetInput().IsKeyDown(Key::LeftControl))
				move *= 10.f;

			p_transform->SetPosition(p_transform->GetPosition() + move * dt * 10.f);
			p_transform->LookAt(p_transform->GetPosition() + fwd);

			Window::SetCursorPos(mouse_lock_pos.x, mouse_lock_pos.y);
		}
	}

	if (Window::GetInput().IsKeyPressed('f'))
		Renderer::GetShaderLibrary().ReloadShaders();
}



void GameLayer::OnRender() {
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	if (Window::GetInput().IsKeyPressed('g')) {
		if (m_rendering_2d) {
			m_ifs_2d.RenderIFS(m_2d_ifs_resources, m_brightness);
			m_render_graph.Execute();
		}
		else {
			m_ifs_3d.RenderIFS(m_3d_ifs_resources);
		}
	}

	if (!m_rendering_2d) m_ifs_3d.RenderRaymarchedIfs(m_3d_ifs_resources, *m_2d_ifs_resources.p_final_render_tex);

	//m_render_framebuffer.Bind();
	//m_render_framebuffer.BindTexture2D(m_res.p_final_render_tex->GetTextureHandle(), GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D);
	//Renderer::GetShaderLibrary().GetQuadShader().ActivateProgram();
	//GL_StateManager::BindTexture(GL_TEXTURE_2D, m_res.p_output_tex->GetTextureHandle(), GL_TEXTURE1, true);
	//Renderer::DrawQuad();
	//glMemoryBarrier(GL_ALL_BARRIER_BITS);
	//m_render_graph.Execute();

	glClearColor(0.f, 0.f, 0.f, 1.f);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	GL_StateManager::DefaultClearBits();
	GL_StateManager::BindTexture(GL_TEXTURE_2D, m_2d_ifs_resources.p_final_render_tex->GetTextureHandle(), GL_TEXTURE1, true);
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


	Ifs2D::Resources res;
	constexpr glm::uvec2 RESOLUTION = { 4096, 4096 };
	res.Init(RESOLUTION);

	RenderGraph temp_rg;
	temp_rg.AddRenderpass<BloomPass>();
	temp_rg.SetData("PPS", &m_scene.post_processing);
	temp_rg.SetData("Scene", &m_scene);
	temp_rg.SetData("OutCol", &*res.p_final_render_tex);
	temp_rg.SetData("BloomInCol", &*res.p_output_tex);
	temp_rg.Init();

	m_ifs_2d.RenderIFS(res, m_brightness);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	m_render_framebuffer.Bind();
	m_render_framebuffer.BindTexture2D(res.p_final_render_tex->GetTextureHandle(), GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D);
	Renderer::GetShaderLibrary().GetQuadShader().ActivateProgram();
	GL_StateManager::BindTexture(GL_TEXTURE_2D, res.p_output_tex->GetTextureHandle(), GL_TEXTURE1, true);
	glViewport(0, 0, RESOLUTION.x, RESOLUTION.y);
	Renderer::DrawQuad();
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	temp_rg.Execute();
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	res.p_final_render_tex->GenerateMips();

	auto& spec = res.p_final_render_tex->GetSpec();

	std::vector<uint8_t> pixels;
	for (int i = 0; i < 4; i++) {
		glm::uvec2 mip_res = { spec.width / pow(2, i) , spec.height / pow(2, i)};

		const size_t NUM_PIXELS = mip_res.x * mip_res.y;
		pixels.resize(NUM_PIXELS * 4);
		glGetTextureImage(res.p_final_render_tex->GetTextureHandle(), i, GL_RGBA, GL_UNSIGNED_BYTE, NUM_PIXELS * 4, pixels.data());

		std::string output_image_path = output_dir + "/" + name + std::to_string(i) + ".png";
		// Write out image preview of variant
		stbi_write_png(output_image_path.c_str(), mip_res.x, mip_res.y, 4, pixels.data(), 0);
	}


	constexpr auto SHADER_PATH = BASE_DIR "/res/shaders/Ifs.comp";
	// Write out shader file for variant
	FileCopy(SHADER_PATH, output_dir + "/Ifs.comp");

	//mp_bloom_pass->ResizeTexture(ceil(Window::GetWidth() * 0.5f), ceil(Window::GetHeight() * 0.5f));
	m_render_framebuffer.BindTexture2D(m_2d_ifs_resources.p_final_render_tex->GetTextureHandle(), GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D);

}

void GameLayer::OnImGuiRender() {
	ImGui::SetNextWindowPos({ 0, 0 });
	if (ImGui::Begin("Menu", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		static std::string save_name;
		ImGui::InputText("Save name", &save_name);

		if (ImGui::Button("Save")) {
			SaveVariant(save_name);
		}

		ImGui::DragFloat("Bloom threshold", &m_scene.post_processing.bloom.threshold);
		ImGui::DragFloat("Bloom intensity", &m_scene.post_processing.bloom.intensity);
		ImGui::DragFloat("Brightness", &m_brightness, 0.1f, 0.f);
		ImGui::Checkbox("Rendering 2D", &m_rendering_2d);
	}

	ImGui::End();
}
