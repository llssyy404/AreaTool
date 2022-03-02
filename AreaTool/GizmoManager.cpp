#include "stdafx.h"
#include "Gizmo.h"

#include "GizmoManager.h"

using namespace std::placeholders;

GizmoManager::GizmoManager()
{
	m_bSelectGizmo = false;
	m_arrGizmo[DEFINE::CHANGE_TYPE::C_TRNAS] = new TransGizmo();
	m_arrGizmo[DEFINE::CHANGE_TYPE::C_ROT] = new RotationGizmo();
	m_arrGizmo[DEFINE::CHANGE_TYPE::C_SCALE] = new ScalingGizmo();
	m_spkSelectObject = nullptr;
}

GizmoManager::~GizmoManager()
{
	if(m_spkSelectObject) m_spkSelectObject = nullptr;
	for (int i = 0; i < DEFINE::CHANGE_TYPE::MAX_CHANGE_TYPE; ++i)
	{
		delete m_arrGizmo[i];
	}
}

Gizmo* GizmoManager::GetGizmo(DEFINE::CHANGE_TYPE eChangeType) const
{ 
	if (eChangeType >= DEFINE::CHANGE_TYPE::MAX_CHANGE_TYPE)
		return nullptr;

	return m_arrGizmo[eChangeType]; 
}

void GizmoManager::SetSelection(DEFINE::CHANGE_TYPE eChangeType, bool bSelection)
{
	if (m_eChangeType >= DEFINE::CHANGE_TYPE::MAX_CHANGE_TYPE)
		return;

	ChangeType(eChangeType);
	m_bSelectGizmo = bSelection;
	if (!bSelection)
		m_spkSelectObject = nullptr;
}

void GizmoManager::SetSelectObject(std::shared_ptr<Object>& spkSelectObject)
{
	m_spkSelectObject = spkSelectObject;
	if (m_spkSelectObject)
	{
		XMFLOAT3 vPosition = m_spkSelectObject->GetPosition();
		XMFLOAT3 f3Rotation = m_spkSelectObject->GetRotation();
		m_arrGizmo[m_eChangeType]->SetPosition(vPosition);
		m_arrGizmo[m_eChangeType]->SetRotation(f3Rotation);
	}
}

void GizmoManager::ChangeType(DEFINE::CHANGE_TYPE eChangeType)
{
	m_eChangeType = eChangeType;
	if (m_spkSelectObject)
	{
		XMFLOAT3 vPosition = m_spkSelectObject->GetPosition();
		XMFLOAT3 f3Rotation = m_spkSelectObject->GetRotation();
		m_arrGizmo[m_eChangeType]->SetPosition(vPosition);
		m_arrGizmo[m_eChangeType]->SetRotation(f3Rotation);
	}
}

bool GizmoManager::PickGizmo(XMVECTOR& rayPos, XMVECTOR& rayDir)
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

bool GizmoManager::PickTransGizmo(XMVECTOR& rayPos, XMVECTOR& rayDir)
{
	TransGizmo* transGizmo = dynamic_cast<TransGizmo*>(GetGizmo(DEFINE::CHANGE_TYPE::C_TRNAS));
	if (nullptr == transGizmo)
		return false;

	DEFINE::SELECT_EXIS selExis = transGizmo->IntersectRayAxis(rayPos, rayDir);
	if (selExis == DEFINE::SELECT_EXIS::MAX_SELECT_EXIS)
		return false;

	m_eSelectExis = selExis;
	switch (m_eSelectExis)
	{
	case DEFINE::SELECT_EXIS::SEL_X:
	{
		m_functionOfSelObj = std::bind(&Object::Right, m_spkSelectObject, _1);
		m_functionOfSelGizmo = std::bind(&Gizmo::Right, transGizmo, _1);
	}break;
	case DEFINE::SELECT_EXIS::SEL_Y:
	{
		m_functionOfSelObj = std::bind(&Object::Up, m_spkSelectObject, _1);
		m_functionOfSelGizmo = std::bind(&Gizmo::Up, transGizmo, _1);
	}break;
	case DEFINE::SELECT_EXIS::SEL_Z:
	{
		m_functionOfSelObj = std::bind(&Object::Forward, m_spkSelectObject, _1);
		m_functionOfSelGizmo = std::bind(&Gizmo::Forward, transGizmo, _1);
	}break;
	default:
		break;
	}
	
	return true;
}

