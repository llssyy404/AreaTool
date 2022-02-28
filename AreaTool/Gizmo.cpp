#include "stdafx.h"
#include "DeviceManager.h"
#include "Camera.h"

#include "Gizmo.h"

TransGizmo::TransGizmo() : Gizmo()
{
	Init();

	// aabb
	XMFLOAT3 posX = m_vPosition; posX.x += 0.5f;
	m_AxisAlignedBoxX.Center = posX;
	m_AxisAlignedBoxX.Extents = XMFLOAT3(0.5f, 0.05f, 0.05f);
	m_AxisAlignedBoxX.Scale = XMFLOAT3(1.f, 1.f, 1.f);

	XMFLOAT3 posY = m_vPosition; posY.y += 0.5f;
	m_AxisAlignedBoxY.Center = posY;
	m_AxisAlignedBoxY.Extents = XMFLOAT3(0.05f, 0.5f, 0.05f);
	m_AxisAlignedBoxY.Scale = XMFLOAT3(1.f, 1.f, 1.f);

	XMFLOAT3 posZ = m_vPosition; posZ.z += 0.5f;
	m_AxisAlignedBoxZ.Center = posZ;
	m_AxisAlignedBoxZ.Extents = XMFLOAT3(0.05f, 0.05f, 0.5f);
	m_AxisAlignedBoxZ.Scale = XMFLOAT3(1.f, 1.f, 1.f);

	// obb
	posX = m_vPosition; posX.x += 0.5f;
	m_OrientedBoxX.Center = posX;
	m_OrientedBoxX.Extents = XMFLOAT3(0.5f, 0.05f, 0.05f);
	m_OrientedBoxX.Orientation = XMFLOAT4(0.f, 0.f, 0.f, 1.f);

	posY = m_vPosition; posY.y += 0.5f;
	m_OrientedBoxY.Center = posY;
	m_OrientedBoxY.Extents = XMFLOAT3(0.05f, 0.5f, 0.05f);
	m_OrientedBoxY.Orientation = XMFLOAT4(0.f, 0.f, 0.f, 1.f);

	posZ = m_vPosition; posZ.z += 0.5f;
	m_OrientedBoxZ.Center = posZ;
	m_OrientedBoxZ.Extents = XMFLOAT3(0.05f, 0.05f, 0.5f);
	m_OrientedBoxZ.Orientation = XMFLOAT4(0.f, 0.f, 0.f, 1.f);
}

TransGizmo::~TransGizmo()
{

}

void TransGizmo::CreateFigure(GeometryGenerator::MeshData& kMeshData)
{

}

