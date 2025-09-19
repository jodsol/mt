#pragma once

#include <juce/engine/scene.h>

namespace juce
{
class render_scene : public scene
{
public:
	explicit render_scene(vk_context* context) :
	    scene(context)
	{
		// 아직 context 가 init 전이라 여기선 작업 안함
		// 항상 init() 으로 초기화
	}

	void init() override;
	void update_frame(float dt) override;
	void render_frame() override;
	void release() override;
};
}        // namespace juce
