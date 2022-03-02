#pragma once

#include "Define.h"

class Object;
class Gizmo;

class GizmoManager
{
public:
	GizmoManager();
	~GizmoManager();
	bool GetSelection() const { return m_bSelectGizmo; }
	Gizmo* GetGizmo(DEFINE::CHANGE_TYPE eChangeType) const;

	void SetSelection(DEFINE::CHANGE_TYPE eChangeType, bool bSelection);
	void SetSelectObject(std::shared_ptr<Object>& spkSelectObject);

	void ChangeType(DEFINE::CHANGE_TYPE eChangeType);
	
	bool PickGizmo(XMVECTOR& rayPos, XMVECTOR& rayDir);
	bool PickTransGizmo(XMVECTOR& rayPos, XMVECTOR& rayDir);
	bool PickRotGizmo(XMVECTOR& rayPos, XMVECTOR& rayDir);
	bool PickScaleGizmo(XMVECTOR& rayPos, XMVECTOR& rayDir);
	void UnpickObject();	//

	void ChangeValue(float x, float y); //

	void AnimateObjects(DEFINE::CHANGE_TYPE eChangeType, float fTimeElapsed);
	void Render(DEFINE::CHANGE_TYPE eChangeType);

private:
	Gizmo* m_arrGizmo[DEFINE::MAX_CHANGE_TYPE];
	bool m_bSelectGizmo;
	DEFINE::CHANGE_TYPE m_eChangeType;
	std::shared_ptr<Object> m_spkSelectObject;
	DEFINE::SELECT_EXIS m_eSelectExis;
	std::function<void(float)> m_functionOfSelObj;
	std::function<void(float)> m_functionOfSelGizmo;
};
