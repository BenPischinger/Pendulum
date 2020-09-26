// -------------------------------------------------------------------------------------
// This is the main file for the circle test application
// -------------------------------------------------------------------------------------

#include "DXUT/DXUT.h"
#include "DXUT/DXUTmisc.h"
#include "resource.h"
#include "SceneRenderer.h"
#include "PendulumIntegrator.h"
#include <math.h>





//--------------------------------------------------------------------------------------
// Basic components 
//--------------------------------------------------------------------------------------
SceneRenderer* g_sceneRenderer = NULL;
PendulumIntegrator* g_integrator = NULL;


//------------------------------------
// button status
//------------------------------------
bool g_wasDown = false;
bool g_wasUp = false;
bool g_wasLeft = false;
bool g_wasRight = false;



//--------------------------------------------------------------------------------------
// Forward declarations 
//--------------------------------------------------------------------------------------

HRESULT CALLBACK OnD3D10CreateDevice( ID3D10Device* pd3dDevice, const DXGI_SURFACE_DESC* pBufferSurfaceDesc,
                                      void* pUserContext );
HRESULT CALLBACK OnD3D10ResizedSwapChain( ID3D10Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                                          const DXGI_SURFACE_DESC* pBufferSurfaceDesc, void* pUserContext );
void CALLBACK OnD3D10FrameRender( ID3D10Device* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext );
void CALLBACK OnD3D10DestroyDevice( void* pUserContext );

void CALLBACK OnKeyboard( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext );
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext );
void CALLBACK OnMouse( bool bLeftButtonDown, bool bRightButtonDown, bool bMiddleButtonDown, bool bSideButton1Down, bool bSideButton2Down, int nMouseWheelDelta, int xPos, int yPos, void* pUserContext );



//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
    // Set DXUT callbacks
    DXUTSetCallbackD3D10DeviceCreated( OnD3D10CreateDevice );
    DXUTSetCallbackD3D10SwapChainResized( OnD3D10ResizedSwapChain );
    DXUTSetCallbackD3D10DeviceDestroyed( OnD3D10DestroyDevice );
    DXUTSetCallbackD3D10FrameRender( OnD3D10FrameRender );
	DXUTSetCallbackMouse (OnMouse);
    DXUTSetCallbackKeyboard( OnKeyboard );
    DXUTSetCallbackFrameMove( OnFrameMove );

    DXUTInit( true, true, NULL ); // Parse the command line, show msgboxes on error, no extra command line params
    DXUTSetCursorSettings( true, true ); // Show the cursor and clip it when in full screen
    DXUTCreateWindow( L"Pendulum Test" );
    DXUTCreateDevice( true, 640, 640 );
    DXUTMainLoop(); // Enter into the DXUT render loop

    return DXUTGetExitCode();
}



//--------------------------------------------------------------------------------------
// Create all the required resources.
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D10CreateDevice( ID3D10Device* pd3dDevice, const DXGI_SURFACE_DESC* pBufferSurfaceDesc, void* pUserContext )
{
	float anchorPoint[3] = {0.0f, 10.0f, 0.0f};
	g_sceneRenderer = new SceneRenderer(pd3dDevice, anchorPoint);
	g_integrator = new PendulumIntegrator(anchorPoint);
	return S_OK;
}


//--------------------------------------------------------------------------------------
// Clean up stuff here.
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D10DestroyDevice( void* pUserContext )
{
	delete g_sceneRenderer;
	delete g_integrator;
}

//--------------------------------------------------------------------------------------
// Handle the window size change.
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D10ResizedSwapChain( ID3D10Device* pd3dDevice, IDXGISwapChain* pSwapChain, const DXGI_SURFACE_DESC* pBufferSurfaceDesc, void* pUserContext )
{
	// Setup the projection parameters again
	g_sceneRenderer->SetWindowDimension(static_cast<float>( pBufferSurfaceDesc->Width ), static_cast<float>( pBufferSurfaceDesc->Height ));
    return S_OK;
}


//--------------------------------------------------------------------------------------
// Render the scene using the D3D9 device
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D10FrameRender( ID3D10Device* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext )
{
     
	//
    // Clear the back buffer
    //
    float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f }; // red, green, blue, alpha
    ID3D10RenderTargetView* pRTV = DXUTGetD3D10RenderTargetView();
    pd3dDevice->ClearRenderTargetView( pRTV, ClearColor );

    //
    // Clear the depth stencil
    //
    ID3D10DepthStencilView* pDSV = DXUTGetD3D10DepthStencilView();
    pd3dDevice->ClearDepthStencilView( pDSV, D3D10_CLEAR_DEPTH, 1.0, 0 );

	g_sceneRenderer->Render(pd3dDevice);

	
}




//--------------------------------------------------------------------------------------
// Update the scene here.
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext )
{
	float distanceDelta = 0.0f;
	if (g_wasUp)
		distanceDelta = -0.4f;
	if (g_wasDown)
		distanceDelta = 0.4f;

	float angleDelta = 0.0f;
	if (g_wasLeft)
		angleDelta = 0.1f;
	if (g_wasRight)
		angleDelta = -0.1f;

	g_sceneRenderer->ChangeCameraPosition(distanceDelta, angleDelta);

	g_integrator->UpdateSimulation(fElapsedTime);
	float position[3];
	g_integrator->ObtainCurrentPosition(position);
	g_sceneRenderer->SetPositionOfSphere(position);
}


//--------------------------------------------------------------------------------------
// Mouse presses.
//--------------------------------------------------------------------------------------
void CALLBACK OnMouse( bool bLeftButtonDown, bool bRightButtonDown, bool bMiddleButtonDown, bool bSideButton1Down, bool bSideButton2Down, int nMouseWheelDelta, int xPos, int yPos, void* pUserContext )
{
	if (bLeftButtonDown)
	{
		float distance = g_sceneRenderer->GetCameraDistanceOrigin();
		float position[3];
		float direction[3];
		g_sceneRenderer->GetPickingRay((float)xPos, (float)yPos, position, direction);

		position[0] += direction[0] * distance;
		position[1] += direction[1] * distance;
		position[2] += direction[2] * distance;

		g_integrator->SetPendulumPosition(position);
		
		
	}
}



//--------------------------------------------------------------------------------------
// Handle key presses
//--------------------------------------------------------------------------------------
void CALLBACK OnKeyboard( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext )
{

	if (bKeyDown)
	{
		if (nChar == VK_UP )
			g_wasUp = true;
		if (nChar == VK_DOWN )
			g_wasDown = true;
		if (nChar == VK_LEFT )
			g_wasLeft = true;
		if (nChar == VK_RIGHT )
			g_wasRight = true;

	}
	else
	{
		if (nChar == VK_UP )
			g_wasUp = false;
		if (nChar == VK_DOWN )
			g_wasDown = false;
		if (nChar == VK_LEFT )
			g_wasLeft = false;
		if (nChar == VK_RIGHT )
			g_wasRight = false;
	}

	
}