void TransGizmo::BuildGeometryBuffers()
{
	GeometryGenerator::MeshData coneX;
	GeometryGenerator::MeshData coneY;
	GeometryGenerator::MeshData coneZ;

	GeometryGenerator geoGen;

	geoGen.CreateCylinder(0.06f, 0.0f, 0.2f, 6, 1, coneX);
	geoGen.CreateCylinder(0.06f, 0.0f, 0.2f, 6, 1, coneY);
	geoGen.CreateCylinder(0.06f, 0.0f, 0.2f, 6, 1, coneZ);

	for (int i = 0; i < coneX.Vertices.size(); ++i)
	{
		XMMATRIX rot = XMMatrixRotationRollPitchYaw(0, 0, XMConvertToRadians(-90.f));
		XMVECTOR v = XMLoadFloat3(&coneX.Vertices[i].Position);
		v = XMVector3Rotate(v, XMQuaternionRotationMatrix(rot));

		XMStoreFloat3(&coneX.Vertices[i].Position, v);
		coneX.Vertices[i].Position.x += 1.0f;
	}

	for (int i = 0; i < coneY.Vertices.size(); ++i)
	{
		coneY.Vertices[i].Position.y += 1.0f;
	}

	for (int i = 0; i < coneZ.Vertices.size(); ++i)
	{
		XMMATRIX rot = XMMatrixRotationRollPitchYaw(XMConvertToRadians(90.f), 0, 0);
		XMVECTOR v = XMLoadFloat3(&coneZ.Vertices[i].Position);
		v = XMVector3Rotate(v, XMQuaternionRotationMatrix(rot));

		XMStoreFloat3(&coneZ.Vertices[i].Position, v);
		coneZ.Vertices[i].Position.z += 1.0f;
	}

	const int vertexCount = 6;
	std::vector<SimpleVertex> vertices(vertexCount + coneX.Vertices.size() + coneY.Vertices.size() + coneZ.Vertices.size());

	m_iVertexOffset = 0;
	m_uiIndexCount = static_cast<UINT>(vertexCount);
	m_uiIndexCountCone = static_cast<UINT>(coneX.Indices.size());
	m_uiIndexOffset = 0;

	int count = 0;
	vertices[count].m_f3Pos = XMFLOAT3(0.0f, 0.0f, 0.0f); vertices[count++].m_f4Color = DEFINE::COLOR_RED;
	vertices[count].m_f3Pos = XMFLOAT3(1.0f, 0.0f, 0.0f); vertices[count++].m_f4Color = DEFINE::COLOR_RED;
	vertices[count].m_f3Pos = XMFLOAT3(0.0f, 0.0f, 0.0f); vertices[count++].m_f4Color = DEFINE::COLOR_GREEN;
	vertices[count].m_f3Pos = XMFLOAT3(0.0f, 1.0f, 0.0f); vertices[count++].m_f4Color = DEFINE::COLOR_GREEN;
	vertices[count].m_f3Pos = XMFLOAT3(0.0f, 0.0f, 0.0f); vertices[count++].m_f4Color = DEFINE::COLOR_BLUE;
	vertices[count].m_f3Pos = XMFLOAT3(0.0f, 0.0f, 1.0f); vertices[count++].m_f4Color = DEFINE::COLOR_BLUE;

	for (size_t i = 0; i < coneX.Vertices.size(); ++i, ++count)
	{
		vertices[count].m_f3Pos = coneX.Vertices[i].Position;
		vertices[count].m_f4Color = DEFINE::COLOR_RED;
	}

	for (size_t i = 0; i < coneY.Vertices.size(); ++i, ++count)
	{
		vertices[count].m_f3Pos = coneY.Vertices[i].Position;
		vertices[count].m_f4Color = DEFINE::COLOR_GREEN;
	}

	for (size_t i = 0; i < coneZ.Vertices.size(); ++i, ++count)
	{
		vertices[count].m_f3Pos = coneZ.Vertices[i].Position;
		vertices[count].m_f4Color = DEFINE::COLOR_BLUE;
	}

	std::vector<UINT> indices(vertexCount);
	for (int i = 0; i < vertexCount; ++i)
		indices[i] = i;

	indices.insert(indices.end(), coneX.Indices.begin(), coneX.Indices.end());
	indices.insert(indices.end(), coneY.Indices.begin(), coneY.Indices.end());
	indices.insert(indices.end(), coneZ.Indices.begin(), coneZ.Indices.end());

	CreateVertexBuffer(vertices);
	CreateIndexBuffer(indices);
}

