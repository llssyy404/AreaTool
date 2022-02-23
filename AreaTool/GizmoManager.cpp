#include "stdafx.h"
#include "Gizmo.h"

#include "GizmoManager.h"


GizmoManager::GizmoManager()
{
	m_bSelectGiszmo = false;
	m_arrGizmo[DEFINE::C_TRNAS] = new TransGizmo();
	m_arrGizmo[DEFINE::C_ROT] = new RotationGizmo();
	m_arrGizmo[DEFINE::C_SCALE] = new ScalingGizmo();
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
	m_bSelectGiszmo = bSelection;
	if (!bSelection)
		m_spkSelectObject = nullptr;
}

void GizmoManager::SetSelectObject(const std::shared_ptr<Object>& spkSelectObject)
{
	m_spkSelectObject = spkSelectObject;
	if (m_spkSelectObject)
	{
		XMFLOAT3 vPosition = m_spkSelectObject->GetPosition();
		XMFLOAT3 f3Rotation = m_spkSelectObject->GetPosition();
		m_arrGizmo[m_eChangeType]->SetPosition(vPosition);
		m_arrGizmo[m_eChangeType]->SetRotation(f3Rotation);
	}
}

void GizmoManager::AnimateObjects(DEFINE::CHANGE_TYPE eChangeType, float fTimeElapsed)
{
	if (m_eChangeType >= DEFINE::CHANGE_TYPE::MAX_CHANGE_TYPE)
		return;

	if (!m_bSelectGiszmo)
		return;

	if (m_spkSelectObject)
	{
		XMFLOAT3 vPosition = m_spkSelectObject->GetPosition();
		XMFLOAT3 f3Rotation = m_spkSelectObject->GetPosition();
		m_arrGizmo[m_eChangeType]->SetPosition(vPosition);
		m_arrGizmo[m_eChangeType]->SetRotation(f3Rotation);
	}
	m_arrGizmo[m_eChangeType]->AnimateObjects(fTimeElapsed);
}

void GizmoManager::Render(DEFINE::CHANGE_TYPE eChangeType)
{
	if (m_eChangeType >= DEFINE::CHANGE_TYPE::MAX_CHANGE_TYPE)
		return;

	if (!m_bSelectGiszmo)
		return;

	m_arrGizmo[m_eChangeType]->Render();
}