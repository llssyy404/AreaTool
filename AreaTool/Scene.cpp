#include "stdafx.h"
#include "Define.h"
#include "DeviceManager.h"
#include "Camera.h"
#include "GizmoManager.h"
#include "Gizmo.h"
#include "xnacollision.h"

#include "Scene.h"

using namespace std::placeholders;

Scene::Scene()
{
	m_spkSelectObject = nullptr;
	m_eChangeType = DEFINE::C_TRNAS;
}

Scene::~Scene()
{
	if(m_spkSelectObject) m_spkSelectObject = nullptr;
}

void Scene::CreateObjects()
{
	m_spkGrid = std::make_shared<Grid>();
	m_upkGizmoManager = std::make_unique<GizmoManager>();
}

void Scene::ReleaseObjects()
{
	m_spkSelectObject = nullptr;
}

void Scene::GetRayPosAndDir(int x, int y, XMVECTOR& rayPos, XMVECTOR& rayDir)
{
	XMMATRIX P = Camera::GetInstance().Proj();
	float vx = (+2.0f*x / DEFINE::SCREEN_WIDTH - 1.0f) / P(0, 0);
	float vy = (-2.0f*y / DEFINE::SCREEN_HEIGHT + 1.0f) / P(1, 1);

	rayPos = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	rayDir = XMVectorSet(vx, vy, 1.0f, 0.0f);

	XMMATRIX W = XMMatrixIdentity();
	XMMATRIX V = Camera::GetInstance().View();
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(V), V);
	XMMATRIX invWorld = XMMatrixInverse(&XMMatrixDeterminant(W), W);
	XMMATRIX toLocal = XMMatrixMultiply(invView, invWorld);

	rayPos = XMVector3TransformCoord(rayPos, toLocal);
	rayDir = XMVector3TransformNormal(rayDir, toLocal);
	rayDir = XMVector3Normalize(rayDir);
}

bool Scene::PickGizmo(XMVECTOR& rayPos, XMVECTOR& rayDir)
{
	return m_upkGizmoManager->PickGizmo(rayPos, rayDir);
}

void Scene::PickObject(XMVECTOR& rayPos, XMVECTOR& rayDir)
{
	float fDist = 9999.f;
	bool bPick = false;
	SP_Object iter = nullptr;
	float fMin = 9999.f;
	for (auto obj : m_listObject)
	{
		if (false == IntersectRayOrientedBox(rayPos, rayDir, &obj->GetOBB(), &fDist))
			continue;

		if (fDist >= fMin)
			continue;

		fMin = fDist;
		iter = obj;
		bPick = true;
	}

	if (bPick)
	{
		if (nullptr != m_spkSelectObject)
			m_spkSelectObject->SetSelection(false);

		if (m_upkGizmoManager)
		{
			m_spkSelectObject = iter;
			m_spkSelectObject->SetSelection(true);
			m_upkGizmoManager->SetSelectObject(m_spkSelectObject);
			m_upkGizmoManager->SetSelection(m_eChangeType, true);
		}
	}
	else
	{
		if (nullptr == m_spkSelectObject)
			return;

		if (m_upkGizmoManager) m_upkGizmoManager->SetSelection(m_eChangeType, false);
		m_spkSelectObject->SetSelection(false);
		m_spkSelectObject = nullptr;
	}
}

void Scene::DeleteSelectObject()
{
	if (nullptr == m_spkSelectObject)
		return;

	if (m_upkGizmoManager) m_upkGizmoManager->SetSelection(m_eChangeType, false);
	m_listObject.remove(m_spkSelectObject);
	m_spkSelectObject = nullptr;
}

void Scene::OnMouseMoveRightBtn(int x, int y)
{
	SetCursor(NULL);
	float dx = XMConvertToRadians(0.5f*static_cast<float>(x - m_poLastMousePos.x));
	float dy = XMConvertToRadians(0.5f*static_cast<float>(y - m_poLastMousePos.y));
	Camera::GetInstance().Pitch(dy);
	Camera::GetInstance().RotateY(dx);
}

