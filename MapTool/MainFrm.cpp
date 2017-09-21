
// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "MapTool.h"

#include "MainFrm.h"
#include "MapToolView.h"
#include "ControlView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP( CSplitterX , CSplitterWnd )
	ON_WM_NCHITTEST()
END_MESSAGE_MAP()


LRESULT CSplitterX::OnNcHitTest( CPoint point )
{
	// TODO: Add your message handler code here and/or call default

	return HTNOWHERE;		// 마우스로 분할 창 못 건드리게
}

CSplitterX::CSplitterX()
{

}

CSplitterX::~CSplitterX()
{

}

// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_WM_CREATE()
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}
	m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT));

	// TODO: Delete these three lines if you don't want the toolbar to be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);


	return 0;
}

BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT /*lpcs*/,
	CCreateContext* pContext)
{
	m_wndSplitter.CreateStatic( this , 1 , 2 );
	m_wndSplitter.CreateView( 0 , 0 , RUNTIME_CLASS( CMapToolView ) , CSize( 768 , 768 ), pContext );
	m_wndSplitter.CreateView( 0 , 1 , RUNTIME_CLASS( CControlView ) , CSize( 1024 - 768 , 768 ), pContext );

	return TRUE;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	cs.x = 100;
	cs.y = 100;
	cs.cx = 1024;
	cs.cy = 768;

	cs.style &= ~FWS_ADDTOTITLE;				// 타이틀을 수정할 수 있게 해줌
	LPCTSTR titleName = _T( "MapTool" );
	SetTitle( titleName );

	return TRUE;
}

// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}
#endif //_DEBUG


// CMainFrame message handlers


