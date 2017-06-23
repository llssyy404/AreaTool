#pragma once

#include "Timer.h"

class Scene;

class ToolFramework
{
public:
	ToolFramework();
	~ToolFramework();

	void	Init();
	HRESULT InitDevice(HWND hWnd);
	void	CleanupDevice();
	void	InitObject();

	void	OnMouseMove(WPARAM wParam, int x, int y);
	void	OnMouseLDown(WPARAM wParam, int x, int y);
	void	OnMouseRDown(WPARAM wParam, int x, int y);
	void	OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void	OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	void	Render();

private:
	Timer					m_kTimer;

	D3D_DRIVER_TYPE         m_driverType;
	D3D_FEATURE_LEVEL       m_featureLevel;
	ID3D11Device*           m_pd3dDevice;
	ID3D11DeviceContext*    m_pImmediateContext;
	IDXGISwapChain*         m_pSwapChain;
	ID3D11RenderTargetView* m_pRenderTargetView;
	
	Scene*					m_pkScene;
};

