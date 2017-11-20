#include "stdafx.h"
#include "ToolManager.h"
#include "Pick.h"
#include "Terrain.h"

ToolManager::ToolManager()
{
}


ToolManager::~ToolManager()
{
	SAFE_DELETE( m_pDirect3D );
}

HRESULT ToolManager::Init( HWND hWnd , HINSTANCE hInst , int width , int height )
{
	m_pDirect3D = new Direct3D;
	m_nWidth	= width;
	m_nHeight	= height;

	if( FAILED( m_pDirect3D->Init( hWnd , width , height ) ) )
		return E_FAIL;

	m_pDevice = m_pDirect3D->GetDevice();
	g_pPick = new Pick( m_pDevice , hWnd );
	m_pFont = new D3DFont;
	m_pFont->SetFont( m_pDevice , 10 , 20 , "Arial bold" );

	return S_OK;
}

void ToolManager::CameraSetting()
{
	float center = static_cast<float>( g_pTerrain->m_MapInfo.m_nTotalMapSize / 2.0f );
	D3DXVECTOR3 eye( center , 500.0f , -center - 300.0f );
	D3DXVECTOR3 look( center , 0.0f , -center );
	D3DXVECTOR3 up( 0.0f , 1.0f , 0.0f );

	SAFE_DELETE( m_pCamera );
	m_pCamera = new Camera( eye , look , up , m_nWidth , m_nHeight );
}

void ToolManager::CreateMap()
{
	SAFE_DELETE( m_pQuadTree );

	m_pQuadTree = new QuadTree( g_pTerrain->m_MapInfo.m_nTiles + 1 );
	m_pQuadTree->SetupQuadTree( g_pTerrain );
}

void ToolManager::DeleteMap()
{
	SAFE_DELETE( g_pTerrain );
	SAFE_DELETE( m_pQuadTree );

	g_pTerrain = new Terrain( m_pDevice );
}

void ToolManager::InitTexturePreview()
{
	if( m_pDirect3D )
		m_pDirect3D->InitPreview( m_hWnd_Preview , m_nWidth_Preview , m_nHeight_Preview );
}
