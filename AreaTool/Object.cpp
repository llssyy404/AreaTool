#include "stdafx.h"
#include "Define.h"
#include "DeviceManager.h"
#include "GeometryGenerator.h"
#include "Camera.h"

#include "Object.h"

Object::Object()
{
	m_bSelect = false;

	m_f3Scale = XMFLOAT3(1.f, 1.f, 1.f);
	m_vRight = XMFLOAT3(1.f, 0.f, 0.f);
	m_vUp = XMFLOAT3(0.f, 1.f, 0.f);
	m_vLook = XMFLOAT3(0.f, 0.f, 1.f);
	m_vPosition = XMFLOAT3(0.f, 0.f, 0.f);
	m_f3Rotation = XMFLOAT3(0.f, 0.f, 0.f);

	m_AxisAlignedBox.Center = m_vPosition;
	m_AxisAlignedBox.Extents = XMFLOAT3(0.5f, 0.5f, 0.5f);
	m_AxisAlignedBox.Scale = XMFLOAT3(1.f, 1.f, 1.f);
}

Object::~Object()
{
	if (m_pRasterizerStateSolid) m_pRasterizerStateSolid->Release();
	if (m_pRasterizerStateWire) m_pRasterizerStateWire->Release();
	if (m_pConstantBuffer) m_pConstantBuffer->Release();
	if (m_pVertexBuffer) m_pVertexBuffer->Release();
	if (m_pIndexBuffer) m_pIndexBuffer->Release();
	if (m_pVertexLayout) m_pVertexLayout->Release();
	if (m_pVertexShader) m_pVertexShader->Release();
	if (m_pPixelShader) m_pPixelShader->Release();
}

HRESULT Object::Init()
{
	HRESULT hr = S_OK;
	ID3DBlob* pVSBlob = NULL;
	hr = CreateVertexShader(pVSBlob);
	if (FAILED(hr))
	{
		std::cout << "Failed CreateVertexShader" << std::endl;
		return hr;
	}

	hr = CreateInputLayout(pVSBlob);
	if (FAILED(hr))
	{
		std::cout << "Failed CreateInputLayout" << std::endl;
		return hr;
	}

	hr = CreatePixelShader();
	if (FAILED(hr))
	{
		std::cout << "Failed CreatePixelShader" << std::endl;
		return hr;
	}

	BuildGeometryBuffers();

	hr = CreateConstantBuffer();
	if (FAILED(hr))
	{
		std::cout << "Failed CreateConstantBuffer" << std::endl;
		return hr;
	}

	hr = CreateRasterizerState();
	if (FAILED(hr))
	{
		std::cout << "Failed CreateRasterizerState" << std::endl;
		return hr;
	}

	XMMATRIX g_World = XMMatrixIdentity();
	XMStoreFloat4x4(&m_f4x4World, g_World);

	m_World = XMMatrixIdentity();

	return hr;
}

//--------------------------------------------------------------------------------------
// Helper for compiling shaders with D3DX11
//--------------------------------------------------------------------------------------
HRESULT Object::CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
	HRESULT hr = S_OK;

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
	// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows 
	// the shaders to be optimized and to run exactly the way they will run in 
	// the release configuration of this program.
	dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

	ID3DBlob* pErrorBlob;
	hr = D3DX11CompileFromFile(szFileName, NULL, NULL, szEntryPoint, szShaderModel,
		dwShaderFlags, 0, NULL, ppBlobOut, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		if (pErrorBlob != NULL)
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
		if (pErrorBlob) pErrorBlob->Release();
		return hr;
	}
	if (pErrorBlob) pErrorBlob->Release();

	return S_OK;
}