void TransGizmo::WorldMatrixSRT()
{
	XMMATRIX scale = XMMatrixScaling(m_f3Scale.x, m_f3Scale.y, m_f3Scale.z);
	XMMATRIX rot = XMMatrixRotationRollPitchYaw(XMConvertToRadians(m_f3Rotation.x), XMConvertToRadians(m_f3Rotation.y), XMConvertToRadians(m_f3Rotation.z));
	XMMATRIX trans = XMMatrixTranslation(m_vPosition.x, m_vPosition.y, m_vPosition.z);
	m_World = XMMatrixIdentity();
	m_World = XMMatrixMultiply(XMMatrixMultiply(XMMatrixMultiply(m_World, scale), rot), trans);

	TransformAxisAlignedBox(&m_AxisAlignedBoxX, &m_AxisAlignedBoxX, XMLoadFloat3(&m_f3Scale), XMQuaternionRotationMatrix(rot), XMLoadFloat3(&XMFLOAT3(m_vPosition.x + 0.5f, m_vPosition.y, m_vPosition.z)));
	TransformAxisAlignedBox(&m_AxisAlignedBoxY, &m_AxisAlignedBoxY, XMLoadFloat3(&m_f3Scale), XMQuaternionRotationMatrix(rot), XMLoadFloat3(&XMFLOAT3(m_vPosition.x, m_vPosition.y + 0.5f, m_vPosition.z)));
	TransformAxisAlignedBox(&m_AxisAlignedBoxZ, &m_AxisAlignedBoxZ, XMLoadFloat3(&m_f3Scale), XMQuaternionRotationMatrix(rot), XMLoadFloat3(&XMFLOAT3(m_vPosition.x, m_vPosition.y, m_vPosition.z + 0.5f)));
	
	m_OrientedBoxX.Center = XMFLOAT3(m_vPosition.x + 0.5f, m_vPosition.y, m_vPosition.z);
	m_OrientedBoxY.Center = XMFLOAT3(m_vPosition.x, m_vPosition.y + 0.5f, m_vPosition.z);
	m_OrientedBoxZ.Center = XMFLOAT3(m_vPosition.x, m_vPosition.y, m_vPosition.z + 0.5f);
	XMStoreFloat4(&m_OrientedBoxX.Orientation, XMQuaternionRotationRollPitchYaw(XMConvertToRadians(m_f3Rotation.x), XMConvertToRadians(m_f3Rotation.y), XMConvertToRadians(m_f3Rotation.z)));
	XMStoreFloat4(&m_OrientedBoxY.Orientation, XMQuaternionRotationRollPitchYaw(XMConvertToRadians(m_f3Rotation.x), XMConvertToRadians(m_f3Rotation.y), XMConvertToRadians(m_f3Rotation.z)));
	XMStoreFloat4(&m_OrientedBoxZ.Orientation, XMQuaternionRotationRollPitchYaw(XMConvertToRadians(m_f3Rotation.x), XMConvertToRadians(m_f3Rotation.y), XMConvertToRadians(m_f3Rotation.z)));
	//TransformOrientedBox(&m_OrientedBoxX, &m_OrientedBoxX, 1.0f, XMQuaternionRotationMatrix(rot), XMLoadFloat3(&XMFLOAT3(0.0f, 0.0f, 0.0f)));
	//TransformOrientedBox(&m_OrientedBoxY, &m_OrientedBoxY, 1.0f, XMQuaternionRotationMatrix(rot), XMLoadFloat3(&XMFLOAT3(0.0f, 0.0f, 0.0f)));
	//TransformOrientedBox(&m_OrientedBoxZ, &m_OrientedBoxZ, 1.0f, XMQuaternionRotationMatrix(rot), XMLoadFloat3(&XMFLOAT3(0.0f, 0.0f, 0.0f)));
	//std::cout << "postion: " << m_vPosition.x << ", " << m_vPosition.y << ", " << m_vPosition.z << std::endl;
	//std::cout << "X: " << m_OrientedBoxX.Center.x << ", " << m_OrientedBoxX.Center.y << ", " << m_OrientedBoxX.Center.z << std::endl;
	//std::cout << "Y: " << m_OrientedBoxY.Center.x << ", " << m_OrientedBoxY.Center.y << ", " << m_OrientedBoxY.Center.z << std::endl;
	//std::cout << "Z: " << m_OrientedBoxZ.Center.x << ", " << m_OrientedBoxZ.Center.y << ", " << m_OrientedBoxZ.Center.z << std::endl;
}

void TransGizmo::Render()
{
	ConstantBuffer cb;
	cb.m_mtWorld = XMMatrixTranspose(m_World);
	cb.m_mtView = XMMatrixTranspose(Camera::GetInstance().View());
	cb.m_mtProjection = XMMatrixTranspose(Camera::GetInstance().Proj());
	DeviceManager::GetInstance().GetDeviceContext()->UpdateSubresource(m_pConstantBuffer, 0, NULL, &cb, 0, 0);

	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
	DeviceManager::GetInstance().GetDeviceContext()->IASetInputLayout(m_pVertexLayout);
	DeviceManager::GetInstance().GetDeviceContext()->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);
	DeviceManager::GetInstance().GetDeviceContext()->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	DeviceManager::GetInstance().GetDeviceContext()->RSSetState(m_pRasterizerStateSolid);
	DeviceManager::GetInstance().GetDeviceContext()->VSSetShader(m_pVertexShader, NULL, 0);
	DeviceManager::GetInstance().GetDeviceContext()->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	DeviceManager::GetInstance().GetDeviceContext()->PSSetShader(m_pPixelShader, NULL, 0);

	DeviceManager::GetInstance().GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	DeviceManager::GetInstance().GetDeviceContext()->DrawIndexed(m_uiIndexCount, m_uiIndexOffset, m_iVertexOffset);
	DeviceManager::GetInstance().GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	DeviceManager::GetInstance().GetDeviceContext()->DrawIndexed(m_uiIndexCountCone, m_uiIndexCount, 6);
	DeviceManager::GetInstance().GetDeviceContext()->DrawIndexed(m_uiIndexCountCone, m_uiIndexCount + m_uiIndexCountCone, 6 + 30);
	DeviceManager::GetInstance().GetDeviceContext()->DrawIndexed(m_uiIndexCountCone, m_uiIndexCount + m_uiIndexCountCone * 2, 6 + 60);
}

