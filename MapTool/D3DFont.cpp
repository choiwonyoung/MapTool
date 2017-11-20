#include "stdafx.h"
#include "D3DFont.h"


D3DFont::D3DFont()
	: m_pD3DFont(NULL)
{
}


D3DFont::~D3DFont()
{
	SAFE_RELEASE( m_pD3DFont );
}

HRESULT D3DFont::SetFont( LPDIRECT3DDEVICE9 pDevice , LONG width , LONG height , char* pFontName)
{
	D3DXFONT_DESC desc;
	ZeroMemory( &desc , sizeof( D3DXFONT_DESC ) );

	desc.Width				= width;
	desc.Height				= height;
	desc.Weight				= FW_NORMAL;
	desc.MipLevels			= 1;
	desc.Italic				= 0;
	desc.CharSet			= ANSI_CHARSET;
	desc.OutputPrecision	= OUT_DEFAULT_PRECIS;
	desc.Quality			= DEFAULT_QUALITY;
	desc.PitchAndFamily		= FF_DONTCARE;
	wcscpy_s( desc.FaceName , CString( pFontName ) );

	if( FAILED( D3DXCreateFontIndirect( pDevice , &desc , &m_pD3DFont ) ) )
		return E_FAIL;

	return S_OK;
}

void D3DFont::DrawText( LONG x, LONG y, LONG width, LONG height, D3DCOLOR color, UINT direction, char* pString )
{
	RECT rc;
	rc.left = x;
	rc.top = y;
	rc.right = width;
	rc.bottom = height;

	CString strTemp( pString );
	m_pD3DFont->DrawText( NULL , strTemp , -1 , &rc , direction , color );
}

void D3DFont::DrawText( LONG x, LONG y, LONG width, LONG height, D3DCOLOR color, UINT direction, long numeric )
{
	char string[128];

	sprintf_s( string , "%d" , numeric );
	DrawText( x , y , width , height , color , direction , string );
}

void D3DFont::DrawTextEx( LONG x, LONG y, LONG width, LONG height, D3DCOLOR color, UINT direction, char* pFormat, ... )
{
	char string[128] = { 0 , };
	va_list argPtr;

	va_start( argPtr , pFormat );
	vsprintf_s( string , pFormat , argPtr );
	va_end( argPtr );

	DrawText( x , y , width , height , color , direction , string );
}
