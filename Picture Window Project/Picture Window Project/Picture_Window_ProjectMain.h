#pragma once

#include "Common\StepTimer.h"
#include "Common\DeviceResources.h"

// Renders Direct2D and 3D content on the screen.
namespace Picture_Window_Project
{
	class Picture_Window_ProjectMain : public DX::IDeviceNotify
	{
	public:
		Picture_Window_ProjectMain(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		~Picture_Window_ProjectMain();
		void CreateWindowSizeDependentResources();
		void Update();
		bool Render();

		// IDeviceNotify
		virtual void OnDeviceLost();
		virtual void OnDeviceRestored();

	private:
		// Cached pointer to device resources.
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

		// TODO: Replace with your own content renderers.

		// Rendering loop timer.
		DX::StepTimer m_timer;
	};
}