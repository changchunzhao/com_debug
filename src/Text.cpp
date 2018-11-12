///////////////////////////////////////////////////
// Text.cpp -  Definitions for the CViewText, 
//              and CDockText classes

#include "stdafx.h"
#include "Text.h"
#include "resource.h"

#include "TabbedMDIApp.h"
extern CTabbedMDIApp *theApp_ptr;


///////////////////////////////////////////////
// CViewText functions
CViewText::CViewText()
{
	readonly=1;
	pre_flag_y=-1;
	pre_flag_color=0;
}

CViewText::~CViewText()
{
	if(strcmp(file_name, "common.psc")!=0) theApp_ptr->m_Frame.SendMessage(WM_CUSTOMIZE_PSCCLOSE, 0, 0);
}

void CViewText::OnAttach()
{
    m_Font.CreateFont(16, 0, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_MODERN, _T("Courier New"));

    SendMessage(WM_SETFONT, reinterpret_cast<WPARAM>(m_Font.GetHandle()), 0);

    //SetWindowText(_T("Text Edit Window\r\n\r\n You can type some text here ..."));
}

BOOL CViewText::OnCommand(WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    
    UINT nID = LOWORD(wParam);
    switch (nID)
    {
    case IDM_EDIT_COPY:     OnEditCopy();   return TRUE;
    case IDM_EDIT_PASTE:    OnEditPaste();  return TRUE;
    case IDM_EDIT_CUT:      OnEditCut();    return TRUE;
    case IDM_EDIT_DELETE:   OnEditDelete(); return TRUE;
    case IDM_EDIT_REDO:     OnEditRedo();   return TRUE;
    case IDM_EDIT_UNDO:     OnEditUndo();   return TRUE;
    }

    // return FALSE for unhandled commands
    return FALSE;
}

void CViewText::OnEditCopy()
{
    SendMessage(WM_COPY, 0, 0);
}

void CViewText::OnEditPaste()
{
    SendMessage(EM_PASTESPECIAL, CF_TEXT, 0);
}

void CViewText::OnEditCut()
{
    SendMessage(WM_CUT, 0, 0);
}

void CViewText::OnEditDelete()
{
    SendMessage(WM_CLEAR, 0, 0);
}

void CViewText::OnEditRedo()
{
    SendMessage(EM_REDO, 0, 0);
}

void CViewText::OnEditUndo()
{
    SendMessage(EM_UNDO, 0, 0);
}

void CViewText::PreCreate(CREATESTRUCT& cs)
{
    cs.style = ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | WS_CHILD | 
                WS_CLIPCHILDREN | WS_HSCROLL | WS_VISIBLE | WS_VSCROLL;
}

void CViewText::OnSave()
{
	if((file_name[0]!='\0')&&(SendMessage(EM_GETMODIFY, 0, 0)))
	{
		FILE* fd = fopen(file_name, "wb+");
		if ( fd != NULL )
		{
			int len=GetWindowTextLength();
			if(len>0)
			{
				fwrite( GetWindowText().c_str(), len, 1, fd );
			}
			fclose( fd );
			SendMessage(EM_SETMODIFY, 0, 0);
		}
	}
	
	if(readonly==0)
	{
		clear_send_flag();
		readonly=1;
		SendMessage(EM_SETREADONLY, (WPARAM)readonly, 0);
	}
}

void CViewText::SetFileName(char *file_name_string)
{
	strcpy(file_name, file_name_string);
}

void CViewText::OnInitialUpdate()
{
	int len;
	CHARRANGE cr;
	cr.cpMin = -1;
	cr.cpMax = -1;
	char file_cache[1024];
	
	//m_Font.CreateFont(16, 0, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
	//	            CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_MODERN, _T("Courier New"));

	//SendMessage(WM_SETFONT, (WPARAM)m_Font.GetHandle(), 0);
	SendMessage(EM_SETTEXTMODE, TM_PLAINTEXT, 0L);
	
	readonly=0;
	if(file_name[0]!='\0')
	{
		FILE* fd = fopen(file_name, "rb");
		if ( fd != NULL )
		{
			SetWindowText(_T(""));
			while((len=fread( file_cache, 1, sizeof(file_cache), fd ))>0)
			{
				readonly=1;
				file_cache[len]='\0';
				SendMessage(EM_EXSETSEL, 0, (LPARAM)&cr);
				SendMessage(EM_REPLACESEL, (WPARAM)FALSE, (LPARAM)file_cache);
			}
			fclose( fd );
		}
	}

	SendMessage(WM_HSCROLL, MAKEWPARAM(SB_RIGHT, 0), 0);
	//SendMessage(WM_HSCROLL, MAKEWPARAM(SB_LEFT, 0), 0);
	SendMessage(EM_SETMARGINS, EC_LEFTMARGIN|EC_RIGHTMARGIN, MAKELONG(10, 0));
	SendMessage(EM_SETMODIFY, 0, 0);

	SendMessage(EM_SETREADONLY, (WPARAM)readonly, 0);
}

