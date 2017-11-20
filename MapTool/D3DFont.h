#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include <stdio.h>
#include <stdarg.h>		//va_list ����ϱ� ����

/*------------------------------------------------------
	��Ʈ ��ü ����
	D3DFont font;

	��Ʈ ��ü �ʱ�ȭ
	����ü , ����ü , ����ü , �ü�ü , ���ü
	font.SetFont( g_pd3dDevice , 8 , 16 , "����ü" );

	DrawText() �Լ� ���
	1. font.DrawText( 20 , 20 , "�ѱ����" , "D3DCOLOR_XRGB(0x00 , 0xff , 0xff) );
	2. font.DrawText( 20 , 40 , 123 , D3DCOLOR_XRGB( 0x00 , 0xf , 0xff) );
	3. font.DrawText( 20 , 60 , D3DCOLOR_XRGB( 0x00 , 0xff , 0xff ) , "����[%c] ����[%04d] �Ҽ�[%.03f]" , 'k' , 100 , 0,1234f );
	4. font.DrawText( 20 , 80 , 300 , 100 , "�߾���ġ" , DT_CENTER , D3DCOLOR_XRGB(0xff , 0x00 , 0x00) );

------------------------------------------------------*/
class D3DFont
{
public:
	D3DFont();
	~D3DFont();

	// ��Ʈ ��ü �ʱ�ȭ
	HRESULT SetFont( LPDIRECT3DDEVICE9 , LONG , LONG , char* );
	
	void DrawText( LONG , LONG , LONG , LONG , D3DCOLOR , UINT , char* );
	void DrawText( LONG , LONG , LONG , LONG , D3DCOLOR , UINT , long );
	void DrawTextEx( LONG , LONG , LONG , LONG , D3DCOLOR , UINT , char* , ... );
private:
	LPD3DXFONT	m_pD3DFont;
};

