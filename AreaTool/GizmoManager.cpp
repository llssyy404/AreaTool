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
	return m_arrGizmo[static_cast<int>(eChangeType)]; 
}

void GizmoManager::SetSelection(DEFINE::CHANGE_TYPE eChangeType, bool bSelection)
{
	m_bSelectGiszmo = bSelection;
	if (!bSelection)
		m_spkSelectObject = nullptr;
}

void GizmoManager::SetSelectObject(const std::shared_ptr<Object>& spkSelectObject)
{
	m_spkSelectObject = spkSelectObject;
}

void GizmoManager::AnimateObjects(DEFINE::CHANGE_TYPE eChangeType, float fTimeElapsed)
{
	if (!m_bSelectGiszmo)
		return;

	if(m_spkSelectObject) m_arrGizmo[static_cast<int>(eChangeType)]->SetPosition(m_spkSelectObject->GetPosition());
	m_arrGizmo[static_cast<int>(eChangeType)]->AnimateObjects(fTimeElapsed);
}

void GizmoManager::Render(DEFINE::CHANGE_TYPE eChangeType)
{
	if (!m_bSelectGiszmo)
		return;

	m_arrGizmo[static_cast<int>(eChangeType)]->Render();
}