/////////////////////////////////////////////////////////////////////////////////////////
RotationGizmo::RotationGizmo()
{
	Init();

	m_Sphere.Center = m_vPosition;
	m_Sphere.Radius = 1.0f;

	// obb
	XMFLOAT3 posX = m_vPosition; posX.x += 0.5f;
	m_OrientedBoxX.Center = posX;
	m_OrientedBoxX.Extents = XMFLOAT3(0.5f, 0.05f, 0.05f);
	m_OrientedBoxX.Orientation = XMFLOAT4(0.f, 0.f, 0.f, 1.f);

	XMFLOAT3 posY = m_vPosition; posY.y += 0.5f;
	m_OrientedBoxY.Center = posY;
	m_OrientedBoxY.Extents = XMFLOAT3(0.05f, 0.5f, 0.05f);
	m_OrientedBoxY.Orientation = XMFLOAT4(0.f, 0.f, 0.f, 1.f);

	XMFLOAT3 posZ = m_vPosition; posZ.z += 0.5f;
	m_OrientedBoxZ.Center = posZ;
	m_OrientedBoxZ.Extents = XMFLOAT3(0.05f, 0.05f, 0.5f);
	m_OrientedBoxZ.Orientation = XMFLOAT4(0.f, 0.f, 0.f, 1.f);
}

RotationGizmo::~RotationGizmo()
{

}

void	RotationGizmo::CreateFigure(GeometryGenerator::MeshData& kMeshData)
{

}

void	RotationGizmo::BuildGeometryBuffers()
{
	const int vertexCount = 6;
	const int slice = 10;
	std::vector<SimpleVertex> vertices(6 + (360 / slice + 1) * 3);
	std::vector<UINT> indices(6 + (360 / slice + 1) * 3);

	int count = 0;
	vertices[count].m_f3Pos = XMFLOAT3(0.0f, 0.0f, 0.0f); vertices[count++].m_f4Color = DEFINE::COLOR_RED;
	vertices[count].m_f3Pos = XMFLOAT3(1.0f, 0.0f, 0.0f); vertices[count++].m_f4Color = DEFINE::COLOR_RED;
	vertices[count].m_f3Pos = XMFLOAT3(0.0f, 0.0f, 0.0f); vertices[count++].m_f4Color = DEFINE::COLOR_GREEN;
	vertices[count].m_f3Pos = XMFLOAT3(0.0f, 1.0f, 0.0f); vertices[count++].m_f4Color = DEFINE::COLOR_GREEN;
	vertices[count].m_f3Pos = XMFLOAT3(0.0f, 0.0f, 0.0f); vertices[count++].m_f4Color = DEFINE::COLOR_BLUE;
	vertices[count].m_f3Pos = XMFLOAT3(0.0f, 0.0f, 1.0f); vertices[count++].m_f4Color = DEFINE::COLOR_BLUE;
	
	for (int i = 0; i < vertexCount; ++i)
		indices[i] = i;

	for (int i = 0; i * slice <= 360; ++i, ++count)
	{
		vertices[count].m_f3Pos.y = cosf(XMConvertToRadians(static_cast<float>(i * slice)));
		vertices[count].m_f3Pos.z = sinf(XMConvertToRadians(static_cast<float>(i * slice)));
		vertices[count].m_f4Color = DEFINE::COLOR_RED;
		indices[count] = i;
	}

	for (int i = 0; i * slice <= 360; ++i, ++count)
	{
		vertices[count].m_f3Pos.x = cosf(XMConvertToRadians(static_cast<float>(i * slice)));
		vertices[count].m_f3Pos.z = sinf(XMConvertToRadians(static_cast<float>(i * slice)));
		vertices[count].m_f4Color = DEFINE::COLOR_GREEN;
		indices[count] = i;
	}

	for (int i = 0; i * slice <= 360; ++i, ++count)
	{
		vertices[count].m_f3Pos.x = cosf(XMConvertToRadians(static_cast<float>(i * slice)));
		vertices[count].m_f3Pos.y = sinf(XMConvertToRadians(static_cast<float>(i * slice)));
		vertices[count].m_f4Color = DEFINE::COLOR_BLUE;
		indices[count] = i;
	}

	m_iVertexOffset = 0;
	m_uiIndexCount = static_cast<UINT>((indices.size() - 6) / 3);
	m_uiIndexOffset = 0;

	CreateVertexBuffer(vertices);
	CreateIndexBuffer(indices);
}

