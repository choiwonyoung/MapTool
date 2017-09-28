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
	m_pDevice->SetRenderState( D3DRS_LIGHTING , FALSE );			// Light ���� �������� Light ������ ���д�.

	_SetUpMatrix( width , height );
	_SetUpLight();
	SetUpFog( 900.0f , 950.0f );

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
// ���� ������ ���̴��� ��ü�ɼ� �ִ�.
//////////////////////////////////////////////////////////////////////////
void Direct3D::SetUpFog( float fStart , float fEnd, float density /*= 0.0f */ )
{
	/*
		density ���� ������ , D3DFOG_EXP �� ��밡���ϰ�
		���ٸ� D3DFOG_LINEAR �� �����ؼ� ����ؾ� ��
	*/
	if( density )
	{
		m_pDevice->SetRenderState( D3DRS_FOGENABLE , TRUE );
		m_pDevice->SetRenderState( D3DRS_FOGCOLOR , D3DCOLOR_XRGB( 120 , 180 , 255 ) );
		m_pDevice->SetRenderState( D3DRS_FOGSTART , *(DWORD*)&fStart );
		m_pDevice->SetRenderState( D3DRS_FOGEND , *(LPDWORD)&fEnd );
		m_pDevice->SetRenderState( D3DRS_FOGDENSITY , *(LPDWORD)&density );
		m_pDevice->SetRenderState( D3DRS_FOGTABLEMODE , D3DFOG_EXP );
	}
	else
	{
		m_pDevice->SetRenderState( D3DRS_FOGENABLE , TRUE );
		m_pDevice->SetRenderState( D3DRS_FOGCOLOR , D3DCOLOR_XRGB( 120 , 180 , 255 ) );
		m_pDevice->SetRenderState( D3DRS_FOGSTART , *(LPDWORD)&fStart );
		m_pDevice->SetRenderState( D3DRS_FOGEND , *(LPDWORD)&fEnd );
		m_pDevice->SetRenderState( D3DRS_FOGVERTEXMODE , D3DFOG_LINEAR );
	}
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

void Direct3D::_SetUpLight()
{
	// Material ����
	D3DMATERIAL9 mtrl;
	ZeroMemory( &mtrl , sizeof( mtrl ) );
	mtrl.Ambient	= D3DXCOLOR( 1.0f , 1.0f , 1.0f , 1.0f );
	mtrl.Diffuse	= D3DXCOLOR( 1.0f , 1.0f , 1.0f , 1.0f );
	mtrl.Specular	= D3DXCOLOR( 1.0f , 1.0f , 1.0f , 1.0f );
	m_pDevice->SetMaterial( &mtrl );

	// Light ����
	D3DLIGHT9 light;
	ZeroMemory(&light , sizeof(light));
	light.Type = D3DLIGHT_DIRECTIONAL;
	light.Diffuse = D3DXCOLOR( 1.0f , 1.0f , 1.0f , 1.0f );

	D3DXVec3Normalize( (D3DXVECTOR3*)&light.Direction , &D3DXVECTOR3( 1.0f , -1.0f , 1.0f ) );
	m_pDevice->SetLight( 0 , &light );
	m_pDevice->LightEnable( 0 , TRUE );
	m_pDevice->SetRenderState( D3DRS_LIGHTING , TRUE );
	m_pDevice->SetRenderState( D3DRS_AMBIENT , 0x00bfbfbf );			// ȯ�汤�� �� ����
}


HRESULT Direct3D::InitPreview( HWND hWnd , int width , int height )
{
	D3DDISPLAYMODE d3ddm;
	if( FAILED( m_pD3D->GetAdapterDisplayMode( D3DADAPTER_DEFAULT , &d3ddm ) ) )
		return E_FAIL;

	D3DPRESENT_PARAMETERS d3dpm;
	ZeroMemory( &d3dpm , sizeof( d3dpm ) );

	d3dpm.BackBufferCount			= 1;
	d3dpm.Windowed					= TRUE;
	d3dpm.SwapEffect				= D3DSWAPEFFECT_DISCARD;
	d3dpm.BackBufferWidth			= width;
	d3dpm.BackBufferHeight			= height;
	d3dpm.BackBufferFormat			= d3ddm.Format;
	d3dpm.hDeviceWindow				= hWnd;
	d3dpm.EnableAutoDepthStencil	= TRUE;
	d3dpm.AutoDepthStencilFormat	= D3DFMT_D24S8;

	//
	if( FAILED(m_pD3D->CreateDevice(D3DADAPTER_DEFAULT , D3DDEVTYPE_HAL , hWnd , D3DCREATE_SOFTWARE_VERTEXPROCESSING , &d3dpm , &m_pDevice_Preview)))
		return E_FAIL;

	_InitPreviewVertex( width , height );

	/*
		�ؽ��Ĵ� ��ũ�� ��ǥ���� �ﰢ���� ��������.
		�ؽ��� : �ȼ��� ��Ʈ�����̸� ǥ��� ��������� �ﰢ���� ���� �� ����
		�ؽ��� ���� : �̹��� �����͸� �ﰢ���� ���� �� �ְ� �ϴ� ��� , ��ũ�� �����̽����� ������

		�Ϲ������� �ؽ����� �ﰢ���� ��ũ���� �ﰢ���� ũ��� ���δٸ�
		�ؽ����� �ﰢ���� ��ũ���� �ﰢ������ ���� ��� �ؽ����� �ﰢ���� Ȯ���Ͽ� ũ�⸦ ���߰�,
		�ؽ����� �ﰢ���� ��ũ���� �ﰢ������ ū ��� �ؽ����� �ﰢ���� ����Ͽ� ũ�⸦ ����
		�� �ΰ��� ��� ��� �󸶰��� �ְ� ������ �߻��ϴµ�, ���͸��� ���� �ְ������� ���̰� , �ε巴�� �̹����� ����� ���� ���� SetSampleState�� �̿� �� �� ����
	*/
	m_pDevice_Preview->SetSamplerState( 
		0 ,						// �ؽ��� ��������
		D3DSAMP_MINFILTER ,		// ���� Ÿ�� (��� ���� ���)
		D3DTEXF_LINEAR );		// ���͸� ���
	m_pDevice_Preview->SetSamplerState(
		0 , 
		D3DSAMP_MAGFILTER ,		// Ȯ�� ���� ���
		D3DTEXF_LINEAR 
		);
	m_pDevice_Preview->SetSamplerState( 
		0 , 
		D3DSAMP_MIPFILTER ,		// �Ӹ� ������ ��� ����
		D3DTEXF_LINEAR 
		);

	m_pDevice_Preview->SetRenderState( D3DRS_CULLMODE , D3DCULL_CCW );
	m_pDevice_Preview->SetRenderState( D3DRS_LIGHTING , FALSE );

	return S_OK;
}

HRESULT Direct3D::LoadPreviewTexture( const WCHAR* texName )
{
	SAFE_RELEASE( m_pTex );

	if( FAILED( D3DXCreateTextureFromFileEx(
		m_pDevice ,					// IDirective3DDevice9�� ���� ������
		texName ,					// �ؽ��� ���ϸ�
		D3DX_DEFAULT ,				// ���� 0�� ��� ���Ϸκ��� ��� (2�� �¼��̸� DEFAULT�� ������ ����� �� ������ ���� �׷��� ������ ���� �̹����� ���� ũ�⸦ �־��ָ� ���ϴ� ũ��� ����)
		D3DX_DEFAULT ,				// ���� 0�� ��� ���Ϸκ��� ��� (2�� �¼��̸� DEFAULT�� ������ ����� �� ������ ���� �׷��� ������ ���� �̹����� ���� ũ�⸦ �־��ָ� ���ϴ� ũ��� ����)
		D3DX_DEFAULT ,				// �Ӹʷ����� ���� 0�� ��� ������ �Ӹ� ü���� ����
		0 ,							// ��� ���� ���� �÷��׷� ����( D3DUSAGE_RENDERTARGET(ǥ���� ������ ���) , D3DUSAGE_DYNAMIC(ǥ���� ����ó��) )
		D3DFMT_UNKNOWN ,			// D3DFORMAT ������ (D3DFMT_UNKNOWN�� ������ ���Ͽ��� ������ , D3DFMT_FROM_FILE ��� ���Ͽ� �ִ� �״���� ������ ������ ���� )
		D3DPOOL_DEFAULT ,			// D3DPOOL ������
		D3DX_DEFAULT ,				// D3DX_FILTER�� �ϳ� �̻��� �̹����� ���͸� �ϴ� ����� �����ϴ� ��� , D3DX_DEFAULT�� �����ϴ� ���� D3DX_FILTER_TRIANGLE | D3DX_FILTER_DITHER �� �����ϴ� ������ ����
		D3DX_DEFAULT ,				// D3DX_FILTER�� �ϳ� �̻��� ������ ���� , D3DX_DEFAULT�� �����ϴ� ���� D3DX_FILTER_BOX�� �����ϴ� �Ͱ� ����
		0 ,							// D3DCOLOR ���� Ű ��
		NULL ,						// D3DXIMAGE_INFO�� ������ 
		NULL ,						// PALETTEENTRY�� �����ϴ� 256 �� �ȷ�Ʈ�� ��Ÿ���� ����
		&m_pTex						// ��ȯ�Ǵ� Texture ������ ��
		) ) )				
	{
		return E_FAIL;
	}
		

	return S_OK;
}

/*
	������ ����
	1. Device Clear
	2. BeginScene , EndScene ���̿� Rendering �ؾ� �Ұ��� ����
	3. Present -> Front buffer�� Back Buffer �� �ٲ���
*/
void Direct3D::RenderPreview()
{
	if( m_pDevice_Preview )
	{
		m_pDevice_Preview->Clear( 
			0L ,										// 2��° ������ pRect�� �迭�� �ִ� ���簢�� �� , pRect �� NULL�� �����ϴ� ���� , �� �Ķ���͸� 0 ���� ����
			NULL ,										// D3DRECT ����ü�� �迭�� ������, Ŭ���� �ϴ� ���簢���� ��� , NULL �� �����ϸ� , ����Ʈ ���簢�� ��ü�� Ŭ���� �ϰ� ��
			D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER ,		// Ŭ�����ϴ� ǥ���� ��Ÿ���� �÷��� , D3DCLEAR_STENCIL (���ٽ� ���� Ŭ���� , stencil �Ķ���� ������ ��) , D3DCLEAR_TARGET (������ Ÿ���� Ŭ���� , Color �Ķ���� ������ ��) , D3DCLEAR_ZBUFFER (���� ���� Ŭ���� , Z �Ķ���� ������ ��)
			0xffffffff ,								// ���� Ÿ�� ǥ���� Ŭ���� �ϴ� 32��Ʈ ARGB ����
			1.0f ,										// �� �ż��尡 ���� ���ۿ� �����ϴ� ���ο� z��(0.0~1.0) , 0�� �� ���� ����� �Ÿ� , 1.0�� ���� �� �Ÿ�
			0L											// �� ���ٽ� ������ ��Ʈ���� �����ϴ� ������
			);

		if( SUCCEEDED( m_pDevice_Preview->BeginScene() ) )
		{
			_DrawPreviewTexture();
			m_pDevice_Preview->EndScene();
		}

		/*
			��ġ�� �����ϴ� �� ������ ���� �߿��� , ���� ������ �������� ���������̼� �Ѵ�.
		*/
		m_pDevice_Preview->Present(
			NULL ,		// ���� ü���� D3DSWAPEFFECT_COPY�� ���� �ǰ� �ִ� ��츦 ������ , NULL�� �����ؾ� �ϴ� ���� ������ , NULL�� ���ۿ� ǥ�� ��ü�� ǥ��
			NULL ,		// ���� ü���� D3DSWAPEFFECT_COPY�� ���� �ǰ� �ִ� ��츦 ������ , NULL�� �����ؾ� �ϴ� ���� ������ , NULL�� Ŭ���̾�Ʈ ���� ��ü�� ä����
			NULL ,		// Ŭ���̾�Ʈ ������ �� ���������̼��� ������� �� �ٷ������ ������ ������ ������ , NULL �� ��� , D3DPRESENT_PARAMETERS �� hWndDeviceWindow ����� ����
			NULL		// ����ü���� D3DSWAPEFFECT_COPY�� �������� �ʴ� ��, NULL�̿��� �� , 
			);
	}

}
void Direct3D::ReleasePreview()
{
	SAFE_RELEASE( m_pTex );
	SAFE_RELEASE( m_pDevice_Preview );
}

void Direct3D::_InitPreviewVertex( int width , int height )
{
	::ZeroMemory( &m_pPreview , sizeof( m_pPreview ) );

	m_pPreview[0].p = D3DXVECTOR4( 0.0f			, 0.0f			, 0.5f , 1.0f );
	m_pPreview[1].p = D3DXVECTOR4( (float)width , 0.0f			, 0.5f , 1.0f );
	m_pPreview[2].p = D3DXVECTOR4( 0.0f			, (float)height , 0.5f , 1.0f );
	m_pPreview[3].p = D3DXVECTOR4( (float)width , (float)height , 0.5f , 1.0f );

	// uv ��ǥ
	m_pPreview[0].t.x = 0.0f;
	m_pPreview[0].t.y = 0.0f;
	m_pPreview[1].t.x = 1.0f;
	m_pPreview[1].t.y = 0.0f;
	m_pPreview[2].t.x = 0.0f;
	m_pPreview[2].t.y = 1.0f;
	m_pPreview[3].t.x = 1.0f;
	m_pPreview[3].t.y = 1.0f;
}

void Direct3D::_DrawPreviewTexture()
{
	// �ش� Device���� �ѷ����� �ؽ��ĵ鿡 ���� ������
	m_pDevice_Preview->SetTextureStageState( 0 , D3DTSS_COLOROP , D3DTOP_MODULATE );
	m_pDevice_Preview->SetTextureStageState( 0 , D3DTSS_COLORARG1 , D3DTA_TEXTURE );
	m_pDevice_Preview->SetTextureStageState( 0 , D3DTSS_COLORARG2 , D3DTA_DIFFUSE );
	m_pDevice_Preview->SetTextureStageState( 0 , D3DTSS_ALPHAOP , D3DTOP_MODULATE );
	m_pDevice_Preview->SetTextureStageState( 0 , D3DTSS_ALPHAARG1 , D3DTA_TEXTURE );
	m_pDevice_Preview->SetTextureStageState( 0 , D3DTSS_ALPHAARG2 , D3DTA_DIFFUSE );

	m_pDevice_Preview->SetTexture( 0 , m_pTex );
	m_pDevice_Preview->SetFVF( PREVIEWVERTEX::FVF );
	/*
	DrawPrimitive�� ���׷��̵� ���� , DrawPrimitive�� CreateVertexBuffer�� ���ؽ��� �����ؾ� �ϴµ�
	DrawPrimitiveUP�� ���ؽ� ���۸� �������� �ʰ� �ý��� �޸��� ���������� �������� �׸� �� �ִ�.
	������ , �ý��� �޸��� ������ GPU�� ����ϱ� ������ �ӵ��� ����
	DrawPrimitiveUP ���� UP �� User Point�� �ǹ���
	*/
	m_pDevice_Preview->DrawPrimitiveUP(
		D3DPT_TRIANGLESTRIP ,			// D3DPRIMITIVETYPE ������ ���
		2 ,								// ������ �ϴ� �⺻������ �� , D3DCAPS9 ����ü�� MaxPrimitiveCount ����� üũ�ؼ� �����ȴ�.
		m_pPreview ,					// ���� �������� ����� �޸� ������
		sizeof( PREVIEWVERTEX )
		);
}






