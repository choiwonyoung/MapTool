#include "stdafx.h"
#include "Direct3D.h"


Direct3D::Direct3D()
	: m_pD3D(NULL)
	, m_pDevice(NULL)
	, m_pDevice_Preview(NULL)
	, m_pTex(NULL)
{
	ZeroMemory(m_pPreview , sizeof(m_pPreview));
}


Direct3D::~Direct3D()
{
	SAFE_RELEASE( m_pTex );
	SAFE_RELEASE( m_pDevice_Preview );
	SAFE_RELEASE( m_pDevice );
	SAFE_RELEASE( m_pD3D );
}

/*
	DirectX �� �ʱ�ȭ �ϴ� ����
	1. D3D ����
	2. Present Parameter ����
	3. D3Device ����
	4. ���� ���� ����
*/
HRESULT Direct3D::Init( HWND hWnd , int width , int height )
{
	if( NULL == ( m_pD3D = Direct3DCreate9( D3D_SDK_VERSION ) ) )
		return E_FAIL;

	// ��ġ�� ���� ���� ������ �����´�.
	D3DCAPS9 caps;
	m_pD3D->GetDeviceCaps( 
		D3DADAPTER_DEFAULT ,		// ���÷��� ����͸� ��Ÿ���� ���� , D3DADAPTER_DEFAULT is always the primary display adapter.
		D3DDEVTYPE_HAL ,			// D3DDEVTYPE ������ ��� , ��ġ�� ������ ��Ÿ����.
		&caps						// ��ġ�� ��� ���� ����
		);
	
	int vp = 0;
	if( caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT )		// HardWare�� ��ȯ �� �������� �����Ǵ��� �˻�
		vp = D3DCREATE_HARDWARE_VERTEXPROCESSING;
	else
		vp = D3DCREATE_SOFTWARE_VERTEXPROCESSING;

	// ���� DisplayMode �� �����´�.
	D3DDISPLAYMODE d3ddm;
	if( FAILED( m_pD3D->GetAdapterDisplayMode( D3DADAPTER_DEFAULT , &d3ddm ) ) )
		return E_FAIL;

	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory( &d3dpp , sizeof( d3dpp ) );

	d3dpp.BackBufferCount		= 1;
	d3dpp.Windowed				= TRUE;
	/*
		Present�� 2���� ������� ����
		1. ������� ������ �������� ������ �ִ� ���
		2. ������� ������ ����� �� �ִ� ���
		D3DSWAPEFFECT_DISCARD �� Present �Ҷ� ������� ������ ����Ǿ ��� ���ٰ� �˷������μ� ���� ȿ�� ���� ������ �ø����� �����ϰ� ����

		�̰��� ���� ������ ���� ���۸� �ϴ��� ���������δ� ���� �������(0) ������ ���ο� ����(1)�� �����ϴ� �۾� ����

		D3DSWAPEFFCT_COPY/D3DSWAPEFFECT_FLIP
		Present�� 2������ ����
		1. ������� ������ ����Ʈ ���ۿ� ����(COPY)�ϴ� ���(����� ���� , ���� ����� ����)		- D3DSWAPEFFECT_COPY
		2. (����)�����+�ּҺ����� ����Ͽ� ����۰� ���ư��� ����Ʈ ���۰� �Ǵ� ���			- D3DSWAPEFFECT_FLIP

		FLIP�� �ϴ��� DISCARD�� ���� ������ ������� ������ �����Ǿ�� �ϱ⶧���� ���縦 �ϰ� �˴ϴ�.

	*/
	d3dpp.SwapEffect			= D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferWidth		= width;
	d3dpp.BackBufferHeight		= height;
	d3dpp.BackBufferFormat		= d3ddm.Format;
	d3dpp.hDeviceWindow			= hWnd;

	/*
		EnableAutoDepthStencil �� TRUE �̸� ���� ���۸� �����ϵ��� ����
		AutoDepthStencilFormat�� ��ȿ�� ���� ���� �������� �����ϴ� �� D3DFMT_16�� 16��Ʈ ���� ���۸� ����� �� �ִ� ������ ����

	*/
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;					// 24 bit Depth / 8 Bit Stencil

	/*
		D3DPRESENT_INTERVAL_DEFAULT : DirectX �� ������ ���� �����Ͽ� ������ ������ ����
		D3DPRESENT_INTERVAL_IMMEDIATE : �׻� ������ �Ѵ�.
		D3DPRESENT_INTERVAL_ONE : ����� ������ 1�� �̷������ �ð��� ������ �Ѵ�.
		....
	*/
	d3dpp.PresentationInterval	 = D3DPRESENT_INTERVAL_IMMEDIATE;	// Adapter�� Refresh �ϴ� ���� ��� Update�ϵ��� ����

	// Device ����
	if( FAILED( m_pD3D->CreateDevice(
		D3DADAPTER_DEFAULT ,	// ���÷��� ����� , D3DADAPTER_DEFAULT�� �׻� 1�� ���÷��� �����
		D3DDEVTYPE_HAL ,		// ������ ��ġ Ÿ�� (�ϵ���� Ÿ��)
		hWnd ,					// ���� ���� ������ �ڵ�
		vp ,					// ��ġ ������ �����ϴ� 1�� �Ǵ� ���� ���� �ɼ�����
		&d3dpp ,				// D3DPRESENT_PARAMETERS ����
		&m_pDevice				//������ ����̽�
		) ) )
	{
		return E_FAIL;
	}

	m_pDevice->SetRenderState( D3DRS_ZENABLE , TRUE );
	m_pDevice->SetRenderState( D3DRS_CULLMODE , D3DCULL_CCW );		// ������ �޸� ���� ���� �÷��� , D3DCULL_CCW(Counter Clock Wise): �ð� �ݴ� �������� ������ ȸ����Ų ���� �޸����� �ν�
	m_pDevice->SetRenderState( D3DRS_LIGHTING , FALSE );

	_SetUpMatrix( width , height );
	_SetUpLight();
	SetUpFog( 900.0f , 950.0f );

	return S_OK;
}

void Direct3D::SetUpFog( float fStart , float fEnd, float density /*= 0.0f */ )
{

}

void Direct3D::_SetUpMatrix( int width , int height )
{
	D3DXMATRIX matWorld;
	D3DXMatrixIdentity( &matWorld );
	m_pDevice->SetTransform( D3DTS_WORLD , &matWorld );
		
	D3DXMATRIX matProj;
	D3DXMatrixPerspectiveFovLH( 
		&matWorld ,					
		D3DX_PI / 4.0f ,				// y ������ �þ�(���� ����)
		(float)width / (float)height ,  // ���� ���� ���� , �� ������ ���̸� ������ ���� ��
		1.0f ,							// ����� �� ����� Z ��
		2000.0f							// �� �� ����� Z ��
		);

	m_pDevice->SetTransform( D3DTS_PROJECTION , &matProj );
}