void RotationGizmo::WorldMatrixSRT()
{
	XMMATRIX scale = XMMatrixScaling(m_f3Scale.x, m_f3Scale.y, m_f3Scale.z);
	XMMATRIX rot = XMMatrixRotationRollPitchYaw(XMConvertToRadians(m_f3Rotation.x), XMConvertToRadians(m_f3Rotation.y), XMConvertToRadians(m_f3Rotation.z));
	XMMATRIX trans = XMMatrixTranslation(m_vPosition.x, m_vPosition.y, m_vPosition.z);
	m_World = XMMatrixIdentity();
	m_World = XMMatrixMultiply(XMMatrixMultiply(XMMatrixMultiply(m_World, scale), rot), trans);

	TransformSphere(&m_Sphere, &m_Sphere, m_f3Scale.x, XMQuaternionRotationMatrix(rot), XMLoadFloat3(&m_vPosition));

	m_OrientedBoxX.Center = XMFLOAT3(m_vPosition.x + 0.5f, m_vPosition.y, m_vPosition.z);
	m_OrientedBoxY.Center = XMFLOAT3(m_vPosition.x, m_vPosition.y + 0.5f, m_vPosition.z);
	m_OrientedBoxZ.Center = XMFLOAT3(m_vPosition.x, m_vPosition.y, m_vPosition.z + 0.5f);
	XMStoreFloat4(&m_OrientedBoxX.Orientation, XMQuaternionRotationRollPitchYaw(XMConvertToRadians(m_f3Rotation.x), XMConvertToRadians(m_f3Rotation.y), XMConvertToRadians(m_f3Rotation.z)));
	XMStoreFloat4(&m_OrientedBoxY.Orientation, XMQuaternionRotationRollPitchYaw(XMConvertToRadians(m_f3Rotation.x), XMConvertToRadians(m_f3Rotation.y), XMConvertToRadians(m_f3Rotation.z)));
	XMStoreFloat4(&m_OrientedBoxZ.Orientation, XMQuaternionRotationRollPitchYaw(XMConvertToRadians(m_f3Rotation.x), XMConvertToRadians(m_f3Rotation.y), XMConvertToRadians(m_f3Rotation.z)));
}

