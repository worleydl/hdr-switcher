// main.cpp : Defines the entry point for the console application.
//

/*
This is a utility application to put a display into HDR mode

The display will remain in HDR mode until enter is pressed in the keyboard.
This allows users to create shaders for other apps without necessaryily modifying the code.
*/

#include <iostream>

#include "DeviceManager.h"
#include "uhdDisplay.h"

/*
	
*/
class UIController : public IVisualController
{
private:
	bool first_loop = true;
	float ui_update_time;

public:
	UIController()
	{
		ui_update_time = 0.0;
	}

	~UIController()
	{
		// No-op
	}

	virtual LRESULT MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		return 1;
	}

	virtual void Animate(double fElapsedTimeSeconds)
	{
		if (first_loop) {
			first_loop = false;
			SetHdrMonitorMode(true);
		}
		
		ui_update_time += fElapsedTimeSeconds;

		// Kill the window after giving the display suitable time enable HDR
		if (ui_update_time > 5.0) {
			PostQuitMessage(0);
		}
	}

	virtual void Render(ID3D11Device*, ID3D11DeviceContext*, ID3D11RenderTargetView*, ID3D11DepthStencilView*)
	{
		// No-op
	}

	virtual HRESULT DeviceCreated(ID3D11Device* pDevice)
	{
		return S_OK;
	}

	virtual void DeviceDestroyed()
	{
		// No-op
	}

	virtual void BackBufferResized(ID3D11Device* pDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
	{
		// No-op
	}
};


int main()
{
	std::cout << "This program will attempt to put your display in HDR mode\n" << std::endl;
	std::cout << "The display will remain in HDR mode until this application is closed." << std::endl;

	DeviceManager* mgr = new DeviceManager();

	auto ui_controller = UIController();
	mgr->AddControllerToFront(&ui_controller);

	DeviceCreationParameters deviceParams;
	deviceParams.swapChainFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
	deviceParams.swapChainSampleCount = 1;
	deviceParams.startFullscreen = true;
	deviceParams.backBufferWidth = 8196;
	deviceParams.backBufferHeight = 8196;
	deviceParams.displayIndex = 0; // TODO: Allow override via args

	if (FAILED(mgr->CreateWindowDeviceAndSwapChain(deviceParams, L"HDRDisplay image viewer")))
	{
		MessageBox(nullptr, L"Cannot initialize the D3D11 device with the requested parameters", L"Error", MB_OK | MB_ICONERROR);
		return 1;
	}
	mgr->SetVsyncEnabled(true);
	
	mgr->MessageLoop();
	mgr->Shutdown();

	delete mgr;
	
	// Wait for user to press enter to close the app
	std::cout << std::endl;
	std::cout << "Press enter to close the application and disable HDR rendering.";
	getchar();

	SetHdrMonitorMode(false);

    return 0;
}

