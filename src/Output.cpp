///////////////////////////////////////////////////
// Output.cpp -  Definitions for the CViewOutput, CContainOutput
//              and CDockOutput classes


#include "stdafx.h"
#include "Output.h"
#include "resource.h"

#include "TabbedMDIApp.h"
extern CTabbedMDIApp *theApp_ptr;

#pragma comment(lib,"Ws2_32.lib")

static void paser_connect_string(char *connect_str, CONNECTINFO *connect_info)
{
	char connect_str_cp[128];
	char *param_ptr;
	int i, flag;
	char dumpbuffer [12];
	char dbuffer [12];
	char tbuffer [12];
	
	memset(connect_info, 0, sizeof(CONNECTINFO));
	strcpy(connect_str_cp, connect_str);
	
	i=0;
	connect_str=connect_str_cp;
	param_ptr=connect_str;
	flag=1;
	while(flag)
	{
		if((*connect_str==':')||(*connect_str=='\0'))
		{
			if(*connect_str=='\0') flag=0;
			else *connect_str='\0';
			
			if(i==0)
			{
				strcpy(connect_info->com_ip_str, param_ptr);
				if((*param_ptr=='C')||(*param_ptr=='c'))
				{
					connect_info->mode=1;
					connect_info->baud_port=115200;
					connect_info->databits=8;
					connect_info->stopbits=1;
					connect_info->parity=0;
				}
				else if(*param_ptr!='\0')
				{
					connect_info->mode=2;
					connect_info->baud_port=65500;
				}
			}
			else if(i==1)
			{
				sscanf(param_ptr, "%d", &(connect_info->baud_port));
			}
			else if(i==2)
			{
				if(connect_info->mode==1)
				{
					sscanf(param_ptr, "%01d%01d%01d", &(connect_info->databits), &(connect_info->stopbits), &(connect_info->parity));
				}
				else
				{
					strcpy(connect_info->dump_file, param_ptr);
				}
			}
			else if(i==3)
			{
				if(connect_info->mode==1) strcpy(connect_info->dump_file, param_ptr);
			}
			i++;
			param_ptr=connect_str;
		}
		else if(*param_ptr=='\0')
		{
			param_ptr++;
		}
		connect_str++;
	}

	if(strcmp(connect_info->dump_file, "L")==0)
	{
		_strdate(dumpbuffer);
		dbuffer[0]=dumpbuffer[6];
		dbuffer[1]=dumpbuffer[7];
		dbuffer[2]=dumpbuffer[0];
		dbuffer[3]=dumpbuffer[1];
		dbuffer[4]=dumpbuffer[3];
		dbuffer[5]=dumpbuffer[4];
		dbuffer[6]='\0';
		
		_strtime(dumpbuffer);
		tbuffer[0]=dumpbuffer[0];
		tbuffer[1]=dumpbuffer[1];
		tbuffer[2]=dumpbuffer[3];
		tbuffer[3]=dumpbuffer[4];
		tbuffer[4]=dumpbuffer[6];
		tbuffer[5]=dumpbuffer[7];
		tbuffer[6]='\0';

		sprintf(connect_info->dump_file, "LOG_%s_%d_%s-%s.log", connect_info->com_ip_str, connect_info->baud_port, dbuffer, tbuffer);
	}
}


///////////////////////////////////////////////
// CViewOutput functions
CViewOutput::CViewOutput()
{
	display_mode=0;
	in_counts=0;
	out_counts=0;
	m_Socket.parent_ptr=this;
	m_Socket.ConnectStatus=0;
}

CViewOutput::~CViewOutput()
{
	m_Serial.Close();
}

void CViewOutput::OnAttach()
{
    m_Font.CreateFont(16, 0, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
                    CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_MODERN, _T("Courier New"));

    SetFont(m_Font, FALSE);
    //SetWindowText(_T("Read Only Output Window"));
}

void CViewOutput::PreCreate(CREATESTRUCT& cs)
{
    cs.style = ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | WS_CHILD | 
                WS_CLIPCHILDREN | WS_HSCROLL | WS_VISIBLE | WS_VSCROLL | ES_READONLY;

	char temp_string[128];
	sprintf(temp_string, "Terminal: %s\n", connect_str);
	SetWindowText(_T(temp_string));

	paser_connect_string(connect_str, &connect_info);
}

