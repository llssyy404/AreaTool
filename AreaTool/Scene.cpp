#include "stdafx.h"
#include "Define.h"
#include "DeviceManager.h"
#include "Camera.h"
#include "GizmoManager.h"
#include "Gizmo.h"
#include "Object.h"
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

bool Scene::PickTransGizmo(XMVECTOR& rayPos, XMVECTOR& rayDir)
{
	const TransGizmo* transGizmo = dynamic_cast<const TransGizmo*>(m_upkGizmoManager->GetGizmo(DEFINE::C_TRNAS));
	if (nullptr == transGizmo)
		return false;

	float fDist = 0.f;
	if (TRUE == IntersectRayAxisAlignedBox(rayPos, rayDir, &transGizmo->GetAABBX(), &fDist))
	{
		m_eSelectExis = SEL_X;
		m_functionOfSelObj = std::bind(&Object::Right, m_spkSelectObject, _1);
	}
	else if (TRUE == IntersectRayAxisAlignedBox(rayPos, rayDir, &transGizmo->GetAABBY(), &fDist))
	{
		m_eSelectExis = SEL_Y;
		m_functionOfSelObj = std::bind(&Object::Up, m_spkSelectObject, _1);
	}
	else if (TRUE == IntersectRayAxisAlignedBox(rayPos, rayDir, &transGizmo->GetAABBZ(), &fDist))
	{
		m_eSelectExis = SEL_Z;
		m_functionOfSelObj = std::bind(&Object::Forward, m_spkSelectObject, _1);
	}
	else
		return false;

	return true;
}

bool Scene::PickRotGizmo(XMVECTOR& rayPos, XMVECTOR& rayDir)
{
	const RotationGizmo* rotateGizmo = dynamic_cast<const RotationGizmo*>(m_upkGizmoManager->GetGizmo(DEFINE::C_ROT));
	if (nullptr == rotateGizmo)
		return false;

	float fDist = 0.f;
	if (TRUE == IntersectRaySphere(rayPos, rayDir, &rotateGizmo->GetSphere(), &fDist))
	{
		m_eSelectExis = SEL_X;
		m_functionOfSelObj = std::bind(&Object::Roll, m_spkSelectObject, _1);
	}
	else
		return false;

	return true;
}

bool Scene::PickScaleGizmo(XMVECTOR& rayPos, XMVECTOR& rayDir)
{
	const ScalingGizmo* scaleGizmo = dynamic_cast<const ScalingGizmo*>(m_upkGizmoManager->GetGizmo(DEFINE::C_SCALE));
	if (nullptr == scaleGizmo)
		return false;

	float fDist = 0.f;
	if (TRUE == IntersectRayAxisAlignedBox(rayPos, rayDir, &scaleGizmo->GetAABBX(), &fDist))
	{
		m_eSelectExis = SEL_X;
		m_functionOfSelObj = std::bind(&Object::ScalingX, m_spkSelectObject, _1);
	}
	else if (TRUE == IntersectRayAxisAlignedBox(rayPos, rayDir, &scaleGizmo->GetAABBY(), &fDist))
	{
		m_eSelectExis = SEL_Y;
		m_functionOfSelObj = std::bind(&Object::ScalingY, m_spkSelectObject, _1);
	}
	else if (TRUE == IntersectRayAxisAlignedBox(rayPos, rayDir, &scaleGizmo->GetAABBZ(), &fDist))
	{
		m_eSelectExis = SEL_Z;
		m_functionOfSelObj = std::bind(&Object::ScalingZ, m_spkSelectObject, _1);
	}
	else
		return false;

	return true;
}

bool Scene::PickGizmo(XMVECTOR& rayPos, XMVECTOR& rayDir)
{
	if (nullptr == m_spkSelectObject)
		return false;

	switch (m_eChangeType)
	{
	case DEFINE::C_TRNAS:
		return PickTransGizmo(rayPos, rayDir);
	case DEFINE::C_ROT:
		return PickRotGizmo(rayPos, rayDir);
	case DEFINE::C_SCALE:
		return PickScaleGizmo(rayPos, rayDir);
	default:
		break;
	}

	return false;
}

void Scene::PickObject(XMVECTOR& rayPos, XMVECTOR& rayDir)
{
	float fDist = 9999.f;
	bool bPick = false;
	SP_Object iter = nullptr;
	float fMin = 9999.f;
	for (auto obj : m_listObject)
	{
		if (false == IntersectRayAxisAlignedBox(rayPos, rayDir, &obj->GetAABB(), &fDist))
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

		m_spkSelectObject = iter;
		m_spkSelectObject->SetSelection(true);
		if (m_upkGizmoManager) m_upkGizmoManager->SetSelection(m_eChangeType, true);
		if (m_upkGizmoManager) m_upkGizmoManager->SetSelectObject(m_spkSelectObject);
	}
	else
	{
		if (nullptr == m_spkSelectObject)
			return;

		m_spkSelectObject->SetSelection(false);
		if (m_upkGizmoManager) m_upkGizmoManager->SetSelection(m_eChangeType, false);
		m_spkSelectObject = nullptr;
	}
}

void Scene::DeleteSelectObject()
{
	if (nullptr == m_spkSelectObject)
		return;

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
	
	if (nullptr == m_functionOfSelObj)
		return;

	XMVECTOR rayPos; XMVECTOR rayDir;
	GetRayPosAndDir(x, y, rayPos, rayDir);

	float dx = XMConvertToRadians(0.5f*static_cast<float>(x - m_poLastMousePos.x));
	float dy = XMConvertToRadians(0.5f*static_cast<float>(y - m_poLastMousePos.y));
	
	switch (m_eChangeType)
	{
	case DEFINE::C_TRNAS:
		m_functionOfSelObj(m_eSelectExis == SEL_Y ? -dy : dx);
		break;
	case DEFINE::C_ROT:
	{
		m_functionOfSelObj(-dy * 50);
		m_upkGizmoManager->GetGizmo(m_eChangeType)->Roll(-dy * 50);
	}break;
	case DEFINE::C_SCALE:
		m_functionOfSelObj(m_eSelectExis == SEL_Y ? -dy : dx);
		break;
	default:
		break;
	}
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
	if (m_functionOfSelObj)
	{
		m_functionOfSelObj = nullptr;
	}
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
			m_eChangeType = DEFINE::C_TRNAS;
			break;
		case 'E':
			m_eChangeType = DEFINE::C_ROT;
			break;
		case 'R':
			m_eChangeType = DEFINE::C_SCALE;
			break;
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
	for (auto obj : m_listObject)
		obj->AnimateObjects(fTimeElapsed);

	if(m_upkGizmoManager) m_upkGizmoManager->AnimateObjects(m_eChangeType, fTimeElapsed);
}

void Scene::Render(float fTimeElapsed)
{
	for (auto obj : m_listObject)
		obj->Render();

	if(m_upkGizmoManager) m_upkGizmoManager->Render(m_eChangeType);
	if(m_spkGrid) m_spkGrid->Render();
}