LRESULT CViewText::WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT retval;
	
	retval=WndProcDefault(uMsg, wParam, lParam);
	
	switch (uMsg)
	{
		case WM_LBUTTONDBLCLK:
			//OnBUTTONDBLCLK(); 
			if(readonly==1)
			{
				OnBUTTONDBLCLK(); 
			}
		break;

		case WM_PAINT:
			FlagOnPain();
		break;
	}
	
	//return WndProcDefault(uMsg, wParam, lParam);
	return retval;
}

void CViewText::FlagOnPain(void)
{
	int nRed, nGreen, nBlue;
	int xLeft, xRight, yTop, yBottom;
	
	if(pre_flag_y==-1) return;
	
	xLeft=2;
	xRight=8;
	yTop=pre_flag_y+4;
	yBottom=yTop+9;
	nRed    = 0;
	nGreen  = 0;
	nBlue   = 0;
	if(pre_flag_color==0) nRed=255;
	else if(pre_flag_color==1) nGreen=255;
	else if(pre_flag_color==2) nBlue=255;

	CClientDC RectDC(*this);
	RectDC.CreateSolidBrush (RGB (nRed, nGreen, nBlue));
	//RectDC.CreatePen(PS_SOLID,2,RGB(255,255,255));
	RectDC.Rectangle(xLeft, yTop, xRight, yBottom);
	//UpdateWindow();
}

void CViewText::clear_send_flag(void)
{
	int nRed, nGreen, nBlue;
	int xLeft, xRight, yTop, yBottom;
	
	if(pre_flag_y==-1) return;
	
	xLeft=2;
	xRight=8;
	yTop=pre_flag_y+4;
	yBottom=yTop+9;
	nRed    = 255;
	nGreen  = 255;
	nBlue   = 255;

	CClientDC RectDC(*this);
	//RectDC.CreateSolidBrush (RGB (nRed, nGreen, nBlue));
	RectDC.CreatePen(PS_SOLID,2,RGB(255,255,255));
	RectDC.Rectangle(xLeft, yTop, xRight, yBottom);
}

void CViewText::set_send_flag(int line_first_char)
{
	int nRed, nGreen, nBlue;
	int xLeft, xRight, yTop, yBottom;
	int m_cxClientMax=100, m_cyClientMax=100;
	CPoint pt;
	
	SendMessage(EM_POSFROMCHAR, (WPARAM)&pt, (LPARAM)line_first_char);
	
	if(pre_flag_y!=pt.y) clear_send_flag();
	pre_flag_y=pt.y;
	pre_flag_color++;
	if(pre_flag_color>2) pre_flag_color=0;
	xLeft=2;
	xRight=8;
	yTop=pt.y+4;
	yBottom=yTop+9;
	nRed    = 0;
	nGreen  = 0;
	nBlue   = 0;
	if(pre_flag_color==0) nRed=255;
	else if(pre_flag_color==1) nGreen=255;
	else if(pre_flag_color==2) nBlue=255;

	CClientDC RectDC(*this);
	RectDC.CreateSolidBrush (RGB (nRed, nGreen, nBlue));
	//RectDC.CreatePen(PS_SOLID,2,RGB(nRed, nGreen, nBlue));

	int Left   = (xLeft < xRight) ? xLeft : xRight;
	int Top    = (yTop < yBottom) ? yTop  : yBottom;
	int Right  = (xLeft > xRight) ? xLeft : xRight;
	int Bottom = (yTop > yBottom) ? yTop  : yBottom;
	RectDC.Rectangle(Left, Top, Right, Bottom);
}