void	RotationGizmo::Render()
{
	ConstantBuffer cb;
	cb.m_mtWorld = XMMatrixTranspose(m_World);
	cb.m_mtView = XMMatrixTranspose(Camera::GetInstance().View());
	cb.m_mtProjection = XMMatrixTranspose(Camera::GetInstance().Proj());
	DeviceManager::GetInstance().GetDeviceContext()->UpdateSubresource(m_pConstantBuffer, 0, NULL, &cb, 0, 0);

	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
	DeviceManager::GetInstance().GetDeviceContext()->IASetInputLayout(m_pVertexLayout);
	DeviceManager::GetInstance().GetDeviceContext()->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);
	DeviceManager::GetInstance().GetDeviceContext()->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	DeviceManager::GetInstance().GetDeviceContext()->RSSetState(m_pRasterizerStateSolid);
	DeviceManager::GetInstance().GetDeviceContext()->VSSetShader(m_pVertexShader, NULL, 0);
	DeviceManager::GetInstance().GetDeviceContext()->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	DeviceManager::GetInstance().GetDeviceContext()->PSSetShader(m_pPixelShader, NULL, 0);

	DeviceManager::GetInstance().GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	DeviceManager::GetInstance().GetDeviceContext()->DrawIndexed(6, m_uiIndexOffset, m_iVertexOffset);
	DeviceManager::GetInstance().GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
	DeviceManager::GetInstance().GetDeviceContext()->DrawIndexed(m_uiIndexCount, 6, 6);
	DeviceManager::GetInstance().GetDeviceContext()->DrawIndexed(m_uiIndexCount, 6 + m_uiIndexCount, 6 + m_uiIndexCount);
	DeviceManager::GetInstance().GetDeviceContext()->DrawIndexed(m_uiIndexCount, 6 + m_uiIndexCount * 2, 6 + m_uiIndexCount * 2);
}

/////////////////////////////////////////////////////////////////////////////////////////
ScalingGizmo::ScalingGizmo()
{
	Init();

	// aabb
	XMFLOAT3 posX = m_vPosition; posX.x += 0.5f;
	m_AxisAlignedBoxX.Center = posX;
	m_AxisAlignedBoxX.Extents = XMFLOAT3(0.5f, 0.05f, 0.05f);
	m_AxisAlignedBoxX.Scale = XMFLOAT3(1.f, 1.f, 1.f);

	XMFLOAT3 posY = m_vPosition; posY.y += 0.5f;
	m_AxisAlignedBoxY.Center = posY;
	m_AxisAlignedBoxY.Extents = XMFLOAT3(0.05f, 0.5f, 0.05f);
	m_AxisAlignedBoxY.Scale = XMFLOAT3(1.f, 1.f, 1.f);

	XMFLOAT3 posZ = m_vPosition; posZ.z += 0.5f;
	m_AxisAlignedBoxZ.Center = posZ;
	m_AxisAlignedBoxZ.Extents = XMFLOAT3(0.05f, 0.05f, 0.5f);
	m_AxisAlignedBoxZ.Scale = XMFLOAT3(1.f, 1.f, 1.f);

	// obb
	posX = m_vPosition; posX.x += 0.5f;
	m_OrientedBoxX.Center = posX;
	m_OrientedBoxX.Extents = XMFLOAT3(0.5f, 0.05f, 0.05f);
	m_OrientedBoxX.Orientation = XMFLOAT4(0.f, 0.f, 0.f, 1.f);

	posY = m_vPosition; posY.y += 0.5f;
	m_OrientedBoxY.Center = posY;
	m_OrientedBoxY.Extents = XMFLOAT3(0.05f, 0.5f, 0.05f);
	m_OrientedBoxY.Orientation = XMFLOAT4(0.f, 0.f, 0.f, 1.f);

	posZ = m_vPosition; posZ.z += 0.5f;
	m_OrientedBoxZ.Center = posZ;
	m_OrientedBoxZ.Extents = XMFLOAT3(0.05f, 0.05f, 0.5f);
	m_OrientedBoxZ.Orientation = XMFLOAT4(0.f, 0.f, 0.f, 1.f);
}

ScalingGizmo::~ScalingGizmo()
{

}

void	ScalingGizmo::CreateFigure(GeometryGenerator::MeshData& kMeshData)
{

}