static char *mode_string[3]={"Char", "Hex", "Hex + Char"};

int CViewOutput::GetStatusString(char *status_string)
{
	int retval=0;
	char temp_string[128];
	
	if(status_string==NULL) status_string=temp_string;
	
	*status_string='\0';
	if(connect_info.mode==1)
	{
		if(!m_Serial.m_bOpened)
		{
			status_string+=sprintf(status_string, "DISCONNECT");
		}
		else
		{
			retval=1;
			status_string+=sprintf(status_string, "CONNECT   ");
		}

		sprintf(status_string, "     Rx: %-8d   Tx: %-8d   Mode: %s", in_counts, out_counts, mode_string[display_mode]);
	}
	else if(connect_info.mode==2)
	{
		if(!m_Socket.ConnectStatus)
		{
			status_string+=sprintf(status_string, "DISCONNECT");
		}
		else
		{
			retval=1;
			status_string+=sprintf(status_string, "CONNECT   ");
		}

		sprintf(status_string, "     Rx: %-8d   Tx: %-8d   Mode: %s", in_counts, out_counts, mode_string[display_mode]);
	}

	return retval;
}

void CViewOutput::OnEditCopy()
{
	SendMessage(WM_COPY, 0, 0);
}

void CViewOutput::AppendString(char *str)
{
	CHARRANGE cr;
	cr.cpMin = -1;
	cr.cpMax = -1;
	SendMessage(EM_EXSETSEL, 0, (LPARAM)&cr);
	SendMessage(EM_REPLACESEL, (WPARAM)FALSE, (LPARAM)str);
	SendMessage(WM_VSCROLL, MAKEWPARAM(SB_BOTTOM,0), 0);
}

void CViewOutput::DisplayData(char *data, int count)
{
	int i, len;
	char *hex_string_ptr;
	char hex_string_line[256];

	SaveLog(data, count);
	if(display_mode==0)
	{
		for(i=0;i<count;i++) if(data[i]=='\0') data[i]='.';
		data[count]='\0';
		AppendString(data);
	}
	else if((display_mode==1)||(display_mode==2))
	{
		do
		{
			hex_string_ptr=hex_string_line;
			len=32-rest_hex_count;
			if(count<len) len=count;
			if(len>0)
			{
				memcpy(&rest_hex_data[rest_hex_count], data, len);
				data+=len;
				count-=len;
				rest_hex_count+=len;
				rest_hex_data[rest_hex_count]='\0';
			}
			
			if((rest_hex_count<32)&&(data!=0)) break;
			
			for(i=0;i<rest_hex_count;i++)
			{
				hex_string_ptr+=sprintf(hex_string_ptr, "%02X ", rest_hex_data[i]);
				if(rest_hex_data[i]<' ') rest_hex_data[i]='.';
			}

			if(display_mode==2)
			{
				if(rest_hex_count<32)
				{
					len=3*(32-rest_hex_count);
					memset(hex_string_ptr, ' ', len);
					hex_string_ptr+=len;
				}
				sprintf(hex_string_ptr, ": %s\r\n", (char *)rest_hex_data);
			}
			else
			{
				sprintf(hex_string_ptr, "\r\n");
			}
			
			rest_hex_count=0;
			AppendString(hex_string_line);
		}while(count>0);
	}
}

LRESULT CViewOutput::WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_TIMER: 
			On1Timer(wParam, lParam);
		break;
		
		case WM_SETFOCUS:
		case WM_MOUSEACTIVATE:
			theApp_ptr->m_Frame.SendMessage(WM_CUSTOMIZE_STATUS, 0, 0);
		break;
		
		case WM_CUSTOMIZE_SEND:
			return OnComSend(wParam, lParam);
			
		case WM_CUSTOMIZE_DGB:
			AppendString((char *)lParam);
			return TRUE;
		
		case WM_CUSTOMIZE_OPEN:
			OnConnectOpen();
			return TRUE;
		
		case WM_CUSTOMIZE_CLOSE:
			OnConnectClose();
			return TRUE;
		
		case WM_CUSTOMIZE_DISMODE:
			if((int)wParam!=display_mode)
			{
				display_mode=(int)wParam;
				if(display_mode>2) display_mode=0;
				rest_hex_count=0;
				ClearOutputText();
				ClearTransCounts();
				//theApp_ptr->m_Frame.SendMessage(WM_CUSTOMIZE_STATUS, 0, 0);
			}
			return TRUE;
	}
	return WndProcDefault(uMsg, wParam, lParam);
}

