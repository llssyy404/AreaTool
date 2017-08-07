#include "stdafx.h"
#include "Define.h"
#include "DeviceManager.h"
#include "Camera.h"
#include "Scene.h"

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
	m_pkScene = NULL;
}

//--------------------------------------------------------------------------------------
// Create Direct3D device and swap chain
//--------------------------------------------------------------------------------------
HRESULT ToolFramework::InitDevice(HWND hWnd)
{
	HRESULT hr = S_OK;
	hr = DeviceManager::GetInstance().InitDevice(hWnd);
	if (FAILED(hr))
		return hr;

	// camera setting
	Camera::GetInstance().SetViewport(DeviceManager::GetInstance().GetDeviceContext(), 0, 0, DEFINE::SCREEN_WIDTH, DEFINE::SCREEN_HEIGHT, 0.0f, 1.0f);
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

	DeviceManager::Destroy();
}

void ToolFramework::InitObject()
{
	m_pkScene = new Scene();
	m_pkScene->CreateObjects();
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

void ToolFramework::OnMouseLUp(WPARAM wParam, int x, int y)
{
	if (m_pkScene)m_pkScene->OnMouseLUp(wParam, x, y);
}

void ToolFramework::OnMouseRUp(WPARAM wParam, int x, int y)
{
	if (m_pkScene)m_pkScene->OnMouseRUp(wParam, x, y);
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
		OnMouseLUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		break;
	case WM_RBUTTONUP:
		OnMouseRUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
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

	if (m_pkScene) m_pkScene->AnimateObjects(m_kTimer.GetTimeElapsed());

	float ClearColor[4] = { 0.5f, 0.7f, 1.f, 1.0f }; // red,green,blue,alpha
	DeviceManager::GetInstance().GetDeviceContext()->ClearRenderTargetView(DeviceManager::GetInstance().GetRenderTargetView(), ClearColor);
	DeviceManager::GetInstance().GetDeviceContext()->ClearDepthStencilView(DeviceManager::GetInstance().GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// object render
	if (m_pkScene) m_pkScene->Render(m_kTimer.GetTimeElapsed());
	Camera::GetInstance().UpdateViewMatrix();

	//
	DeviceManager::GetInstance().GetSwapChain()->Present(0, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////