void CViewText::OnBUTTONDBLCLK()
{
	CTabbedMDI* pTabbedMDI = theApp_ptr->m_Frame.GetTabbedMDI();
	CHARRANGE cr;
	long line_index, line_first_char;
	int len;
	char test_string[1*1024];

	assert(IsWindow());

	SendMessage(EM_EXGETSEL, 0, (LPARAM)&cr);
	line_index=SendMessage(EM_EXLINEFROMCHAR, 0, (LPARAM)cr.cpMin);
	
	line_first_char=SendMessage(EM_LINEINDEX, line_index, 0);
	cr.cpMin=line_first_char;
	cr.cpMax=line_first_char;
	SendMessage(EM_EXSETSEL, 0, (LPARAM)&cr);

	len=SendMessage(EM_GETLINE, (WPARAM)line_index, (LPARAM)test_string);
	if((test_string[0]=='\r')||(test_string[0]=='\n')) return ;
	if((len>2)&&(test_string[0]=='#')&&(test_string[1]=='#')) return ;
	
	set_send_flag(line_first_char);
	
	if((len>2)&&(test_string[0]=='T')&&(test_string[1]==':'))
	{
		if(test_string[len-1]==0x0D) test_string[len-1]='\0';
		len=2;
		while(test_string[len]==' ') len++;
		theApp_ptr->m_Frame.SendMessage(WM_CUSTOMIZE_NEW, 0, (LPARAM)(test_string+len));
		pTabbedMDI->GetActiveMDIChild()->SendMessage(WM_CUSTOMIZE_OPEN, 0, 0);
	}
	else if((len>2)&&(test_string[0]=='F')&&(test_string[1]==':'))
	{
		if(test_string[len-1]==0x0D) test_string[len-1]='\0';
		len=2;
		while(test_string[len]==' ') len++;
		theApp_ptr->m_Frame.SendMessage(WM_CUSTOMIZE_PSC, 0, (LPARAM)(test_string+len));
	}
	else if((len>2)&&(test_string[0]=='M')&&(test_string[1]==':'))
	{
		len=strlen(test_string);
		if(test_string[len-1]==0x0D) test_string[len-1]='\0';
		len=2;
		while(test_string[len]==' ') len++;
		if(pTabbedMDI->GetActiveMDIChild())
		{
			pTabbedMDI->GetActiveMDIChild()->SendMessage(WM_CUSTOMIZE_SEND, 1, (LPARAM)(test_string+len));
		}
	}
	else if((len>2)&&(test_string[0]=='P')&&(test_string[1]==':'))
	{
		len=strlen(test_string);
		if(test_string[len-1]==0x0D) len--;
		test_string[len]='\r';
		test_string[len+1]='\n';
		test_string[len+2]='\0';
		len=2;
		while(test_string[len]==' ') len++;
		if(pTabbedMDI->GetActiveMDIChild())
		{
			pTabbedMDI->GetActiveMDIChild()->SendMessage(WM_CUSTOMIZE_SEND, 0, (LPARAM)(test_string+len));
		}
	}
	else if((len>2)&&(test_string[0]=='E')&&(test_string[1]==':'))
	{
		if(strstr(test_string, "edit"))
		{
			readonly=0;
			SendMessage(EM_SETREADONLY, (WPARAM)readonly, 0);
		}
	}
	else if((len>2)&&(test_string[0]=='D')&&(test_string[1]==':'))
	{
		if((strstr(test_string, "hex"))&&(strstr(test_string, "char")))
		{
			if(pTabbedMDI->GetActiveMDIChild())
			{
				pTabbedMDI->GetActiveMDIChild()->SendMessage(WM_CUSTOMIZE_DISMODE, 2, 0);
			}
		}
		else if(strstr(test_string, "hex"))
		{
			if(pTabbedMDI->GetActiveMDIChild())
			{
				pTabbedMDI->GetActiveMDIChild()->SendMessage(WM_CUSTOMIZE_DISMODE, 1, 0);
			}
		}
		else if(strstr(test_string, "char"))
		{
			if(pTabbedMDI->GetActiveMDIChild())
			{
				pTabbedMDI->GetActiveMDIChild()->SendMessage(WM_CUSTOMIZE_DISMODE, 0, 0);
			}
		}
	}
	else if((len>2)&&(test_string[0]=='C')&&(test_string[1]==':'))
	{
		if(strstr(test_string, "recv"))
		{
			if((len>=0)&&(pTabbedMDI->GetActiveMDIChild()))
			{
				theApp_ptr->m_Frame.SendMessage(WM_CUSTOMIZE_CLEAR, 0, 0);
			}
		}
		if(strstr(test_string, "send"))
		{
			if((len>=0)&&(pTabbedMDI->GetActiveMDIChild()))
			{
				theApp_ptr->m_Frame.SendMessage(WM_CUSTOMIZE_CLEAR, 1, 0);
			}
		}
		if(strstr(test_string, "count"))
		{
			if((len>=0)&&(pTabbedMDI->GetActiveMDIChild()))
			{
				theApp_ptr->m_Frame.SendMessage(WM_CUSTOMIZE_CLEAR, 2, 0);
			}
		}
		
	}
	else
	{
		len=strlen(test_string);
		if(test_string[len-1]==0x0D) len--;
		test_string[len]='\r';
		test_string[len+1]='\n';
		test_string[len+2]='\0';
		if(pTabbedMDI->GetActiveMDIChild())
		{
			pTabbedMDI->GetActiveMDIChild()->SendMessage(WM_CUSTOMIZE_SEND, 0, (LPARAM)test_string);
		}
	}
}


///////////////////////////////////////////////
// CContainText functions
CContainText::CContainText()
{    
    //SetDockCaption (_T("Text View - Docking container"));
    //SetTabText(_T("Text"));
    //SetTabIcon(IDI_TEXT);
    SetView(m_ViewText);
} 

void CContainText::set_CViewText_file_name(char *file_name)
{
	//strcpy(m_ViewText.file_name, file_name);
	m_ViewText.SetFileName(file_name);
	SetDockCaption (_T(m_ViewText.file_name));
	SetTabText(_T(m_ViewText.file_name));
}

char *CContainText::get_CViewText_file_name()
{
	return m_ViewText.file_name;
}

LRESULT CContainText::WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_CUSTOMIZE_SAVE:	
			m_ViewText.OnSave();
			return TRUE;
	}
	
	return WndProcDefault(uMsg, wParam, lParam);
}


//////////////////////////////////////////////
//  Definitions for the CDockText class
CDockText::CDockText()
{
    // Set the view window to our edit control
    SetView(m_View);

    // Set the width of the splitter bar
    SetBarWidth(2);
}

void CDockText::set_CContainText_file_name(char *file_name)
{
	m_View.set_CViewText_file_name(file_name);
}

