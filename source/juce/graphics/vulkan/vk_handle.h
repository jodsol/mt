#pragma once

namespace juce
{
template <typename T>
class vk_handle
{
public:
	vk_handle()                            = default;
	vk_handle(T rhs)                       = delete;
	vk_handle(const vk_handle&)            = delete;
	vk_handle& operator=(const vk_handle&) = delete;

	// 암묵적 변환은 좀 위험 할수 있으니 좀더 생각중
	operator T() const noexcept
	{
		return m_handle;
	}

	T handle() const
	{
		return m_handle;
	}

	T* handle_ptr()
	{
		return &m_handle;
	}

	const T* handle_ptr() const
	{
		return &m_handle;
	}

	bool valid()
	{
		return m_handle ? true : false;
	}

protected:
	T m_handle = 0;
};
}        // namespace juce
