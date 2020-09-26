#include "SceneRenderer.h"
#include <math.h>


// Creates the cube renderer with the device.
// Loads all the required assets here.
SceneRenderer::SceneRenderer(ID3D10Device* basicRenderingDeviceDevice, float anchorPointCylinder[3])
{

	// Safe the position of the cylinder.
	m_anchorPointOfCylinder.x = anchorPointCylinder[0];
	m_anchorPointOfCylinder.y = anchorPointCylinder[1];
	m_anchorPointOfCylinder.z = anchorPointCylinder[2];

	// Starting viewing parameteres.
	m_viewingDistance = 60.0f;
	m_viewingAngle = 0.0f;

	m_positionOfSphere = m_anchorPointOfCylinder - D3DXVECTOR3(0.0f, 6.0f, 0.0f);

	// Load and compile the effect.
	HRESULT hr = S_OK;
	hr = D3DX10CreateEffectFromFile( L"SimpleShader.fx", NULL, NULL, "fx_4_0", 0, 0, basicRenderingDeviceDevice, NULL,
                                         NULL, &m_pEffect, NULL, NULL );
    if( FAILED( hr ) )
    {
	
        MessageBox( NULL, L"The FX file cannot be located.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK );
    }

	m_pTechnique = m_pEffect->GetTechniqueByName( "Render" );
    m_pViewVariable = m_pEffect->GetVariableByName( "View" )->AsMatrix();
    m_pProjectionVariable = m_pEffect->GetVariableByName( "Projection" )->AsMatrix();
	m_pObjectOrientationMatrix = m_pEffect->GetVariableByName ("World")->AsMatrix();
	m_pPositionOfLightVariable = m_pEffect->GetVariableByName ("PositionOfLight")->AsVector();

	// Define the input layout
    D3D10_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D10_INPUT_PER_VERTEX_DATA, 0 },
    };
    UINT numElements = sizeof( layout ) / sizeof( layout[0] );

    // Create the input layout
    D3D10_PASS_DESC PassDesc;
    m_pTechnique->GetPassByIndex( 0 )->GetDesc( &PassDesc );
    basicRenderingDeviceDevice->CreateInputLayout( layout, numElements, PassDesc.pIAInputSignature,
                                             PassDesc.IAInputSignatureSize, &m_pVertexLayout );

    // Set the input layout
    basicRenderingDeviceDevice->IASetInputLayout( m_pVertexLayout );
	 // Set primitive topology
    basicRenderingDeviceDevice->IASetPrimitiveTopology( D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	// The light position is 10 units above the anvhor point.
	D3DVECTOR positionOfLight = m_anchorPointOfCylinder;
	positionOfLight.y += 10.0f;
	positionOfLight.x += 4.0f;
	// Set the position
	m_pPositionOfLightVariable->SetFloatVector((float*)&positionOfLight);

	// Veretex buffer declarations, this will be a standard static vertex buffer
	// that does not require CPU access later on.
	D3D10_BUFFER_DESC vertexBuffer;
    vertexBuffer.Usage = D3D10_USAGE_DEFAULT;
    vertexBuffer.BindFlags = D3D10_BIND_VERTEX_BUFFER;
	vertexBuffer.CPUAccessFlags = 0;
    vertexBuffer.MiscFlags = 0;
	int numOfVerts;
	D3D10_SUBRESOURCE_DATA rawData;
	InternalVertexFormat* verts;

	// We create the sphere vertex buffer first.
	verts = GenerateSphereVertexStructure(200,200, numOfVerts);
	rawData.pSysMem = verts;
	vertexBuffer.ByteWidth = sizeof( InternalVertexFormat ) * numOfVerts;
	basicRenderingDeviceDevice->CreateBuffer( &vertexBuffer, &rawData, &m_pVertexBufferSphere);
	delete [] verts;


	// Now we create the cylinder vertex buffer.
	verts = GenerateCylinderVertexStructure(200, numOfVerts);
	rawData.pSysMem = verts;
	vertexBuffer.ByteWidth = sizeof( InternalVertexFormat ) * numOfVerts;
	basicRenderingDeviceDevice->CreateBuffer( &vertexBuffer, &rawData, &m_pVertexBufferCylinder);
	delete [] verts;

	// Prepare for index buffer generation.
	DWORD* indices;
	D3D10_BUFFER_DESC indexBuffer;
	indexBuffer.Usage = D3D10_USAGE_DEFAULT;
    indexBuffer.BindFlags = D3D10_BIND_INDEX_BUFFER;
	indexBuffer.CPUAccessFlags = 0;
    indexBuffer.MiscFlags = 0;


	// Generate index buffer for spheres.
	indices = GenerateSphereIndexStructure(200,200, m_sphereIndices);
	rawData.pSysMem = indices;
	indexBuffer.ByteWidth = sizeof(DWORD) * m_sphereIndices;
	basicRenderingDeviceDevice->CreateBuffer(&indexBuffer, &rawData, &m_pIndexBufferSphere);
	delete [] indices;

	// Generate the index buffer for the cylinder.
	indices = GenerateCylinderIndexStructure(200, m_cylinderIndices);
	rawData.pSysMem = indices;
	indexBuffer.ByteWidth = sizeof(DWORD) * m_cylinderIndices;
	basicRenderingDeviceDevice->CreateBuffer(&indexBuffer, &rawData, &m_pIndexBufferCylinder);
	delete [] indices;

	UpdateCameraMatrix();

}

