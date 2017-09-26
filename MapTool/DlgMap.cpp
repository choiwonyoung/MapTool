// DlgTab1.cpp : implementation file
//

#include "stdafx.h"
#include "MapTool.h"
#include "DlgMap.h"
#include "afxdialogex.h"


// CDlgTab1 dialog

IMPLEMENT_DYNAMIC( CDlgMap , CDialog )

CDlgMap::CDlgMap( CWnd* pParent /*=NULL*/ )
: CDialog( CDlgMap::IDD , pParent )
, m_nHeightSize( 0 )
, m_nFrustumDistance( 0 )
, m_nLodLevelSize( 0 )
, m_bLODCheck( FALSE )
{
}

CDlgMap::~CDlgMap()
{
}

void CDlgMap::DoDataExchange( CDataExchange* pDX )
{
	CDialog::DoDataExchange( pDX );
	DDX_Control( pDX , IDC_SLIDER_HEIGHTSIZE , m_sliderHeightSize );
	DDX_Control( pDX , IDC_SLIDER_FRUSTUMDISTANCE , m_sliderFrustumDistance );
	DDX_Control( pDX , IDC_SLIDER_LODLEVELSIZE , m_sliderLodLevelSize );
	DDX_Text( pDX , IDC_STATIC_HEIGHTSIZE , m_nHeightSize );
	DDX_Text( pDX , IDC_STATIC_FRUSTUMDISTANCE , m_nFrustumDistance );
	DDX_Text( pDX , IDC_STATIC_LODLEVELSIZE , m_nLodLevelSize );
	DDX_Check( pDX , IDC_CHECK_LOD , m_bLODCheck );
}


BEGIN_MESSAGE_MAP( CDlgMap , CDialog )
	ON_BN_CLICKED( IDC_DOWN , &CDlgMap::OnBnClickedDown )
	ON_BN_CLICKED( IDC_UP , &CDlgMap::OnBnClickedUp )
	ON_BN_CLICKED( IDC_FLAT , &CDlgMap::OnBnClickedFlat )
	ON_BN_CLICKED( IDC_ORIGIN , &CDlgMap::OnBnClickedOrigin )
	ON_NOTIFY( NM_CUSTOMDRAW , IDC_SLIDER_HEIGHTSIZE , &CDlgMap::OnNMCustomdrawSliderHeightsize )
	ON_NOTIFY( NM_CUSTOMDRAW , IDC_SLIDER_FRUSTUMDISTANCE , &CDlgMap::OnNMCustomdrawSliderFrustumdistance )
	ON_BN_CLICKED( IDC_CHECK_LOD , &CDlgMap::OnBnClickedCheckLod )
	ON_NOTIFY( NM_CUSTOMDRAW , IDC_SLIDER_LODLEVELSIZE , &CDlgMap::OnNMCustomdrawSliderLodlevelsize )
END_MESSAGE_MAP()



// CDlgTab1 message handlers
BOOL CDlgMap::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_nUpDown = -1;

	m_sliderHeightSize.SetRange( 1 , 10 );
	m_sliderHeightSize.SetPos( m_nHeightSize );

	m_sliderFrustumDistance.SetRange( 1 , 40 );
	m_sliderFrustumDistance.SetPos( 20 );

	m_sliderLodLevelSize.EnableWindow( FALSE );
	m_sliderLodLevelSize.SetRange( 5 , 20 );
	m_sliderLodLevelSize.SetPos( 10 );
	m_nLodLevelSize = (int)m_sliderLodLevelSize.GetPos() * 100;

	UpdateData( FALSE );

	return TRUE;
}

BOOL CDlgMap::PreTranslateMessage( MSG* pMsg )
{
	return CDialog::PreTranslateMessage( pMsg );
}

void CDlgMap::OnBnClickedUp()
{
	// TODO: Add your control notification handler code here
}


void CDlgMap::OnBnClickedDown()
{
	// TODO: Add your control notification handler code here
}




void CDlgMap::OnBnClickedFlat()
{
	// TODO: Add your control notification handler code here
}


void CDlgMap::OnBnClickedOrigin()
{
	// TODO: Add your control notification handler code here
}


void CDlgMap::OnNMCustomdrawSliderHeightsize( NMHDR *pNMHDR , LRESULT *pResult )
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>( pNMHDR );
	// TODO: Add your control notification handler code here
	*pResult = 0;

	OutputDebugString( L"Slider\n" );
}


void CDlgMap::OnNMCustomdrawSliderFrustumdistance( NMHDR *pNMHDR , LRESULT *pResult )
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>( pNMHDR );
	// TODO: Add your control notification handler code here
	*pResult = 0;
}

//////////////////////////////////////////////////////////////////////////
// LOD üũ
//////////////////////////////////////////////////////////////////////////
void CDlgMap::OnBnClickedCheckLod()
{
	// TODO: Add your control notification handler code here
}


void CDlgMap::OnNMCustomdrawSliderLodlevelsize( NMHDR *pNMHDR , LRESULT *pResult )
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>( pNMHDR );
	// TODO: Add your control notification handler code here
	*pResult = 0;
}