void CViewOutput::OnConnectOpen()
{
	char comport[32];
	char debug_string[128];
	char *debug_string_ptr;

	if(connect_info.mode==1)
	{
		if(!m_Serial.m_bOpened)
		{
			sprintf(comport, "\\\\.\\%s", connect_info.com_ip_str);
			debug_string_ptr=debug_string;
			debug_string_ptr+=sprintf(debug_string_ptr, "Open %s:%d ", connect_info.com_ip_str, connect_info.baud_port);
			if(m_Serial.Open(comport, connect_info.baud_port, connect_info.databits, connect_info.stopbits, connect_info.parity)!=TRUE)
			{
				debug_string_ptr+=sprintf(debug_string_ptr, "FAILURE\r\n");
			}
			else
			{
				debug_string_ptr+=sprintf(debug_string_ptr, "SUCCESS %1d%1d%1d %s\r\n", (connect_info.databits), (connect_info.stopbits), (connect_info.parity), connect_info.dump_file);
				//debug_string_ptr+=sprintf(debug_string_ptr, "GetLimitText %d\r\n", GetLimitText()); //valure auto increased
				if(connect_info.baud_port>=38400) SetTimer(1000, 1, NULL);
				else if(connect_info.baud_port>=9600) SetTimer(1000, 10, NULL);
				else SetTimer(1000, 100, NULL);
			}
			AppendString(debug_string);
			SaveLog("--------------------\r\n", 22);
			SaveLog(debug_string, strlen(debug_string));
			SaveLog("--------------------\r\n", 22);
		}
	}
	else if(connect_info.mode==2)
	{
		if(m_Socket.GetSocket()==INVALID_SOCKET)
		{
			u_long mode = 1;
			
			m_Socket.Create(AF_INET, SOCK_STREAM, 0);
			m_Socket.ioCtlSocket(FIONBIO, &mode);
			
			#if 0
			m_Socket.StartEvents();
			debug_string_ptr=debug_string;
			debug_string_ptr+=sprintf(debug_string_ptr, "Open %s:%d ", connect_info.com_ip_str, connect_info.baud_port);
			if(m_Socket.Connect(connect_info.com_ip_str, connect_info.baud_port)!=0)
			{
				//m_Socket.Disconnect();
				//debug_string_ptr+=sprintf(debug_string_ptr, "FAILURE\r\n");
			}
			else
			{
				m_Socket.ConnectStatus=1;
				debug_string_ptr+=sprintf(debug_string_ptr, "SUCCESS %s\r\n", connect_info.dump_file);
				//m_Socket.StartEvents();
			}
			AppendString(debug_string);
			SaveLog("--------------------\r\n", 22);
			SaveLog(debug_string, strlen(debug_string));
			SaveLog("--------------------\r\n", 22);
			#endif
			m_Socket.Connect(connect_info.com_ip_str, connect_info.baud_port);
			AppendString("connecting...\r\n");
			
			SetTimer(1000, 1, NULL);
		}
	}

	theApp_ptr->m_Frame.SendMessage(WM_CUSTOMIZE_STATUS, 0, 0);
}

void CViewOutput::OnConnectClose()
{
	char temp_string[128];
	
	if(connect_info.mode==1)
	{
		if(m_Serial.m_bOpened)
		{
			KillTimer(1000);
			m_Serial.Close();
			sprintf(temp_string, "%s closed\r\n", connect_info.com_ip_str);
			AppendString(temp_string);
		}
	}
	else if(connect_info.mode==2)
	{
		if(m_Socket.GetSocket()!=INVALID_SOCKET)
		{
			KillTimer(1000);
			//if(m_Socket.ConnectStatus)
			{
				sprintf(temp_string, "%s closed\r\n", connect_info.com_ip_str);
				AppendString(temp_string);
			}
			m_Socket.ConnectStatus=0;
			//m_Socket.StopEvents();
			m_Socket.Disconnect();
		}
	}

	theApp_ptr->m_Frame.SendMessage(WM_CUSTOMIZE_STATUS, 0, 0);
}