/// Remove the allocated resources.
SceneRenderer::~SceneRenderer(void)
{
	SAFE_RELEASE(m_pVertexLayout);
	SAFE_RELEASE(m_pEffect);
	
	SAFE_RELEASE(m_pVertexBufferSphere);
	SAFE_RELEASE(m_pVertexBufferCylinder);

	SAFE_RELEASE(m_pIndexBufferSphere);
	SAFE_RELEASE(m_pIndexBufferCylinder);
}


// Sets the new aspect ratio on a window resize.
void SceneRenderer::SetWindowDimension(float width, float height)
{
	m_viewPortWidth = width;
	m_viewPortHeight = height;
	float ratio = width / height;

	D3DXMatrixPerspectiveFovLH( &m_Projection, D3DX_PI * 0.25f, ratio, 1.0f, 500.0f );
	m_pProjectionVariable->SetMatrix( ( float* )&m_Projection );
}


// Resets the position of the sphere.
void SceneRenderer::SetPositionOfSphere(float spherePosition[3])
{
	m_positionOfSphere.x = spherePosition[0];
	m_positionOfSphere.y = spherePosition[1];
	m_positionOfSphere.z = spherePosition[2];
}


// Changes the position of the camera.
void SceneRenderer::ChangeCameraPosition(float radius, float angle)
{
	m_viewingDistance += radius;
	m_viewingAngle += angle;

	UpdateCameraMatrix();
}

// Gets a picking ray for the indicated x,y coordinates.
void SceneRenderer::GetPickingRay(float x, float y, float origin[3], float direction[3])
{
	D3DXVECTOR3 viewingRay;
	viewingRay.x = (((2.0f * x ) / m_viewPortWidth ) - 1.0f) / m_Projection._11;
	viewingRay.y = -(((2.0f * y ) / m_viewPortHeight ) - 1.0f) / m_Projection._22;
	viewingRay.z = 1.0f;

	D3DXMATRIX inverse;
	D3DXMatrixInverse( &inverse, NULL, &m_View );
	origin[0] = inverse._41;
	origin[1] = inverse._42;
	origin[2] = inverse._43;

	direction[0] = viewingRay.x * inverse._11 + viewingRay.y * inverse._21 + viewingRay.z * inverse._31;
	direction[1] = viewingRay.x * inverse._12 + viewingRay.y * inverse._22 + viewingRay.z * inverse._32;
	direction[2] = viewingRay.x * inverse._13 + viewingRay.y * inverse._23 + viewingRay.z * inverse._33;
}



