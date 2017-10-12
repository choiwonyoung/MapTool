#include "stdafx.h"
#include "Input.h"


Input::Input()
	: m_pDI(NULL)
	, m_pKeyboard(NULL)
	, m_pMouse(NULL)
{
	ZeroMemory( &m_mouseState , sizeof( m_mouseState ) );
	ZeroMemory( &m_KeyState , sizeof( m_KeyState ) );
}


Input::~Input()
{
	if( m_pDI )
	{
		if( m_pKeyboard )
		{
			m_pKeyboard->Unacquire();
			m_pKeyboard->Release();
			m_pKeyboard = NULL;
		}

		if( m_pMouse )
		{
			m_pMouse->Unacquire();
			m_pMouse->Release();
			m_pMouse = NULL;
		}

		m_pDI->Release();
		m_pDI = NULL;
	}
}

//-----------------------------------------------------------------------------
// Direct Input 초기화
//-----------------------------------------------------------------------------
bool Input::InitInput( HWND hWnd , HINSTANCE hInst )
{
	//-----------------------------------------------------------------------------
	// DirectInput 생성
	//-----------------------------------------------------------------------------
	if( FAILED( DirectInput8Create( 
		hInst , 
		DIRECTINPUT_VERSION , 
		IID_IDirectInput8 , 
		(void**)&m_pDI , 
		NULL 
		) ) )
		return false;

	//-----------------------------------------------------------------------------
	// keyboard 초기화
	//-----------------------------------------------------------------------------

	// keyboard 디바이스 생성
	if( FAILED( m_pDI->CreateDevice( GUID_SysKeyboard , &m_pKeyboard , NULL ) ) )
	{
		::MessageBox( NULL , L"Keyboard Device Error." , L"Error" , MB_OK );
		return false;
	}

	// keyboard 협력 수준 설정
	/*
		dwFlag : 장치에 관련짓는 협력 레벨을 나타냄.
		DISCL_BACKGROUND : 백그라운드 액세스권을 요구 , 백그라운드 엑세스권이 허가되면 할당된 윈도우가 액티브 윈도우가 아닌 경우에도 언제라도 장치 취득가능해짐
		DISCL_EXCLUSIVE : 배타 엑세스권을 요구 , 배타 액세스권이 주어지면 장치가 취득되고 있는 동안, 장치의 다른 인스턴스는 그장치의 배타 액세스권을 취득할 수 없음
						  다만 , 다른 어플리케이션이 베타 액세스권을 취득하고 있어도 , 장치의 비베타 엑세스권은 항상 인정됨
						  애플리케이션이 마우스 또는 키보드 장치를 베타 모드로 취득하고 있는 경우는 , WM_ENTERSIZEMOVE 메세지 및 WM_ENTERMENULOOP 메세지를 받아들였을 때에, 
						  항상 장치를 릴리즈 할 필요가 있음
		DISCL_FOREGROUND : 애플리케이션은 foreground 액세스권을 요구한다. 할 당되면 백그라운드로 이동 할 때 , 장치는 자동적으로 릴리즈
		DISCL_NONEXCLUSIVE : 비 배타 액세스권 요구 , 장치에 대한 액세스권은 같은 장치에 액세스 하고 있는 다른 애플리케이션을 방해하지 않음
		DISCL_NOWINKEY : 윈도우 키를 무효로 함, 이 플래그를 설정하면 , 사용자는 잘못해 어플리케이션을 종료하는 것이 없어짐

	*/
	if( FAILED( m_pKeyboard->SetCooperativeLevel( hWnd , DISCL_BACKGROUND | DISCL_NONEXCLUSIVE ) ) )
	{
		MessageBox( NULL , L"Keyboard SetCooperativeLevel Error" , L"Error" , MB_OK );
		return false;
	}

	// Keyboard 데이타 형식 설정
	/*
		lpdf :	c_dfDIKeyboard
				c_dfDIMouse
				c_dfDIMouse2
				c_dfDIJoystick
				c_dfDIJoystick2
	*/
	if( FAILED( m_pKeyboard->SetDataFormat( &c_dfDIKeyboard ) ) )
	{
		MessageBox( NULL , L"Keyboard SetDataFormat Error" , L"Error" , MB_OK );
		return false;
	}

	// Keyboard 장치 얻어오기
	if( FAILED( m_pKeyboard->Acquire() ) )
	{
		MessageBox( NULL , L"Keyboard Acquire Error" , L"Error" , MB_OK );
		return false;
	}

	//-----------------------------------------------------------------------------
	// Mouse 초기화
	//-----------------------------------------------------------------------------

	//마우스 디바이스 생성
	if( FAILED( m_pDI->CreateDevice( GUID_SysMouse , &m_pMouse , NULL ) ) )
	{
		MessageBox( NULL , L"Mouse Device Error" , L"Error" , MB_OK );
		return false;
	}

	// Mouse 협력 수준 설정
	if( FAILED( m_pMouse->SetCooperativeLevel( hWnd , DISCL_BACKGROUND | DISCL_NONEXCLUSIVE ) ) )
	{
		MessageBox( NULL , L"Mouse SetCooperativeLevel Error" , L"Error" , MB_OK );
		return false;
	}

	// Mouse 데이터 설정
	if( FAILED( m_pMouse->SetDataFormat( &c_dfDIMouse ) ) )
	{
		MessageBox( NULL , L"Mouse SetDataFormat Error" , L"Error" , MB_OK );
		return false;
	}

	if( FAILED( m_pMouse->Acquire() ) )
	{
		MessageBox( NULL , L"Mouse Acquire Error" , L"Error" , MB_OK );
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// keyboard , mouse 입력값 얻어오기
//-----------------------------------------------------------------------------
bool Input::Update()
{
	/*
		GetDeviceState 로 해당 입력장치의 데이타를 가져온다.
	*/
	if( FAILED( m_pKeyboard->GetDeviceState( sizeof( m_KeyState ) , (LPVOID)m_KeyState ) ) )
	{
		if( m_pKeyboard->Acquire() )
			return false;

		if( FAILED( m_pKeyboard->GetDeviceState( sizeof( m_KeyState ) , (LPVOID)m_KeyState ) ) )
			return false;
	}

	if( FAILED( m_pMouse->GetDeviceState( sizeof( DIMOUSESTATE ) , &m_mouseState ) ) )
	{
		if( FAILED(m_pMouse->Acquire() ) )
			return false;

		if( FAILED( m_pMouse->GetDeviceState( sizeof( DIMOUSESTATE ) , &m_mouseState ) ) )
			return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// 키보드 키 업 , 다운
//-----------------------------------------------------------------------------
bool Input::KeyDown( BYTE key )
{
	return ( m_KeyState[key] & 0x80 ) ? true : false;
}

bool Input::KeyUp( BYTE key )
{
	return ( m_KeyState[key] & 0x80 ) ? true : false;
}

//-----------------------------------------------------------------------------
// 마우스 버튼 업 , 다운 ,
// button : 0 : 좌측 , 1 : 우측
//-----------------------------------------------------------------------------
bool Input::ButtonDown( int button )
{
	return ( m_mouseState.rgbButtons[button] & 0x80 ) ? true : false;
}

bool Input::ButtonUp( int button )
{
	return ( m_mouseState.rgbButtons[button] & 0x80 ) ? true : false;
}

//-----------------------------------------------------------------------------
// 마우스가 움직일때 좌표
//-----------------------------------------------------------------------------
void Input::GetMouseMovement( int &dx , int &dy )
{
	dx = m_mouseState.lX;
	dy = m_mouseState.lY;
}

//-----------------------------------------------------------------------------
// 마우스 휠을 움직일때
//-----------------------------------------------------------------------------
int Input::GetMouseWheelMovement()
{
	return m_mouseState.lZ;
}


