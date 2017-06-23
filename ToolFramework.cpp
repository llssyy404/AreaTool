#include "stdafx.h"
#include "Define.h"
#include "Scene.h"
#include "Camera.h"

#include "ToolFramework.h"

ToolFramework::ToolFramework()
{
	Init();
}

ToolFramework::~ToolFramework()
{
}

void ToolFramework::Init()
{
	m_driverType = D3D_DRIVER_TYPE_NULL;
	m_featureLevel = D3D_FEATURE_LEVEL_11_0;
	m_pd3dDevice = NULL;
	m_pImmediateContext = NULL;
	m_pSwapChain = NULL;
	m_pRenderTargetView = NULL;

	m_pkScene = NULL;
}

//--------------------------------------------------------------------------------------
// Create Direct3D device and swap chain
//--------------------------------------------------------------------------------------
HRESULT ToolFramework::InitDevice(HWND hWnd)
{
	HRESULT hr = S_OK;

	RECT rc;
	GetClientRect(hWnd, &rc);
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;

	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = width;
	sd.BufferDesc.Height = height;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;

	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		m_driverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDeviceAndSwapChain(NULL, m_driverType, NULL, createDeviceFlags, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, &sd, &m_pSwapChain, &m_pd3dDevice, &m_featureLevel, &m_pImmediateContext);
		if (SUCCEEDED(hr))
			break;
	}
	if (FAILED(hr))
		return hr;

	// Create a render target view
	ID3D11Texture2D* pBackBuffer = NULL;
	hr = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
	if (FAILED(hr))
		return hr;

	hr = m_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &m_pRenderTargetView);
	pBackBuffer->Release();
	if (FAILED(hr))
		return hr;

	m_pImmediateContext->OMSetRenderTargets(1, &m_pRenderTargetView, NULL);

	// camera setting
	Camera::GetInstance().SetViewport(m_pImmediateContext, 0, 0, DEFINE::SCREEN_WIDTH, DEFINE::SCREEN_HEIGHT, 0.0f, 1.0f);
	Camera::GetInstance().SetLens(XM_PIDIV2, static_cast<float>(DEFINE::SCREEN_WIDTH / DEFINE::SCREEN_HEIGHT), 0.01f, 100.0f);
	XMVECTOR Eye = XMVectorSet(0.0f, 1.0f, -5.0f, 0.0f);
	XMVECTOR At = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	Camera::GetInstance().LookAt(Eye, At, Up);

	InitObject();

	return S_OK;
}

//--------------------------------------------------------------------------------------
// Clean up the objects we've created
//--------------------------------------------------------------------------------------
void ToolFramework::CleanupDevice()
{
	Camera::GetInstance().Destroy();
	if (m_pkScene) m_pkScene->ReleaseObjects();
	if (m_pkScene) delete m_pkScene;

	if (m_pImmediateContext) m_pImmediateContext->ClearState();
	if (m_pRenderTargetView) m_pRenderTargetView->Release();
	if (m_pSwapChain) m_pSwapChain->Release();
	if (m_pImmediateContext) m_pImmediateContext->Release();
	if (m_pd3dDevice) m_pd3dDevice->Release();
}

void ToolFramework::InitObject()
{
	m_pkScene = new Scene();
	m_pkScene->CreateObjects(m_pd3dDevice);
}

void ToolFramework::OnMouseMove(WPARAM wParam, int x, int y)
{
	if (m_pkScene)m_pkScene->OnMouseMove(wParam, x, y);
}

void ToolFramework::OnMouseLDown(WPARAM wParam, int x, int y)
{
	if (m_pkScene)m_pkScene->OnMouseLDown(wParam, x, y);
}

void ToolFramework::OnMouseRDown(WPARAM wParam, int x, int y)
{
	if (m_pkScene)m_pkScene->OnMouseRDown(wParam, x, y);
}


void ToolFramework::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
}

void ToolFramework::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_KEYUP:
		if (m_pkScene)m_pkScene->OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam, m_kTimer.GetTimeElapsed());
		switch (wParam)
		{
		case VK_ESCAPE:
			::PostQuitMessage(0);
			break;
		default:
			break;
		}
		break;
	case WM_CHAR:
	case WM_KEYDOWN:
		if (m_pkScene)m_pkScene->OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam, m_kTimer.GetTimeElapsed());
		break;
	default:
		break;
	}
}

LRESULT CALLBACK ToolFramework::OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{

	switch (nMessageID)
	{
	case WM_LBUTTONDOWN:
		OnMouseLDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		break;
	case WM_RBUTTONDOWN:
		OnMouseRDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		break;
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		break;
	case WM_MOUSEMOVE:
		OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		break;
	case WM_KEYDOWN:
	case WM_CHAR:
	case WM_KEYUP:
		OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
		break;
	}
	return(0);
}

//--------------------------------------------------------------------------------------
// Render a frame
//--------------------------------------------------------------------------------------
void ToolFramework::Render()
{
	m_kTimer.Tick();

	float ClearColor[4] = { 0.5f, 0.7f, 1.f, 1.0f }; // red,green,blue,alpha
	m_pImmediateContext->ClearRenderTargetView(m_pRenderTargetView, ClearColor);

	m_pkScene->Render(m_pImmediateContext, m_kTimer.GetTimeElapsed());

	Camera::GetInstance().UpdateViewMatrix();

	m_pSwapChain->Present(0, 0);
}
