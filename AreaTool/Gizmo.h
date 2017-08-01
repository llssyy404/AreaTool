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

private:
	UINT m_uiIndexCountCone;
	XNA::AxisAlignedBox m_AxisAlignedBoxX;
	XNA::AxisAlignedBox m_AxisAlignedBoxY;
	XNA::AxisAlignedBox m_AxisAlignedBoxZ;
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

private:
	XNA::Sphere m_Sphere;
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

private:
	XNA::AxisAlignedBox m_AxisAlignedBoxX;
	XNA::AxisAlignedBox m_AxisAlignedBoxY;
	XNA::AxisAlignedBox m_AxisAlignedBoxZ;
};