HRESULT Object::CreateRasterizerState()
{
	HRESULT hResult = S_OK;
	D3D11_RASTERIZER_DESC rsDesc;
	ZeroMemory(&rsDesc, sizeof(D3D11_RASTERIZER_DESC));
	rsDesc.FillMode = D3D11_FILL_WIREFRAME;
	rsDesc.CullMode = D3D11_CULL_BACK;
	rsDesc.FrontCounterClockwise = false;
	rsDesc.DepthClipEnable = true;

	hResult = DeviceManager::GetInstance().GetDevice()->CreateRasterizerState(&rsDesc, &m_pRasterizerStateWire);
	if (FAILED(hResult))
		return hResult;

	ZeroMemory(&rsDesc, sizeof(D3D11_RASTERIZER_DESC));
	rsDesc.FillMode = D3D11_FILL_SOLID;
	rsDesc.CullMode = D3D11_CULL_NONE;
	rsDesc.FrontCounterClockwise = false;
	rsDesc.DepthClipEnable = true;

	hResult = DeviceManager::GetInstance().GetDevice()->CreateRasterizerState(&rsDesc, &m_pRasterizerStateSolid);
	if (FAILED(hResult))
		return hResult;

	return hResult;
}

HRESULT Object::CreateConstantBuffer()
{
	HRESULT hResult = S_OK;
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hResult = DeviceManager::GetInstance().GetDevice()->CreateBuffer(&bd, NULL, &m_pConstantBuffer);

	return hResult;
}

HRESULT Object::CreatePixelShader()
{
	HRESULT hResult = S_OK;
	ID3DBlob* pPSBlob = NULL;
	hResult = CompileShaderFromFile(L"AreaTool.fx", "PS", "ps_4_0", &pPSBlob);
	if (FAILED(hResult))
	{
		MessageBox(NULL,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hResult;
	}

	hResult = DeviceManager::GetInstance().GetDevice()->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &m_pPixelShader);
	pPSBlob->Release();

	return hResult;
}

HRESULT Object::CreateInputLayout(ID3DBlob * &pVSBlob)
{
	HRESULT hResult = S_OK;
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = ARRAYSIZE(layout);

	// Create the input layout
	hResult = DeviceManager::GetInstance().GetDevice()->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(), &m_pVertexLayout);
	pVSBlob->Release();
	if (FAILED(hResult))
		return hResult;

	return hResult;
}

HRESULT Object::CreateVertexShader(ID3DBlob * &pVSBlob)
{
	HRESULT hResult = S_OK;
	hResult = CompileShaderFromFile(L"AreaTool.fx", "VS", "vs_4_0", &pVSBlob);
	if (FAILED(hResult))
	{
		MessageBox(NULL,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hResult;
	}

	hResult = DeviceManager::GetInstance().GetDevice()->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &m_pVertexShader);
	if (FAILED(hResult))
	{
		pVSBlob->Release();
		return hResult;
	}

	return hResult;
}

void Object::BuildGeometryBuffers()
{
	GeometryGenerator::MeshData object;
	XMFLOAT4 color = DEFINE::COLOR_WHITE;

	CreateFigure(object);

	m_iVertexOffset = 0;
	m_uiIndexCount = static_cast<UINT>(object.Indices.size());
	m_uiIndexOffset = 0;

	std::vector<SimpleVertex> vertices(object.Vertices.size());

	for (size_t i = 0; i < object.Vertices.size(); ++i)
	{
		vertices[i].m_f3Pos = object.Vertices[i].Position;
		vertices[i].m_f4Color = color;
	}

	CreateVertexBuffer(vertices);
	CreateIndexBuffer(object.Indices);
}

void Object::CreateIndexBuffer(const std::vector<UINT> &indices)
{
	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * static_cast<UINT>(indices.size());
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	DeviceManager::GetInstance().GetDevice()->CreateBuffer(&ibd, &iinitData, &m_pIndexBuffer);
}

void Object::CreateVertexBuffer(const std::vector<SimpleVertex> &vertices)
{
	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(SimpleVertex) * static_cast<UINT>(vertices.size());
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	DeviceManager::GetInstance().GetDevice()->CreateBuffer(&vbd, &vinitData, &m_pVertexBuffer);
}

