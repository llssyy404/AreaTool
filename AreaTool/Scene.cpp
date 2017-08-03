#include "stdafx.h"
#include "Define.h"
#include "DeviceManager.h"
#include "Camera.h"
#include "GizmoManager.h"
#include "Gizmo.h"
#include "Object.h"
#include "xnacollision.h"

#include "Scene.h"


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
	m_spkGrid = std::shared_ptr<Object>(new Grid());
	m_spkGizmoManager = std::shared_ptr<GizmoManager>(new GizmoManager);
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

void Scene::TransformSelectObject(const XMVECTOR &rayPos, const XMVECTOR &rayDir, float &fDist, float dx, float dy)
{
	if (DEFINE::C_TRNAS != m_eChangeType)
	{
		std::cout << "DEFINE::C_TRNAS != m_eChangeType" << std::endl;
		return;
	}

	TransGizmo* transGizmo = dynamic_cast<TransGizmo*>(m_spkGizmoManager->GetGizmo(m_eChangeType));
	if (TRUE == IntersectRayAxisAlignedBox(rayPos, rayDir, &transGizmo->GetAABBX(), &fDist))
	{
		std::cout << "X축선택" << std::endl;
		if (nullptr != m_spkSelectObject) {
			m_spkSelectObject->Right(dx); transGizmo->SetPosition(m_spkSelectObject->GetPosition());
		}
	}
	else if (TRUE == IntersectRayAxisAlignedBox(rayPos, rayDir, &transGizmo->GetAABBY(), &fDist))
	{
		std::cout << "Y축선택" << std::endl;
		if (nullptr != m_spkSelectObject) {
			m_spkSelectObject->Up(-dy); transGizmo->SetPosition(m_spkSelectObject->GetPosition());
		}
	}
	else if (TRUE == IntersectRayAxisAlignedBox(rayPos, rayDir, &transGizmo->GetAABBZ(), &fDist))
	{
		std::cout << "Z축선택" << std::endl;
		if (nullptr != m_spkSelectObject) {
			m_spkSelectObject->Forward(dx); transGizmo->SetPosition(m_spkSelectObject->GetPosition());
		}
	}

}

void Scene::RotateSelectObject(const XMVECTOR &rayPos, const XMVECTOR &rayDir, float &fDist, float dx, float dy)
{
	if (DEFINE::C_ROT != m_eChangeType)
	{
		std::cout << "DEFINE::C_ROT != m_eChangeType" << std::endl;
		return;
	}

	RotationGizmo* rotateGizmo = dynamic_cast<RotationGizmo*>(m_spkGizmoManager->GetGizmo(m_eChangeType));
	if (TRUE == IntersectRaySphere(rayPos, rayDir, &rotateGizmo->GetSphere(), &fDist))
	{
		std::cout << "구충돌체 선택" << std::endl;
		if (nullptr != m_spkSelectObject) {
			m_spkSelectObject->Roll(-dy * 50);
		}
	}
}

void Scene::ScaleSelectObject(const XMVECTOR &rayPos, const XMVECTOR &rayDir, float &fDist, float dx, float dy)
{
	if (DEFINE::C_SCALE != m_eChangeType)
	{
		std::cout << "DEFINE::C_ROT != m_eChangeType" << std::endl;
		return;
	}

	ScalingGizmo* scaleGizmo = dynamic_cast<ScalingGizmo*>(m_spkGizmoManager->GetGizmo(m_eChangeType));
	if (TRUE == IntersectRayAxisAlignedBox(rayPos, rayDir, &scaleGizmo->GetAABBX(), &fDist))
	{
		std::cout << "X축선택" << std::endl;
		if (nullptr != m_spkSelectObject) {
			m_spkSelectObject->ScalingX(dx);
		}
	}
	else if (TRUE == IntersectRayAxisAlignedBox(rayPos, rayDir, &scaleGizmo->GetAABBY(), &fDist))
	{
		std::cout << "Y축선택" << std::endl;
		if (nullptr != m_spkSelectObject) {
			m_spkSelectObject->ScalingY(-dy);
		}
	}
	else if (TRUE == IntersectRayAxisAlignedBox(rayPos, rayDir, &scaleGizmo->GetAABBZ(), &fDist))
	{
		std::cout << "Z축선택" << std::endl;
		if (nullptr != m_spkSelectObject) {
			m_spkSelectObject->ScalingZ(dx);
		}
	}
}