// Renders the cylinder and the sphere.
void SceneRenderer::Render(ID3D10Device* basicRenderingDeviceDevice)
{
	
	// Position the sphere.
	D3DXMATRIX result;
	D3DXMatrixTranslation(&result, m_positionOfSphere.x, m_positionOfSphere.y, m_positionOfSphere.z);
	m_pObjectOrientationMatrix->SetMatrix((float*)&result);
	// Now we have to bind vertex and index buffer.
	UINT offset = 0;
	UINT stride = sizeof(InternalVertexFormat);
	basicRenderingDeviceDevice->IASetIndexBuffer(m_pIndexBufferSphere, DXGI_FORMAT_R32_UINT, 0);
	basicRenderingDeviceDevice->IASetVertexBuffers(0, 1, &m_pVertexBufferSphere, &stride, &offset);
	// Now we can draw.
	m_pTechnique->GetPassByIndex( 0 )->Apply(0);
	basicRenderingDeviceDevice->DrawIndexed(m_sphereIndices, 0, 0);

	// For rendering the cylinder we have to scale the cylinder and apply the appropriate orientation.
	D3DXVECTOR3 direction = m_positionOfSphere - m_anchorPointOfCylinder;
	float length = sqrtf(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);
	D3DXMATRIX transformation;

	// The y axis with scaling.
	transformation(1,0) = direction.x;
	transformation(1,1) = direction.y;
	transformation(1,2) = direction.z;
	transformation(1,3) = 0.0f;

	// Normalize now.
	direction /= length;

	// The translation.
	transformation(3,0) = m_anchorPointOfCylinder.x;
	transformation(3,1) = m_anchorPointOfCylinder.y;
	transformation(3,2) = m_anchorPointOfCylinder.z;
	transformation(3,3) = 1.0f;

	
	D3DXVECTOR3 right(direction.y, -direction.x, 0.0f);
	length = sqrtf(right.x * right.x + right.y * right.y);
	right /= length;

	// The x-Axis:
	transformation(0,0) = right.x;
	transformation(0,1) = right.y;
	transformation(0,2) = right.z;
	transformation(0,3) = 0.0f;

	D3DXVECTOR3 zAxis(right.y * direction.z - right.z * direction.y,  right.z * direction.x - right.x * direction.z,
		right.x * direction.y - right.y * direction.x);
	length = sqrtf(zAxis.x * zAxis.x + zAxis.y * zAxis.y + zAxis.z * zAxis.z);
	zAxis /= length;

	// The z-Axis:
	transformation(2,0) = zAxis.x;
	transformation(2,1) = zAxis.y;
	transformation(2,2) = zAxis.z;
	transformation(2,3) = 0.0f;
	
	// Now we can draw the cylinder.
	m_pObjectOrientationMatrix->SetMatrix((float*)&transformation);
	basicRenderingDeviceDevice->IASetIndexBuffer(m_pIndexBufferCylinder, DXGI_FORMAT_R32_UINT, 0);
	basicRenderingDeviceDevice->IASetVertexBuffers(0, 1, &m_pVertexBufferCylinder, &stride, &offset);
	m_pTechnique->GetPassByIndex( 0 )->Apply(0);
	basicRenderingDeviceDevice->DrawIndexed(m_cylinderIndices, 0, 0);
}


// Updates the camera matrix and writes the resources to the 
void SceneRenderer::UpdateCameraMatrix()
{
	// Initialize the view matrix
    D3DXVECTOR3 Eye( m_viewingDistance * cosf(m_viewingAngle), 0.0f, m_viewingDistance * sinf(m_viewingAngle) );
    D3DXVECTOR3 At( 0.0f, 0.0f, 0.0f );
    D3DXVECTOR3 Up( 0.0f, 1.0f, 0.0f );
    D3DXMatrixLookAtLH( &m_View, &Eye, &At, &Up );

    // Update Variables that never change
    m_pViewVariable->SetMatrix( ( float* )&m_View );
}



