#pragma once

#include "Define.h"

class Object;
class Gizmo;

class GizmoManager
{
public:
	GizmoManager();
	~GizmoManager();
	bool GetSelection() const { return m_bSelectGiszmo; }
	Gizmo* GetGizmo(DEFINE::CHANGE_TYPE eChangeType) const;

	void SetSelection(DEFINE::CHANGE_TYPE eChangeType, bool bSelection);
	void SetSelectObject(const std::shared_ptr<Object>& spkSelectObject);

	void ChangeType(DEFINE::CHANGE_TYPE eChangeType) { m_eChangeType = eChangeType; }
	
	void AnimateObjects(DEFINE::CHANGE_TYPE eChangeType, float fTimeElapsed);
	void Render(DEFINE::CHANGE_TYPE eChangeType);

private:
	Gizmo* m_arrGizmo[DEFINE::MAX_CHANGE_TYPE];
	bool m_bSelectGiszmo;
	DEFINE::CHANGE_TYPE m_eChangeType;
	std::shared_ptr<Object> m_spkSelectObject;
};
