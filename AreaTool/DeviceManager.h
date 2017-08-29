#pragma once

#include "Singleton.h"

class DeviceManager : public Singleton<DeviceManager>
{
public:
	DeviceManager();
	~DeviceManager();
	HRESULT InitDevice(HWND hWnd);
	void	CleanupDevice();

	ID3D11Device* GetDevice() const { return m_pd3dDevice; }
	ID3D11DeviceContext* GetDeviceContext() const { return m_pd3dDeviceContext; }
	IDXGISwapChain* GetSwapChain() const { return m_pSwapChain; }

	ID3D11RenderTargetView* GetRenderTargetView() const { return m_pRenderTargetView; }
	ID3D11DepthStencilView* GetDepthStencilView() const { return m_pd3dDepthStencilView; }

private:
	D3D_DRIVER_TYPE         m_driverType;
	D3D_FEATURE_LEVEL       m_featureLevel;
	ID3D11Device*           m_pd3dDevice;
	ID3D11DeviceContext*    m_pd3dDeviceContext;
	IDXGISwapChain*         m_pSwapChain;

	//
	ID3D11RenderTargetView* m_pRenderTargetView;
	ID3D11Texture2D*		m_pd3dDepthStencilBuffer;
	ID3D11DepthStencilView*	m_pd3dDepthStencilView;
};