void	ScalingGizmo::BuildGeometryBuffers()
{
	GeometryGenerator::MeshData objectX;
	GeometryGenerator::MeshData objectY;
	GeometryGenerator::MeshData objectZ;
	GeometryGenerator::MeshData objectCenter;

	GeometryGenerator geoGen;

	geoGen.CreateBox(0.1f, 0.1f, 0.1f, objectX);
	geoGen.CreateBox(0.1f, 0.1f, 0.1f, objectY);
	geoGen.CreateBox(0.1f, 0.1f, 0.1f, objectZ);
	geoGen.CreateBox(0.1f, 0.1f, 0.1f, objectCenter);

	for (int i = 0; i < objectX.Vertices.size(); ++i)
	{
		objectX.Vertices[i].Position.x += 1.0f;
	}

	for (int i = 0; i < objectY.Vertices.size(); ++i)
	{
		objectY.Vertices[i].Position.y += 1.0f;
	}

	for (int i = 0; i < objectZ.Vertices.size(); ++i)
	{
		objectZ.Vertices[i].Position.z += 1.0f;
	}

	const int vertexCount = 6;
	std::vector<SimpleVertex> vertices(vertexCount + objectX.Vertices.size() + objectY.Vertices.size() + objectZ.Vertices.size() + objectCenter.Vertices.size());
	int count = 0;
	vertices[count].m_f3Pos = XMFLOAT3(0.0f, 0.0f, 0.0f); vertices[count++].m_f4Color = DEFINE::COLOR_RED;
	vertices[count].m_f3Pos = XMFLOAT3(1.0f, 0.0f, 0.0f); vertices[count++].m_f4Color = DEFINE::COLOR_RED;
	vertices[count].m_f3Pos = XMFLOAT3(0.0f, 0.0f, 0.0f); vertices[count++].m_f4Color = DEFINE::COLOR_GREEN;
	vertices[count].m_f3Pos = XMFLOAT3(0.0f, 1.0f, 0.0f); vertices[count++].m_f4Color = DEFINE::COLOR_GREEN;
	vertices[count].m_f3Pos = XMFLOAT3(0.0f, 0.0f, 0.0f); vertices[count++].m_f4Color = DEFINE::COLOR_BLUE;
	vertices[count].m_f3Pos = XMFLOAT3(0.0f, 0.0f, 1.0f); vertices[count++].m_f4Color = DEFINE::COLOR_BLUE;

	for (size_t i = 0; i < objectX.Vertices.size(); ++i, ++count)
	{
		vertices[count].m_f3Pos = objectX.Vertices[i].Position;
		vertices[count].m_f4Color = DEFINE::COLOR_RED;
	}

	for (size_t i = 0; i < objectY.Vertices.size(); ++i, ++count)
	{
		vertices[count].m_f3Pos = objectY.Vertices[i].Position;
		vertices[count].m_f4Color = DEFINE::COLOR_GREEN;
	}

	for (size_t i = 0; i < objectZ.Vertices.size(); ++i, ++count)
	{
		vertices[count].m_f3Pos = objectZ.Vertices[i].Position;
		vertices[count].m_f4Color = DEFINE::COLOR_BLUE;
	}

	for (size_t i = 0; i < objectCenter.Vertices.size(); ++i, ++count)
	{
		vertices[count].m_f3Pos = objectCenter.Vertices[i].Position;
		vertices[count].m_f4Color = DEFINE::COLOR_YELLOW;
	}

	std::vector<UINT> indices(vertexCount);
	for (int i = 0; i < vertexCount; ++i)
		indices[i] = i;

	indices.insert(indices.end(), objectX.Indices.begin(), objectX.Indices.end());
	indices.insert(indices.end(), objectY.Indices.begin(), objectY.Indices.end());
	indices.insert(indices.end(), objectZ.Indices.begin(), objectZ.Indices.end());
	indices.insert(indices.end(), objectCenter.Indices.begin(), objectCenter.Indices.end());

	m_iVertexOffset = 0;
	m_uiIndexCount = 36;
	m_uiIndexOffset = 0;

	std::cout << m_uiIndexCount << std::endl;
	CreateVertexBuffer(vertices);
	CreateIndexBuffer(indices);
}

