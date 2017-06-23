#include "stdafx.h"
#include "Define.h"
#include "Camera.h"
#include "Object.h"
#include "xnacollision.h"

#include "Scene.h"


Scene::Scene()
{
	m_itSelectObject = m_listObject.end();
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
	std::list<std::shared_ptr<Object>>::iterator iter = m_listObject.begin();
	for (auto obj : m_listObject)
	{
		if (false == IntersectRayAxisAlignedBox(rayPos, rayDir, &obj->GetAABB(), &fDist))
			continue;

		if(fDist >= fMin)
			continue;

		fMin = fDist;
		*iter = obj;
		bPick = true;
	}

	if (bPick)
	{
		if (m_listObject.end() != m_itSelectObject) 
			(*m_itSelectObject)->SetSelection(false);

		m_itSelectObject = iter;
		(*m_itSelectObject)->SetSelection(true);
	}
	else
	{
		if (m_listObject.end() == m_itSelectObject)
			return;

		(*m_itSelectObject)->SetSelection(false);
		m_itSelectObject = m_listObject.end();
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
	switch (nMessageID)
	{
	case WM_KEYDOWN:
		switch(wParam)
		{
		case VK_RIGHT:
			// 이동 숫자같은거 조정가능하도록
			Camera::GetInstance().Strafe(1.f);
			break;
		case VK_LEFT:
			Camera::GetInstance().Strafe(-1.f);
			break;
		case VK_UP:
			Camera::GetInstance().Walk(1.f);
			break;
		case VK_DOWN:
			Camera::GetInstance().Walk(-1.f);
			break;
		case VK_DELETE:
		{
			if (m_listObject.end() != m_itSelectObject)
			{
				m_listObject.remove(*m_itSelectObject);
				m_itSelectObject = m_listObject.end();
			}
		}break;
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
				if (m_listObject.end() != m_itSelectObject) (*m_itSelectObject)->Forward(1.f);
				break;
			case C_ROT:
				if (m_listObject.end() != m_itSelectObject) (*m_itSelectObject)->Roll(10.f);
				break;
			case C_SCALE:
				if (m_listObject.end() != m_itSelectObject) (*m_itSelectObject)->ScalingZ(1.f);
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
				if (m_listObject.end() != m_itSelectObject) (*m_itSelectObject)->Forward(-1.f);
				break;
			case C_ROT:
				if (m_listObject.end() != m_itSelectObject) (*m_itSelectObject)->Roll(-10.f);
				break;
			case C_SCALE:
				if (m_listObject.end() != m_itSelectObject) (*m_itSelectObject)->ScalingZ(-1.f);
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
				if (m_listObject.end() != m_itSelectObject) (*m_itSelectObject)->Right(1.f);
				break;
			case C_ROT:
				if (m_listObject.end() != m_itSelectObject) (*m_itSelectObject)->Pitch(10.f);
				break;
			case C_SCALE:
				if (m_listObject.end() != m_itSelectObject) (*m_itSelectObject)->ScalingX(1.f);
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
				if (m_listObject.end() != m_itSelectObject) (*m_itSelectObject)->Right(-1.f);
				break;
			case C_ROT:
				if (m_listObject.end() != m_itSelectObject) (*m_itSelectObject)->Pitch(-10.f);
				break;
			case C_SCALE:
				if (m_listObject.end() != m_itSelectObject) (*m_itSelectObject)->ScalingX(-1.f);
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
				if (m_listObject.end() != m_itSelectObject) (*m_itSelectObject)->Up(1.f);
				break;
			case C_ROT:
				if (m_listObject.end() != m_itSelectObject) (*m_itSelectObject)->Yaw(10.f);
				break;
			case C_SCALE:
				if (m_listObject.end() != m_itSelectObject) (*m_itSelectObject)->ScalingY(1.f);
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
				if (m_listObject.end() != m_itSelectObject) (*m_itSelectObject)->Up(-1.f);
				break;
			case C_ROT:
				if (m_listObject.end() != m_itSelectObject) (*m_itSelectObject)->Yaw(-10.f);
				break;
			case C_SCALE:
				if (m_listObject.end() != m_itSelectObject) (*m_itSelectObject)->ScalingY(-1.f);
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

void Scene::CreateObjects(ID3D11Device *&pd3dDevice)
{
	//for (int i = 0; i < 5; ++i)
		m_listObject.push_back(std::shared_ptr<Object>(new Object(pd3dDevice, DEFINE::F_BOX)));
		m_listObject.push_back(std::shared_ptr<Object>(new Object(pd3dDevice, DEFINE::F_GRID)));

}

void Scene::ReleaseObjects()
{
}

bool Scene::ProcessInput(float timeElapsed, HWND hwnd)
{
	return(false);
}

void Scene::Render(ID3D11DeviceContext*&pd3dDeviceContext, float fTimeElapsed)
{
	for (auto &obj : m_listObject)
		obj->Render(pd3dDeviceContext);
}