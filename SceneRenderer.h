#pragma once


#include "DXUT/DXUT.h"
#include "DXUT/DXUTmisc.h"


// Renders the sphere with the cylinder as a representation of a spring.
class SceneRenderer
{
public:
	SceneRenderer(ID3D10Device* basicRenderingDeviceDevice, float anchorPointCylinder[3]);
	~SceneRenderer(void);

	// Sets the new dimension on a window resize.
	void SetWindowDimension(float width, float height);
	// Renders the cylinder and the sphere.
	void Render(ID3D10Device* basicRenderingDeviceDevice);
	// Resets the position of the sphere.
	void SetPositionOfSphere(float spherePosition[3]);
	// Changes the position of the camera.
	void ChangeCameraPosition(float radius, float angle);

	// Gets a picking ray for the indicated x,y coordinates.
	void GetPickingRay(float x, float y, float origin[3], float direction[3]);

	// Gets the distance from the camera.
	float GetCameraDistanceOrigin() { return m_viewingDistance; }

private:
	// -------------------------------------------------------------
	// All the shader paremteres come here
	//-------------------------------------------------------------
	ID3D10Effect*                       m_pEffect;
	ID3D10Buffer*                       m_pVertexBufferSphere;
	ID3D10Buffer*                       m_pVertexBufferCylinder;
	ID3D10Buffer*                       m_pIndexBufferSphere;
	ID3D10Buffer*                       m_pIndexBufferCylinder;

	ID3D10InputLayout*                  m_pVertexLayout;
	ID3D10EffectTechnique*              m_pTechnique;
	ID3D10EffectMatrixVariable*         m_pViewVariable;
	ID3D10EffectMatrixVariable*         m_pProjectionVariable;
	ID3D10EffectMatrixVariable*         m_pObjectOrientationMatrix;
	ID3D10EffectVectorVariable*			m_pPositionOfLightVariable;

	D3DXMATRIX                          m_View;
	D3DXMATRIX                          m_Projection;
	D3DXVECTOR3							m_positionOfLight;
	D3DXVECTOR3							m_anchorPointOfCylinder;

	int									m_sphereIndices;
	int									m_cylinderIndices;

	// As a vertex format we use point and normal as we want to illuminate our scene.
	struct InternalVertexFormat
	{
		D3DVECTOR m_position;
		D3DVECTOR m_normal;
	};

	// The viewingdistance from the origin.
	float								m_viewingDistance;
	// The viewing angle we have fom the origin.
	float								m_viewingAngle;

	// The position of the sphere.
	D3DXVECTOR3							m_positionOfSphere;


	// The viewport width.
	float								m_viewPortWidth;
	float								m_viewPortHeight;


	// Updates the camera matrix and writes the resources to the shader.
	void UpdateCameraMatrix();

	// Creates the description of the sphere vertex structure.
	InternalVertexFormat* GenerateSphereVertexStructure(int rings, int slices, int& numOfVerticesGenerated);
	// Creates the index description for the sphere structure.
	DWORD* GenerateSphereIndexStructure(int rings, int slices, int& numOfIndicesGenerated);

	// Creates the vertex description for the cylinder structure.
	InternalVertexFormat* GenerateCylinderVertexStructure(int sectors, int& numOfVerticesGenerated);
	// Creates the index description for the cylinder structure.
	DWORD* GenerateCylinderIndexStructure(int sectors, int& numOfIndicesGenerated);

};