bool GizmoManager::PickRotGizmo(XMVECTOR& rayPos, XMVECTOR& rayDir)
{
	RotationGizmo* rotateGizmo = dynamic_cast<RotationGizmo*>(GetGizmo(DEFINE::CHANGE_TYPE::C_ROT));
	if (nullptr == rotateGizmo)
		return false;

	DEFINE::SELECT_EXIS selExis = rotateGizmo->IntersectRayAxis(rayPos, rayDir);
	if (selExis == DEFINE::SELECT_EXIS::MAX_SELECT_EXIS)
		return false;

	m_eSelectExis = selExis;
	switch (m_eSelectExis)
	{
	case DEFINE::SELECT_EXIS::SEL_X:
	{
		m_functionOfSelObj = std::bind(&Object::Pitch, m_spkSelectObject, _1);
		m_functionOfSelGizmo = std::bind(&Gizmo::Pitch, rotateGizmo, _1);
	}break;
	case DEFINE::SELECT_EXIS::SEL_Y:
	{
		m_functionOfSelObj = std::bind(&Object::Yaw, m_spkSelectObject, _1);
		m_functionOfSelGizmo = std::bind(&Object::Yaw, rotateGizmo, _1);
	}break;
	case DEFINE::SELECT_EXIS::SEL_Z:
	{
		m_functionOfSelObj = std::bind(&Object::Roll, m_spkSelectObject, _1);
		m_functionOfSelGizmo = std::bind(&Object::Roll, rotateGizmo, _1);
	}break;
	default:
		break;
	}

	return true;
}

bool GizmoManager::PickScaleGizmo(XMVECTOR& rayPos, XMVECTOR& rayDir)
{
	ScalingGizmo* scaleGizmo = dynamic_cast<ScalingGizmo*>(GetGizmo(DEFINE::CHANGE_TYPE::C_SCALE));
	if (nullptr == scaleGizmo)
		return false;

	DEFINE::SELECT_EXIS selExis = scaleGizmo->IntersectRayAxis(rayPos, rayDir);
	if (selExis == DEFINE::SELECT_EXIS::MAX_SELECT_EXIS)
		return false;

	m_eSelectExis = selExis;
	switch (m_eSelectExis)
	{
	case DEFINE::SELECT_EXIS::SEL_X:
		m_functionOfSelObj = std::bind(&Object::ScalingX, m_spkSelectObject, _1);
		break;
	case DEFINE::SELECT_EXIS::SEL_Y:
		m_functionOfSelObj = std::bind(&Object::ScalingY, m_spkSelectObject, _1);
		break;
	case DEFINE::SELECT_EXIS::SEL_Z:
		m_functionOfSelObj = std::bind(&Object::ScalingZ, m_spkSelectObject, _1);
		break;
	default:
		break;
	}

	return true;
}

void GizmoManager::UnpickObject()
{
	if (m_functionOfSelObj)
		m_functionOfSelObj = nullptr;

	if (m_functionOfSelGizmo)
		m_functionOfSelGizmo = nullptr;
}

void GizmoManager::ChangeValue(float x, float y)
{
	if (nullptr == m_spkSelectObject)
		return;

	switch (m_eChangeType)
	{
	case DEFINE::C_TRNAS:
	{
		if(m_functionOfSelObj) m_functionOfSelObj(m_eSelectExis == DEFINE::SELECT_EXIS::SEL_Y ? -y : x);
		if(m_functionOfSelGizmo) m_functionOfSelGizmo(m_eSelectExis == DEFINE::SELECT_EXIS::SEL_Y ? -y : x);
	}break;
	case DEFINE::C_ROT:
	{
		if (m_functionOfSelObj) m_functionOfSelObj(-y * 50);
		if (m_functionOfSelGizmo) m_functionOfSelGizmo(-y * 50);
	}break;
	case DEFINE::C_SCALE:
	{
		if(m_functionOfSelObj) m_functionOfSelObj(m_eSelectExis == DEFINE::SELECT_EXIS::SEL_Y ? -y : x);
	}break;
	default:
		break;
	}
}

void GizmoManager::AnimateObjects(DEFINE::CHANGE_TYPE eChangeType, float fTimeElapsed)
{
	if (m_eChangeType >= DEFINE::CHANGE_TYPE::MAX_CHANGE_TYPE)
		return;

	if (!m_bSelectGizmo)
		return;

	m_arrGizmo[m_eChangeType]->AnimateObjects(fTimeElapsed);
}

void GizmoManager::Render(DEFINE::CHANGE_TYPE eChangeType)
{
	if (m_eChangeType >= DEFINE::CHANGE_TYPE::MAX_CHANGE_TYPE)
		return;

	if (!m_bSelectGizmo)
		return;

	m_arrGizmo[m_eChangeType]->Render();
}