void Object::WorldMatrixSRT()
{
	XMMATRIX scale = XMMatrixScaling(m_f3Scale.x, m_f3Scale.y, m_f3Scale.z);
	XMMATRIX rot = XMMatrixRotationRollPitchYaw(XMConvertToRadians(m_f3Rotation.x), XMConvertToRadians(m_f3Rotation.y), XMConvertToRadians(m_f3Rotation.z));
	XMMATRIX trans = XMMatrixTranslation(m_vPosition.x, m_vPosition.y, m_vPosition.z);
	m_World = XMMatrixIdentity();
	m_World = XMMatrixMultiply(XMMatrixMultiply(XMMatrixMultiply(m_World, scale), rot), trans);
	
	TransformAxisAlignedBox(&m_AxisAlignedBox, &m_AxisAlignedBox, XMLoadFloat3(&m_f3Scale), XMQuaternionRotationMatrix(rot), XMLoadFloat3(&m_vPosition));
}

void Object::Pitch(float fAngle)
{
	m_f3Rotation.x += fAngle;
}

void Object::Yaw(float fAngle)
{
	m_f3Rotation.y += fAngle;
}

void Object::Roll(float fAngle)
{
	m_f3Rotation.z += fAngle;
}

void Object::Right(float d)
{
	XMVECTOR s = XMVectorReplicate(d);
	XMVECTOR r = XMLoadFloat3(&m_vRight);
	XMVECTOR position = XMLoadFloat3(&m_vPosition);
	XMStoreFloat3(&m_vPosition, XMVectorMultiplyAdd(s, r, position));
}

void Object::Forward(float d)
{
	XMVECTOR s = XMVectorReplicate(d);
	XMVECTOR l = XMLoadFloat3(&m_vLook);
	XMVECTOR position = XMLoadFloat3(&m_vPosition);
	XMStoreFloat3(&m_vPosition, XMVectorMultiplyAdd(s, l, position));
}

void Object::Up(float d)
{
	XMVECTOR s = XMVectorReplicate(d);
	XMVECTOR u = XMLoadFloat3(&m_vUp);
	XMVECTOR position = XMLoadFloat3(&m_vPosition);
	XMStoreFloat3(&m_vPosition, XMVectorMultiplyAdd(s, u, position));
}

void Object::ScalingX(float size)
{
	if (0 >= m_f3Scale.x + size)
		return;

	m_f3Scale.x += size;
}

void Object::ScalingY(float size)
{
	if (0 >= m_f3Scale.y + size)
		return;

	m_f3Scale.y += size;
}

void Object::ScalingZ(float size)
{
	if (0 >= m_f3Scale.z + size)
		return;

	m_f3Scale.z += size;
}

void Object::AnimateObjects(float fTimeElapsed)
{
	WorldMatrixSRT();
}

void Object::Render()
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
	DeviceManager::GetInstance().GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	
	if(m_bSelect)
		DeviceManager::GetInstance().GetDeviceContext()->RSSetState(m_pRasterizerStateSolid);
	else
		DeviceManager::GetInstance().GetDeviceContext()->RSSetState(m_pRasterizerStateWire);
	DeviceManager::GetInstance().GetDeviceContext()->VSSetShader(m_pVertexShader, NULL, 0);
	DeviceManager::GetInstance().GetDeviceContext()->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	DeviceManager::GetInstance().GetDeviceContext()->PSSetShader(m_pPixelShader, NULL, 0);
	
	DeviceManager::GetInstance().GetDeviceContext()->DrawIndexed(m_uiIndexCount, m_uiIndexOffset, m_iVertexOffset);
}

///////////////////////////////////////////////////////////////////////////
Box::Box() : Object()
{
	Init();
}

Box::~Box()
{

}

void Box::CreateFigure(GeometryGenerator::MeshData& kMeshData)
{
	GeometryGenerator geoGen;
	geoGen.CreateBox(1.0f, 1.0f, 1.0f, kMeshData);
}

///////////////////////////////////////////////////////////////////////////
Cylinder::Cylinder() : Object()
{
	Init();

}

Cylinder::~Cylinder()
{

}

void Cylinder::CreateFigure(GeometryGenerator::MeshData& kMeshData)
{
	GeometryGenerator geoGen;
	geoGen.CreateCylinder(0.5f, 0.5f, 1.0f, 8, 1, kMeshData);
}

