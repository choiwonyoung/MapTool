#pragma once




struct PREVIEWVERTEX
{
	D3DXVECTOR4 p;
	D3DXVECTOR3 t;

	/*
		D3DFVF_DIFFUSE : ���� ������ diffuse �� ������ �����Ѵ�.

		D3DFVF_NORMAL : ���� ������ ���� ���� ���͸� �����Ѵ�.

		D3DFVF_SPECULAR : ���� ������ specular �� ������ �����Ѵ�.

		D3DFVF_XYZ : ���� ������ ��ȯ���� �ʴ� ������ ��ġ ��ǥ�� �����Ѵ�.
		�� �÷��״� ���� D3DFVF_NORMAL �÷��׿� ���� ���Ǹ� D3DFVF_XYZRHW �� ���� ����� �� ����. (3D ��ǥ)

		D3DFVF_XYZRHW : ���� ������ ��ȯ�� ������ ��ġ ��ǥ�� �����Ѵ�. (2D �� Pixel ��ǥ)
						RHW ( reciprocal homogeneous w : ��ġ ��ǥ 'w�� ����) , 1/w 
						-> �̰��� ����ϴ� ���� : perspective ��ȯ ����� ��ġ�� ������ ��İ��� �츮�� ���ϴ� ��ġ��ǥ�� �ٲٱ� ���� w�� 4��° ������ ������ �ϴµ� , �̰��� ������ �̸� �����س������� rhw ��� ��Ⱑ ��
						������ ������ �̹� ó���� ������ �����ϹǷ�  , 2D�� �ٷ� ����ϱ⿡ ����
						
		D3DFVF_XYZW : ���� ������ ��ȯ �� Ŭ���ε� x,y,z,w�����͸� �����Ѵ�.
					  ProcessVetices�� Ŭ���۸� �⵿���� �ʰ� , �����͸� Ŭ����ǥ�� ����Ѵ�.
					  �� ������ ���α׷��� �� �� �ִ� ���� ������ ���ο����� ����� �������� 
					  �� ������ ���α׷��� �� �� �ִ� ���� ������ �������� �ۿ� ������� �ʴ´�.
	*/
	enum { FVF = D3DFVF_XYZRHW | D3DFVF_TEX1 };
};
class Direct3D
{
//////////////////////////////////////////////////////////////////////////
// �Լ�
//////////////////////////////////////////////////////////////////////////
public:
	Direct3D();
	~Direct3D();

	LPDIRECT3DDEVICE9	GetDevice() { return m_pDevice; }

	HRESULT				Init( HWND hWnd , int width , int height );
	void				SetUpFog( float , float , float density = 0.0f );

	// �ؽ��� �̸������ ���õ� �Լ�
	HRESULT				InitPreview( HWND hWnd , int width , int height );
	HRESULT				LoadPreviewTexture( const WCHAR* texName );
	void				ReleasePreview();
	void				RenderPreview();

private:
	void				_SetUpMatrix( int width , int height );
	void				_SetUpLight();

	void				_InitPreviewVertex( int width , int height );
	void				_DrawPreviewTexture();

//////////////////////////////////////////////////////////////////////////
// ����
//////////////////////////////////////////////////////////////////////////
public:
	LPDIRECT3DDEVICE9	m_pDevice_Preview;

private:
	LPDIRECT3D9			m_pD3D;
	LPDIRECT3DDEVICE9	m_pDevice;

	LPDIRECT3DTEXTURE9	m_pTex;
	PREVIEWVERTEX		m_pPreview[4];			// �̸� ���� ȭ���� ���ؼ� 


};

