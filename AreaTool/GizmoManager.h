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
	void AnimateObjects(DEFINE::CHANGE_TYPE eChangeType, float fTimeElapsed);
	void Render(DEFINE::CHANGE_TYPE eChangeType);

private:
	Gizmo* m_arrGizmo[DEFINE::MAX_CHANGE_TYPE];
	bool m_bSelectGiszmo;
	std::shared_ptr<Object> m_spkSelectObject;
};