// Creates the description of the sphere vertex structure.
SceneRenderer::InternalVertexFormat* SceneRenderer::GenerateSphereVertexStructure(int rings, int slices, int& numOfVerticesGenerated)
{
	const float radius = 3.0f;

	numOfVerticesGenerated = rings * slices;
	SceneRenderer::InternalVertexFormat* result = new SceneRenderer::InternalVertexFormat[numOfVerticesGenerated];
	float deltaHorrizontal = 2.0f * D3DX_PI / (rings - 1);
	float deltaVertical = D3DX_PI / (slices - 1);
	float currentHorrizontal = 0.0f;
	float currentVertical = - 0.5f * D3DX_PI; 
	int count = 0;

	for(int ring = 0; ring < rings; ++ring)
	{
		currentHorrizontal = 0;
		float z = sinf(currentVertical);
		float baseLength = cosf(currentVertical);
		for(int slice = 0; slice < slices; ++slice)
		{
			float x = baseLength * cosf(currentHorrizontal);
			float y = baseLength * sinf(currentHorrizontal);
			result[count].m_normal = D3DXVECTOR3(x,y,z);
			result[count].m_position = D3DXVECTOR3(x * radius, y * radius, z * radius);
			++count;
			currentHorrizontal += deltaHorrizontal;
		}
		currentVertical += deltaVertical;
	}

	return result;
}
	

// Creates the index description for the sphere structure.
DWORD* SceneRenderer::GenerateSphereIndexStructure(int rings, int slices, int& numOfIndicesGenerated)
{
	numOfIndicesGenerated = 6 * (rings - 1) * (slices - 1);
	DWORD* result = new DWORD[numOfIndicesGenerated];
	int count = 0;

	for(int ring = 0; ring < rings - 1; ++ring)
	{
		for(int slice = 0; slice < slices - 1; ++slice)
		{
			result[count++] = slice + ring * slices;
			result[count++] = (slice + 1) + ring * slices;
			result[count++] = slice + (ring + 1) * slices;

			result[count++] = (slice + 1) + ring * slices;
			result[count++] = (slice + 1) + (ring + 1) * slices;
			result[count++] = slice + (ring + 1) * slices;
		}
	}

	return result;
}

// Creates the vertex description for the cylinder structure.
SceneRenderer::InternalVertexFormat* SceneRenderer::GenerateCylinderVertexStructure(int sectors, int& numOfVerticesGenerated)
{
	const float radius = 1.0f;

	numOfVerticesGenerated = sectors * 2;
	SceneRenderer::InternalVertexFormat* result = new SceneRenderer::InternalVertexFormat[numOfVerticesGenerated];
	float deltaAngle = 2.0f * D3DX_PI / (sectors - 1);
	float currentAngle = 0;

	int count = 0;

	for(int sector = 0; sector < sectors; ++sector)
	{
		result[count].m_normal=D3DXVECTOR3(cosf(currentAngle), 0.0f,  sinf(currentAngle) );
		result[count].m_position =D3DXVECTOR3(cosf(currentAngle) * radius , 0.0f, sinf(currentAngle) * radius);
		result[count + sectors] = result[count];
		result[count + sectors].m_position.y = 1.0f;
		++count;
		currentAngle += deltaAngle;
	}
	return result;

}

// Creates the index description for the cylinder structure.
DWORD* SceneRenderer::GenerateCylinderIndexStructure(int sectors, int& numOfIndicesGenerated)
{
	numOfIndicesGenerated = 6 * (sectors - 1);
	DWORD* result = new DWORD[numOfIndicesGenerated];
	int count = 0;

	for(int sector = 0; sector < sectors - 1; ++sector)
	{
		result[count++] = sector;
		result[count++] = sector + 1;
		result[count++] = sector + sectors;

		result[count++] = sector + 1;
		result[count++] = sector + 1 + sectors;
		result[count++] = sector + sectors;
	}

	return result;
}