void CViewOutput::ClearTransCounts()
{
	in_counts=0;
	out_counts=0;
	theApp_ptr->m_Frame.SendMessage(WM_CUSTOMIZE_STATUS, 0, 0);
}

void CViewOutput::ClearOutputText()
{
	SetWindowText(_T(""));
}

static unsigned char zcc_detect_data(char d)
{
	if((d<='9')&&(d>='0')) return d-'0';
	else if((d<='F')&&(d>='A')) return d-'A'+0x0A;
	else if((d<='f')&&(d>='a')) return d-'a'+0x0A;
	else if(d==' ') return 0xfe;
	else return 0xff;
}

static int bytes2hex(char *in, unsigned char *out, int c)
{
	int len=0, flag=0;
	unsigned char temp, temp1=0;
	
	while(c--)
	{
		temp=zcc_detect_data(*in);
		in++;
		if(temp==0xFF) return -1;
		if(temp==0xFE)
		{
			if(flag!=0)
			{
				flag=0;
				*out=temp1;
				out++;
				temp1=0;
				len++;
			}
		}
		else
		{
			temp1|=temp;
			if(flag==0)
			{
				flag=1;
				temp1<<=4;
			}
			else
			{
				flag=0;
				*out=temp1;
				out++;
				temp1=0;
				len++;
			}
		}
	}
	
	if(flag==1)
	{
		*out=temp1;
		out++;
		len++;
	}
	
	return len;
}

void CViewOutput::SaveLog(char *data, int len)
{
	if((connect_info.dump_file[0]==0)||(len==0)) return;
	
	FILE* fd = fopen(connect_info.dump_file, "ab+");
	if (fd != NULL)
	{
		fwrite(data, 1, len, fd);
		fclose(fd);
	}
}

LRESULT CViewOutput::OnComSend(WPARAM wParam, LPARAM lParam)
{
	int len=0, flag=0;
	char *old_data=(char *)lParam;
	char *new_data=NULL;
	char *new_data_ptr;
	char *temp_ptr=old_data;

	if(((connect_info.mode==1)&&(!m_Serial.m_bOpened))
		||((connect_info.mode==2)&&(!m_Socket.ConnectStatus)))
	{
		return TRUE;
	}
	
	if(wParam==0)
	{
		len=strlen((char *)lParam);
		temp_ptr=old_data;
	}
	else if(wParam==1)
	{
		new_data=(char *)malloc(strlen(old_data)+1);
		if(new_data==NULL) return TRUE;
		new_data_ptr=new_data;

		while(*old_data!='\0')
		{
			if(*old_data=='[')
			{
				temp_ptr=strstr(old_data, "]");
				if(temp_ptr!=NULL)
				{
					len=bytes2hex(old_data+1, (unsigned char *)new_data_ptr, temp_ptr-old_data-1);
					if(len<=0)
					{
						*new_data_ptr=*old_data;
						new_data_ptr++;
					}
					else
					{
						new_data_ptr+=len;
						old_data=temp_ptr;
					}
				}
				else
				{
					strcpy(new_data_ptr, old_data);
					new_data_ptr+=strlen(old_data);
					break;
				}
			}
			else
			{
				*new_data_ptr=*old_data;
				new_data_ptr++;
			}
			old_data++;
		}
		
		temp_ptr=new_data;
		len=new_data_ptr-new_data;
	}

	if(connect_info.mode==1)
	{
		m_Serial.SendData(temp_ptr, len);
	}
	else if(connect_info.mode==2)
	{
		m_Socket.Send(temp_ptr, len, 0);
	}
	if(new_data!=NULL) free(new_data);
	out_counts+=len;
	theApp_ptr->m_Frame.SendMessage(WM_CUSTOMIZE_STATUS, 0, 0);
	
	return TRUE;
}