///////////////////////////////////////////////////////////////////////////
Sphere::Sphere() : Object()
{
	Init();
}

Sphere::~Sphere()
{

}

void Sphere::CreateFigure(GeometryGenerator::MeshData& kMeshData)
{
	GeometryGenerator geoGen;
	geoGen.CreateSphere(0.5f, 10, 10, kMeshData);
}

///////////////////////////////////////////////////////////////////////////
Grid::Grid() : Object()
{
	Init();

	m_AxisAlignedBox.Center = m_vPosition;
	m_AxisAlignedBox.Extents = XMFLOAT3(0.0f, 0.0f, 0.0f);
}

Grid::~Grid()
{

}

void Grid::CreateFigure(GeometryGenerator::MeshData& kMeshData)
{
	GeometryGenerator geoGen;
	XMFLOAT4 color = DEFINE::COLOR_GRAY;
	geoGen.CreateGrid(30.0f, 30.0f, 30, 30, kMeshData);
}

///////////////////////////////////////////////////////////////////////////
TransGizmo::TransGizmo() : Gizmo()
{
	Init();
	m_eChangeType = DEFINE::C_TRNAS;
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
	vertices[count].m_f3Pos.x = 0.f; vertices[count].m_f3Pos.y = 0.f; vertices[count].m_f3Pos.z = 0.f; vertices[count++].m_f4Color = DEFINE::COLOR_RED;
	vertices[count].m_f3Pos.x = 1.f; vertices[count].m_f3Pos.y = 0.f; vertices[count].m_f3Pos.z = 0.f; vertices[count++].m_f4Color = DEFINE::COLOR_RED;
	vertices[count].m_f3Pos.x = 0.f; vertices[count].m_f3Pos.y = 1.f; vertices[count].m_f3Pos.z = 0.f; vertices[count++].m_f4Color = DEFINE::COLOR_GREEN;
	vertices[count].m_f3Pos.x = 0.f; vertices[count].m_f3Pos.y = 0.f; vertices[count].m_f3Pos.z = 0.f; vertices[count++].m_f4Color = DEFINE::COLOR_GREEN;
	vertices[count].m_f3Pos.x = 0.f; vertices[count].m_f3Pos.y = 0.f; vertices[count].m_f3Pos.z = 0.f; vertices[count++].m_f4Color = DEFINE::COLOR_BLUE;
	vertices[count].m_f3Pos.x = 0.f; vertices[count].m_f3Pos.y = 0.f; vertices[count].m_f3Pos.z = 1.f; vertices[count++].m_f4Color = DEFINE::COLOR_BLUE;

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

	if (DEFINE::C_TRNAS == m_eChangeType)
	{
		DeviceManager::GetInstance().GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
		DeviceManager::GetInstance().GetDeviceContext()->DrawIndexed(m_uiIndexCount, m_uiIndexOffset, m_iVertexOffset);
		DeviceManager::GetInstance().GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		DeviceManager::GetInstance().GetDeviceContext()->DrawIndexed(m_uiIndexCountCone, m_uiIndexCount, 6);
		DeviceManager::GetInstance().GetDeviceContext()->DrawIndexed(m_uiIndexCountCone, m_uiIndexCount + m_uiIndexCountCone, 6+30);
		DeviceManager::GetInstance().GetDeviceContext()->DrawIndexed(m_uiIndexCountCone, m_uiIndexCount + m_uiIndexCountCone * 2, 6+60);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
RotationGizmo::RotationGizmo()
{
	Init();
}

RotationGizmo::~RotationGizmo()
{

}

void	RotationGizmo::CreateFigure(GeometryGenerator::MeshData& kMeshData)
{

}

void	RotationGizmo::BuildGeometryBuffers() 
{
	const int slice = 10;
	std::vector<SimpleVertex> vertices((360 / slice+1)*3);
	std::vector<UINT> indices((360 / slice + 1) * 3);
	int count = 0;
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
	m_uiIndexCount = static_cast<UINT>(indices.size()/3);
	m_uiIndexOffset = 0;

	CreateVertexBuffer(vertices);
	CreateIndexBuffer(indices);
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
	DeviceManager::GetInstance().GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);

	DeviceManager::GetInstance().GetDeviceContext()->RSSetState(m_pRasterizerStateSolid);
	DeviceManager::GetInstance().GetDeviceContext()->VSSetShader(m_pVertexShader, NULL, 0);
	DeviceManager::GetInstance().GetDeviceContext()->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	DeviceManager::GetInstance().GetDeviceContext()->PSSetShader(m_pPixelShader, NULL, 0);

	DeviceManager::GetInstance().GetDeviceContext()->DrawIndexed(m_uiIndexCount, m_uiIndexOffset, m_iVertexOffset);
	DeviceManager::GetInstance().GetDeviceContext()->DrawIndexed(m_uiIndexCount, m_uiIndexCount, m_uiIndexCount);
	DeviceManager::GetInstance().GetDeviceContext()->DrawIndexed(m_uiIndexCount, m_uiIndexCount * 2, m_uiIndexCount*2);
}

/////////////////////////////////////////////////////////////////////////////////////////
ScalingGizmo::ScalingGizmo()
{
	Init();
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
	std::vector<SimpleVertex> vertices(vertexCount+ objectX.Vertices.size() + objectY.Vertices.size() + objectZ.Vertices.size() + objectCenter.Vertices.size());
	int count = 0;
	vertices[count].m_f3Pos.x = 0.f; vertices[count].m_f3Pos.y = 0.f; vertices[count].m_f3Pos.z = 0.f; vertices[count++].m_f4Color = DEFINE::COLOR_RED;
	vertices[count].m_f3Pos.x = 1.f; vertices[count].m_f3Pos.y = 0.f; vertices[count].m_f3Pos.z = 0.f; vertices[count++].m_f4Color = DEFINE::COLOR_RED;
	vertices[count].m_f3Pos.x = 0.f; vertices[count].m_f3Pos.y = 1.f; vertices[count].m_f3Pos.z = 0.f; vertices[count++].m_f4Color = DEFINE::COLOR_GREEN;
	vertices[count].m_f3Pos.x = 0.f; vertices[count].m_f3Pos.y = 0.f; vertices[count].m_f3Pos.z = 0.f; vertices[count++].m_f4Color = DEFINE::COLOR_GREEN;
	vertices[count].m_f3Pos.x = 0.f; vertices[count].m_f3Pos.y = 0.f; vertices[count].m_f3Pos.z = 0.f; vertices[count++].m_f4Color = DEFINE::COLOR_BLUE;
	vertices[count].m_f3Pos.x = 0.f; vertices[count].m_f3Pos.y = 0.f; vertices[count].m_f3Pos.z = 1.f; vertices[count++].m_f4Color = DEFINE::COLOR_BLUE;

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

/////////////////////////////////////////////////////////////////////////////////////////
GizmoManager::GizmoManager()
{
	m_bSelectGiszmo = false;
	m_vecGizmo.push_back(std::shared_ptr<Gizmo>(new TransGizmo()));
	m_vecGizmo.push_back(std::shared_ptr<Gizmo>(new RotationGizmo()));
	m_vecGizmo.push_back(std::shared_ptr<Gizmo>(new ScalingGizmo()));
}

GizmoManager::~GizmoManager()
{

}

void GizmoManager::SetSelection(DEFINE::CHANGE_TYPE eChangeType, bool bSelection)
{
	m_bSelectGiszmo = bSelection;
}

void GizmoManager::AnimateObjects(DEFINE::CHANGE_TYPE eChangeType, float fTimeElapsed)
{
	if (!m_bSelectGiszmo)
		return;

	m_vecGizmo[static_cast<int>(eChangeType)]->AnimateObjects(fTimeElapsed);
}

void GizmoManager::Render(DEFINE::CHANGE_TYPE eChangeType)
{
	if (!m_bSelectGiszmo)
		return;

	m_vecGizmo[static_cast<int>(eChangeType)]->Render();
}