void ScalingGizmo::WorldMatrixSRT()
{
	XMMATRIX scale = XMMatrixScaling(m_f3Scale.x, m_f3Scale.y, m_f3Scale.z);
	XMMATRIX rot = XMMatrixRotationRollPitchYaw(XMConvertToRadians(m_f3Rotation.x), XMConvertToRadians(m_f3Rotation.y), XMConvertToRadians(m_f3Rotation.z));
	XMMATRIX trans = XMMatrixTranslation(m_vPosition.x, m_vPosition.y, m_vPosition.z);
	m_World = XMMatrixIdentity();
	m_World = XMMatrixMultiply(XMMatrixMultiply(XMMatrixMultiply(m_World, scale), rot), trans);

	TransformAxisAlignedBox(&m_AxisAlignedBoxX, &m_AxisAlignedBoxX, XMLoadFloat3(&m_f3Scale), XMQuaternionRotationMatrix(rot), XMLoadFloat3(&XMFLOAT3(m_vPosition.x + 0.5f, m_vPosition.y, m_vPosition.z)));
	TransformAxisAlignedBox(&m_AxisAlignedBoxY, &m_AxisAlignedBoxY, XMLoadFloat3(&m_f3Scale), XMQuaternionRotationMatrix(rot), XMLoadFloat3(&XMFLOAT3(m_vPosition.x, m_vPosition.y + 0.5f, m_vPosition.z)));
	TransformAxisAlignedBox(&m_AxisAlignedBoxZ, &m_AxisAlignedBoxZ, XMLoadFloat3(&m_f3Scale), XMQuaternionRotationMatrix(rot), XMLoadFloat3(&XMFLOAT3(m_vPosition.x, m_vPosition.y, m_vPosition.z + 0.5f)));
	
	m_OrientedBoxX.Center = XMFLOAT3(m_vPosition.x + 0.5f, m_vPosition.y, m_vPosition.z);
	m_OrientedBoxY.Center = XMFLOAT3(m_vPosition.x, m_vPosition.y + 0.5f, m_vPosition.z);
	m_OrientedBoxZ.Center = XMFLOAT3(m_vPosition.x, m_vPosition.y, m_vPosition.z + 0.5f);
	XMStoreFloat4(&m_OrientedBoxX.Orientation, XMQuaternionRotationRollPitchYaw(XMConvertToRadians(m_f3Rotation.x), XMConvertToRadians(m_f3Rotation.y), XMConvertToRadians(m_f3Rotation.z)));
	XMStoreFloat4(&m_OrientedBoxY.Orientation, XMQuaternionRotationRollPitchYaw(XMConvertToRadians(m_f3Rotation.x), XMConvertToRadians(m_f3Rotation.y), XMConvertToRadians(m_f3Rotation.z)));
	XMStoreFloat4(&m_OrientedBoxZ.Orientation, XMQuaternionRotationRollPitchYaw(XMConvertToRadians(m_f3Rotation.x), XMConvertToRadians(m_f3Rotation.y), XMConvertToRadians(m_f3Rotation.z)));
}

void	ScalingGizmo::Render()
{
	ConstantBuffer cb;
	cb.m_mtWorld = XMMatrixTranspose(m_World);
	cb.m_mtView = XMMatrixTranspose(Camera::GetInstance().View());
	cb.m_mtProjection = XMMatrixTranspose(Camera::GetInstance().Proj());
	DeviceManager::GetInstance().GetDeviceContext()->UpdateSubresource(m_pConstantBuffer, 0, NULL, &cb, 0, 0);

	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
	DeviceManager::GetInstance().GetDeviceContext()->IASetInputLayout(m_pVertexLayout);
	DeviceManager::GetInstance().GetDeviceContext()->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);
	DeviceManager::GetInstance().GetDeviceContext()->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	DeviceManager::GetInstance().GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	DeviceManager::GetInstance().GetDeviceContext()->RSSetState(m_pRasterizerStateSolid);
	DeviceManager::GetInstance().GetDeviceContext()->VSSetShader(m_pVertexShader, NULL, 0);
	DeviceManager::GetInstance().GetDeviceContext()->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	DeviceManager::GetInstance().GetDeviceContext()->PSSetShader(m_pPixelShader, NULL, 0);

	DeviceManager::GetInstance().GetDeviceContext()->DrawIndexed(6, m_uiIndexOffset, m_iVertexOffset);
	DeviceManager::GetInstance().GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	DeviceManager::GetInstance().GetDeviceContext()->DrawIndexed(m_uiIndexCount, 6, 6);
	DeviceManager::GetInstance().GetDeviceContext()->DrawIndexed(m_uiIndexCount, 6 + m_uiIndexCount, 30);
	DeviceManager::GetInstance().GetDeviceContext()->DrawIndexed(m_uiIndexCount, 6 + m_uiIndexCount * 2, 54);
	DeviceManager::GetInstance().GetDeviceContext()->DrawIndexed(m_uiIndexCount, 6 + m_uiIndexCount * 3, 78);
}