BOOL CViewOutput::OnCommand(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);

	switch (LOWORD(wParam))
	{
		case IDM_EDIT_COPY:		OnEditCopy();	return TRUE;
	}

	// return FALSE for unhandled commands
	return FALSE;
}

void CViewOutput::OnSerialTimer()
{
	int retval, flag=0;
	char temp_recv[1024];
	
	if(connect_info.mode==1)
	{
		if(m_Serial.m_bOpened)
		{
			retval=m_Serial.ReadData(temp_recv, sizeof(temp_recv)-1);
			if(retval>0)
			{
				in_counts+=retval;
				DisplayData(temp_recv, retval);
				//temp_recv[retval]='\0';
				//AppendString(temp_recv);
				theApp_ptr->m_Frame.SendMessage(WM_CUSTOMIZE_STATUS, 0, 0);
			}
			else if(rest_hex_count>0)
			{
				DisplayData(0, 0);
			}
		}
	}
	else if(connect_info.mode==2)
	{
		if(m_Socket.GetSocket()==INVALID_SOCKET) return;

		if(m_Socket.ConnectStatus==0)
		{
			retval=m_Socket.Connect(connect_info.com_ip_str, connect_info.baud_port);
			int last_errno=WSAGetLastError();
			if(last_errno==WSAEISCONN)
			{
				m_Socket.ConnectStatus=1;
				
				sprintf(temp_recv, "Open %s:%d SUCCESS %s\r\n", connect_info.com_ip_str, connect_info.baud_port, connect_info.dump_file);
				
				AppendString(temp_recv);
				SaveLog("--------------------\r\n", 22);
				SaveLog(temp_recv, strlen(temp_recv));
				SaveLog("--------------------\r\n", 22);
			}
			else if((retval!=0)&&(last_errno!=WSAEINPROGRESS)&&(last_errno!=WSAEALREADY))
			{
				OnConnectClose();
				sprintf(temp_recv, "Open %s:%d FAILURE\r\n", connect_info.com_ip_str, connect_info.baud_port);
				
				AppendString(temp_recv);
				SaveLog("--------------------\r\n", 22);
				SaveLog(temp_recv, strlen(temp_recv));
				SaveLog("--------------------\r\n", 22);
			}
		}
		else
		{
			while((retval=m_Socket.Receive(temp_recv, sizeof(temp_recv)-1, 0))>0)
			{
				flag=1;
				in_counts+=retval;
				DisplayData(temp_recv, retval);
				//temp_recv[retval]='\0';
				//AppendString(temp_recv);
			}

			if((retval==0)||(WSAGetLastError()!=WSAEWOULDBLOCK))
			{
				OnConnectClose();
			}
			else if((flag==0)&&(rest_hex_count>0))
			{
				DisplayData(0, 0);
			}
			
			theApp_ptr->m_Frame.SendMessage(WM_CUSTOMIZE_STATUS, 0, 0);
		}
	}
}

LRESULT CViewOutput::On1Timer(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	
	switch (LOWORD(wParam))
	{
		case 1000:		OnSerialTimer();	return TRUE;
	}
	
	return FALSE;
}

#if 0
void CViewOutput::OnInitialUpdate()
{
	//m_Font.CreateFont(16, 0, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
	//	            CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_MODERN, _T("Courier New"));

	//SetFont(&m_Font, FALSE);
	//SetWindowText(_T("Read Only Output Window\n"));

	char temp_string[128];
	sprintf(temp_string, "Terminal: %s\n", connect_str);
	SetWindowText(_T(temp_string));

	paser_connect_string(connect_str, &connect_info);
}
#endif


//////////////////////////////////////////////
//  Definitions for the CContainOutput class
CContainOutput::CContainOutput() 
{
    SetView(m_ViewOutput); 
    SetDockCaption (_T("Output View - Docking container"));
    SetTabText(_T("Output"));
    SetTabIcon(IDI_TEXT);
} 


//////////////////////////////////////////////
//  Definitions for the CDockOutput class
CDockOutput::CDockOutput()
{
    // Set the view window to our edit control
    SetView(m_View);

    // Set the width of the splitter bar
    SetBarWidth(2);
}

