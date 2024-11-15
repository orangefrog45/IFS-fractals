#pragma once
#include "EngineAPI.h"

namespace ORNG {
	class GameLayer : public Layer {
	public:
		void OnInit() override;
		void Update() override {};
		void OnRender() override;
		void OnShutdown() override {};
		void OnImGuiRender() override {};

	private:
		ShaderVariants* mp_ifs_compute = nullptr;
		ShaderVariants* mp_ifs_transfer = nullptr;
		std::unique_ptr<Texture2D> mp_output_tex = nullptr;
		std::unique_ptr<Texture2D> mp_ifs_tex = nullptr;
	};
}