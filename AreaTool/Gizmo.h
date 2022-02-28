#pragma once

#include "Object.h"

class Gizmo : public Object
{
public:
	Gizmo() : Object() {}
	virtual ~Gizmo() {}
	virtual void	CreateFigure(GeometryGenerator::MeshData& kMeshData) override = 0;
	virtual void	BuildGeometryBuffers() override = 0;
	virtual void	WorldMatrixSRT() override = 0;
	virtual void	Render() override = 0;
};

class TransGizmo : public Gizmo
{
public:
	TransGizmo();
	virtual ~TransGizmo();
	virtual void	CreateFigure(GeometryGenerator::MeshData& kMeshData) override;
	virtual void	BuildGeometryBuffers() override;
	virtual void	WorldMatrixSRT() override;
	virtual void	Render() override;

	XNA::AxisAlignedBox GetAABBX() const { return m_AxisAlignedBoxX; }
	XNA::AxisAlignedBox GetAABBY() const { return m_AxisAlignedBoxY; }
	XNA::AxisAlignedBox GetAABBZ() const { return m_AxisAlignedBoxZ; }
	XNA::OrientedBox GetOBBX() const { return m_OrientedBoxX; }
	XNA::OrientedBox GetOBBY() const { return m_OrientedBoxY; }
	XNA::OrientedBox GetOBBZ() const { return m_OrientedBoxZ; }

private:
	UINT m_uiIndexCountCone;
	XNA::AxisAlignedBox m_AxisAlignedBoxX;
	XNA::AxisAlignedBox m_AxisAlignedBoxY;
	XNA::AxisAlignedBox m_AxisAlignedBoxZ;
	XNA::OrientedBox m_OrientedBoxX;
	XNA::OrientedBox m_OrientedBoxY;
	XNA::OrientedBox m_OrientedBoxZ;
};

class RotationGizmo : public Gizmo
{
public:
	RotationGizmo();
	virtual ~RotationGizmo();
	virtual void	CreateFigure(GeometryGenerator::MeshData& kMeshData) override;
	virtual void	BuildGeometryBuffers() override;
	virtual void	WorldMatrixSRT() override;
	virtual void	Render() override;

	XNA::Sphere GetSphere() const { return m_Sphere; }
	XNA::OrientedBox GetOBBX() const { return m_OrientedBoxX; }
	XNA::OrientedBox GetOBBY() const { return m_OrientedBoxY; }
	XNA::OrientedBox GetOBBZ() const { return m_OrientedBoxZ; }

private:
	XNA::Sphere m_Sphere;
	XNA::OrientedBox m_OrientedBoxX;
	XNA::OrientedBox m_OrientedBoxY;
	XNA::OrientedBox m_OrientedBoxZ;
};

class ScalingGizmo : public Gizmo
{
public:
	ScalingGizmo();
	virtual ~ScalingGizmo();
	virtual void	CreateFigure(GeometryGenerator::MeshData& kMeshData) override;
	virtual void	BuildGeometryBuffers() override;
	virtual void	WorldMatrixSRT() override;
	virtual void	Render() override;

	XNA::AxisAlignedBox GetAABBX() const { return m_AxisAlignedBoxX; }
	XNA::AxisAlignedBox GetAABBY() const { return m_AxisAlignedBoxY; }
	XNA::AxisAlignedBox GetAABBZ() const { return m_AxisAlignedBoxZ; }
	XNA::OrientedBox GetOBBX() const { return m_OrientedBoxX; }
	XNA::OrientedBox GetOBBY() const { return m_OrientedBoxY; }
	XNA::OrientedBox GetOBBZ() const { return m_OrientedBoxZ; }

private:
	XNA::AxisAlignedBox m_AxisAlignedBoxX;
	XNA::AxisAlignedBox m_AxisAlignedBoxY;
	XNA::AxisAlignedBox m_AxisAlignedBoxZ;
	XNA::OrientedBox m_OrientedBoxX;
	XNA::OrientedBox m_OrientedBoxY;
	XNA::OrientedBox m_OrientedBoxZ;
};