void Scene::OnMouseMoveRightBtn(int x, int y)
{
	SetCursor(NULL);
	// 각도계산부분
	float dx = XMConvertToRadians(0.5f*static_cast<float>(x - m_poLastMousePos.x));
	float dy = XMConvertToRadians(0.5f*static_cast<float>(y - m_poLastMousePos.y));
	Camera::GetInstance().Pitch(dy);
	Camera::GetInstance().RotateY(dx);
}

void Scene::OnMouseMoveLeftBtn(int x, int y)
{
	XMVECTOR rayPos; XMVECTOR rayDir;
	GetRayPosAndDir(x, y, rayPos, rayDir);

	float fDist = 9999.f;
	float dx = XMConvertToRadians(0.5f*static_cast<float>(x - m_poLastMousePos.x));
	float dy = XMConvertToRadians(0.5f*static_cast<float>(y - m_poLastMousePos.y));
	if (m_spkGizmoManager->GetSelection())
	{
		switch (m_eChangeType)
		{
		case DEFINE::C_TRNAS:
			TransformSelectObject(rayPos, rayDir, fDist, dx, dy);
			break;
		case DEFINE::C_ROT:
			RotateSelectObject(rayPos, rayDir, fDist, dx, dy);
			break;
		case DEFINE::C_SCALE:
			ScaleSelectObject(rayPos, rayDir, fDist, dx, dy);
			break;
		case DEFINE::MAX_CHANGE_TYPE:
			std::cout << "Invalid Change Type: " << m_eChangeType << std::endl;
			break;
		default:
			break;
		}
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

	bool bPick = false;
	float fDist = 9999.f;
	float fMin = 9999.f;
	SP_Object iter = nullptr;

	for (auto obj : m_listObject)
	{
		if (false == IntersectRayAxisAlignedBox(rayPos, rayDir, &obj->GetAABB(), &fDist))	// 도형 선택
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
		if(m_spkGizmoManager) m_spkGizmoManager->SetSelection(m_eChangeType, true);
		if (m_spkGizmoManager) m_spkGizmoManager->SetSelectObject(m_spkSelectObject);
	}
	else
	{
		if (nullptr == m_spkSelectObject)
			return;

		m_spkSelectObject->SetSelection(false);
		if(m_spkGizmoManager) m_spkGizmoManager->SetSelection(m_eChangeType, false);
		m_spkSelectObject = nullptr;
	}
}

void Scene::OnMouseRDown(WPARAM wParam, int x, int y)
{
}

bool Scene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
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
		{
			if (nullptr == m_spkSelectObject)
				return false;

			m_listObject.remove(m_spkSelectObject);
			m_spkSelectObject = nullptr;
		}break;
		case '1':
			m_listObject.push_back(std::shared_ptr<Box>(new Box()));
			break;
		case '2':
			m_listObject.push_back(std::shared_ptr<Cylinder>(new Cylinder()));
			break;
		case '3':
			m_listObject.push_back(std::shared_ptr<Sphere>(new Sphere()));
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

bool Scene::ProcessInput(float timeElapsed, HWND hwnd)
{
	return(false);
}

void Scene::AnimateObjects(float fTimeElapsed)
{
	for (auto obj : m_listObject)
		obj->AnimateObjects(fTimeElapsed);

	if(m_spkGizmoManager) m_spkGizmoManager->AnimateObjects(m_eChangeType, fTimeElapsed);
}

void Scene::Render(float fTimeElapsed)
{
	for (auto obj : m_listObject)
		obj->Render();

	if(m_spkGizmoManager) m_spkGizmoManager->Render(m_eChangeType);
	if(m_spkGrid) m_spkGrid->Render();
}