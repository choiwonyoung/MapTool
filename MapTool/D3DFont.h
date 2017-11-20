#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include <stdio.h>
#include <stdarg.h>		//va_list 사용하기 위해

/*------------------------------------------------------
	폰트 객체 생성
	D3DFont font;

	폰트 객체 초기화
	굴림체 , 돋움체 , 바탕체 , 궁서체 , 고딕체
	font.SetFont( g_pd3dDevice , 8 , 16 , "바탕체" );

	DrawText() 함수 사용
	1. font.DrawText( 20 , 20 , "한글출력" , "D3DCOLOR_XRGB(0x00 , 0xff , 0xff) );
	2. font.DrawText( 20 , 40 , 123 , D3DCOLOR_XRGB( 0x00 , 0xf , 0xff) );
	3. font.DrawText( 20 , 60 , D3DCOLOR_XRGB( 0x00 , 0xff , 0xff ) , "문자[%c] 정수[%04d] 소수[%.03f]" , 'k' , 100 , 0,1234f );
	4. font.DrawText( 20 , 80 , 300 , 100 , "중앙위치" , DT_CENTER , D3DCOLOR_XRGB(0xff , 0x00 , 0x00) );

------------------------------------------------------*/
class D3DFont
{
public:
	D3DFont();
	~D3DFont();

	// 폰트 객체 초기화
	HRESULT SetFont( LPDIRECT3DDEVICE9 , LONG , LONG , char* );
	
	void DrawText( LONG , LONG , LONG , LONG , D3DCOLOR , UINT , char* );
	void DrawText( LONG , LONG , LONG , LONG , D3DCOLOR , UINT , long );
	void DrawTextEx( LONG , LONG , LONG , LONG , D3DCOLOR , UINT , char* , ... );
private:
	LPD3DXFONT	m_pD3DFont;
};

