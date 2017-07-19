#include "stdafx.h"
#include "Define.h"
#include "DeviceManager.h"
#include "Camera.h"
#include "Object.h"
#include "xnacollision.h"

#include "Scene.h"


Scene::Scene()
{
	m_spkSelectObject = NULL;
	m_kChangeType = C_TRNAS;
}

Scene::~Scene()
{
}

void Scene::OnMouseMove(WPARAM wParam, int x, int y)
{
	if (0 != (wParam & MK_RBUTTON))
	{
		SetCursor(NULL);
		// 각도계산부분
		float dx = XMConvertToRadians(0.5f*static_cast<float>(x - m_poLastMousePos.x));
		float dy = XMConvertToRadians(0.5f*static_cast<float>(y - m_poLastMousePos.y));
		Camera::GetInstance().Pitch(dy);
		Camera::GetInstance().RotateY(dx);
	}

	m_poLastMousePos.x = x;
	m_poLastMousePos.y = y;
}

void Scene::OnMouseLDown(WPARAM wParam, int x, int y)
{
	XMMATRIX P = Camera::GetInstance().Proj();
	float vx = (+2.0f*x / DEFINE::SCREEN_WIDTH - 1.0f) / P(0, 0);
	float vy = (-2.0f*y / DEFINE::SCREEN_HEIGHT + 1.0f) / P(1, 1);

	XMVECTOR rayPos = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	XMVECTOR rayDir = XMVectorSet(vx, vy, 1.0f, 0.0f);

	XMMATRIX W = XMMatrixIdentity();
	XMMATRIX V = Camera::GetInstance().View();
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(V), V);
	XMMATRIX invWorld = XMMatrixInverse(&XMMatrixDeterminant(W), W);
	XMMATRIX toLocal = XMMatrixMultiply(invView, invWorld);

	rayPos = XMVector3TransformCoord(rayPos, toLocal);
	rayDir = XMVector3TransformNormal(rayDir, toLocal);
	rayDir = XMVector3Normalize(rayDir);

	bool bPick = false;
	float fDist = 9999.f;
	float fMin = 9999.f;
	SP_Object iter = NULL;
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
		if (NULL != m_spkSelectObject)
			m_spkSelectObject->SetSelection(false);

		m_spkSelectObject = iter;
		m_spkSelectObject->SetSelection(true);
	}
	else
	{
		if (NULL == m_spkSelectObject)
			return;

		m_spkSelectObject->SetSelection(false);
		m_spkSelectObject = NULL;
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
			if (NULL != m_spkSelectObject)
			{
				m_listObject.remove(m_spkSelectObject);
				m_spkSelectObject = NULL;
			}
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
			m_kChangeType = C_TRNAS;
			break;
		case 'E':
			m_kChangeType = C_ROT;
			break;
		case 'R':
			m_kChangeType = C_SCALE;
			break;
		case 'T':
		{
			switch (m_kChangeType)
			{
			case C_TRNAS:
				if (NULL != m_spkSelectObject) m_spkSelectObject->Forward(1.f);
				break;
			case C_ROT:
				if (NULL != m_spkSelectObject) m_spkSelectObject->Roll(10.f);
				break;
			case C_SCALE:
				if (NULL != m_spkSelectObject) m_spkSelectObject->ScalingZ(1.f);
				break;
			default:
				break;
			}
		}break;
		case 'G':
		{
			switch (m_kChangeType)
			{
			case C_TRNAS:
				if (NULL != m_spkSelectObject) m_spkSelectObject->Forward(-1.f);
				break;
			case C_ROT:
				if (NULL != m_spkSelectObject) m_spkSelectObject->Roll(-10.f);
				break;
			case C_SCALE:
				if (NULL != m_spkSelectObject) m_spkSelectObject->ScalingZ(-1.f);
				break;
			default:
				break;
			}
		}break;
		case 'H':
		{
			switch (m_kChangeType)
			{
			case C_TRNAS:
				if (NULL != m_spkSelectObject) m_spkSelectObject->Right(1.f);
				break;
			case C_ROT:
				if (NULL != m_spkSelectObject) m_spkSelectObject->Pitch(10.f);
				break;
			case C_SCALE:
				if (NULL != m_spkSelectObject) m_spkSelectObject->ScalingX(1.f);
				break;
			default:
				break;
			}
		}break;
		case 'F':
		{
			switch (m_kChangeType)
			{
			case C_TRNAS:
				if (NULL != m_spkSelectObject) m_spkSelectObject->Right(-1.f);
				break;
			case C_ROT:
				if (NULL != m_spkSelectObject) m_spkSelectObject->Pitch(-10.f);
				break;
			case C_SCALE:
				if (NULL != m_spkSelectObject) m_spkSelectObject->ScalingX(-1.f);
				break;
			default:
				break;
			}
		}break;
		case 'Y':
		{
			switch (m_kChangeType)
			{
			case C_TRNAS:
				if (NULL != m_spkSelectObject) m_spkSelectObject->Up(1.f);
				break;
			case C_ROT:
				if (NULL != m_spkSelectObject) m_spkSelectObject->Yaw(10.f);
				break;
			case C_SCALE:
				if (NULL != m_spkSelectObject) m_spkSelectObject->ScalingY(1.f);
				break;
			default:
				break;
			}
		}break;
		case 'V':
		{
			switch (m_kChangeType)
			{
			case C_TRNAS:
				if (NULL != m_spkSelectObject) m_spkSelectObject->Up(-1.f);
				break;
			case C_ROT:
				if (NULL != m_spkSelectObject) m_spkSelectObject->Yaw(-10.f);
				break;
			case C_SCALE:
				if (NULL != m_spkSelectObject) m_spkSelectObject->ScalingY(-1.f);
				break;
			default:
				break;
			}
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

void Scene::CreateObjects()
{
	m_spkGrid = std::shared_ptr<Object>(new Grid());
}

void Scene::ReleaseObjects()
{
	m_spkSelectObject = NULL;
}

bool Scene::ProcessInput(float timeElapsed, HWND hwnd)
{
	return(false);
}

void Scene::Render(float fTimeElapsed)
{
	m_spkGrid->Render();

	for (auto obj : m_listObject)
		obj->Render();
}