void Scene::OnMouseMoveLeftBtn(int x, int y)
{
	if (nullptr == m_spkSelectObject)
		return;
	
	XMVECTOR rayPos; XMVECTOR rayDir;
	GetRayPosAndDir(x, y, rayPos, rayDir);

	float dx = XMConvertToRadians(0.5f*static_cast<float>(x - m_poLastMousePos.x));
	float dy = XMConvertToRadians(0.5f*static_cast<float>(y - m_poLastMousePos.y));
	m_upkGizmoManager->ChangeValue(dx, dy);
}

void Scene::OnMouseMove(WPARAM wParam, int x, int y)
{
	if (0 != (wParam & MK_RBUTTON))
	{
		OnMouseMoveRightBtn(x, y);
	}
	else if (0 != (wParam & MK_LBUTTON))
	{
		OnMouseMoveLeftBtn(x, y);
	}

	m_poLastMousePos.x = x;
	m_poLastMousePos.y = y;
}

void Scene::OnMouseLDown(WPARAM wParam, int x, int y)
{
	XMVECTOR rayPos; XMVECTOR rayDir;
	GetRayPosAndDir(x, y, rayPos, rayDir);
	
	if (true == PickGizmo(rayPos, rayDir))		// 기즈모 선택
		return;

	PickObject(rayPos, rayDir);					// 도형 선택
}

void Scene::OnMouseRDown(WPARAM wParam, int x, int y)
{
}

void Scene::OnMouseLUp(WPARAM wParam, int x, int y)
{
	m_upkGizmoManager->UnpickObject();
}

void Scene::OnMouseRUp(WPARAM wParam, int x, int y)
{
}

bool Scene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
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
	default:
		break;
	}

	return(false);
}

bool Scene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam, float fTimeElapsed)
{
	const float fCamVelocity = 1000.f;
	const float fPlusDir = 1.f;
	const float fMinusDir = -1.f;
	switch (nMessageID)
	{
	case WM_KEYDOWN:
		switch(wParam)
		{
		case VK_RIGHT:
			Camera::GetInstance().Strafe(fCamVelocity*fPlusDir*fTimeElapsed);
			break;
		case VK_LEFT:
			Camera::GetInstance().Strafe(fCamVelocity*fMinusDir*fTimeElapsed);
			break;
		case VK_UP:
			Camera::GetInstance().Walk(fCamVelocity*fPlusDir*fTimeElapsed);
			break;
		case VK_DOWN:
			Camera::GetInstance().Walk(fCamVelocity*fMinusDir*fTimeElapsed);
			break;
		case VK_DELETE:
			DeleteSelectObject();
			break;
		case '1':
			m_listObject.push_back(std::make_shared<Box>());
			break;
		case '2':
			m_listObject.push_back(std::make_shared<Cylinder>());
			break;
		case '3':
			m_listObject.push_back(std::make_shared<Sphere>());
			break;
		case 'W':
		{
			m_eChangeType = DEFINE::C_TRNAS;
			m_upkGizmoManager->ChangeType(m_eChangeType);
		}break;
		case 'E':
		{
			m_eChangeType = DEFINE::C_ROT;
			m_upkGizmoManager->ChangeType(m_eChangeType);
		}break;
		case 'R':
		{
			m_eChangeType = DEFINE::C_SCALE;
			m_upkGizmoManager->ChangeType(m_eChangeType);
		}break;
		default:
			break;
		}
		break;
	case WM_KEYUP:
		break;
	default:
		break;
	}

	return(false);
}

void Scene::AnimateObjects(float fTimeElapsed)
{
	if (m_upkGizmoManager) m_upkGizmoManager->AnimateObjects(m_eChangeType, fTimeElapsed);
	
	for (auto obj : m_listObject)
		obj->AnimateObjects(fTimeElapsed);
}

void Scene::Render(float fTimeElapsed)
{
	for (auto obj : m_listObject)
		obj->Render();

	if(m_upkGizmoManager) m_upkGizmoManager->Render(m_eChangeType);
	if(m_spkGrid) m_spkGrid->Render();
}