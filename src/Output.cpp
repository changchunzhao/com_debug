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
	//char dumpbuffer [12];
	//char dbuffer [12];
	//char tbuffer [12];
	
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
					//strcpy(connect_info->dump_file, param_ptr);
				}
			}
			else if(i==3)
			{
				//if(connect_info->mode==1) strcpy(connect_info->dump_file, param_ptr);
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

	#if 0
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
	#endif
}


///////////////////////////////////////////////
// CViewOutput functions
CViewOutput::CViewOutput()
{
	display_mode=DISMODE_CHAR;
	in_counts=0;
	out_counts=0;
	recv_data_count=0;
	m_Socket.parent_ptr=this;
	m_Socket.ConnectStatus=0;
	recv_log_file[0]='\0';
	disp_log_file[0]='\0';
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

static char *mode_string[4]={"Char", "Hex", "Time", "Echo"};
static char *conn_string[2]={"DISCONNECT", "CONNECT   "};

int CViewOutput::GetStatusString(char *status_string)
{
	int retval=1, i, flag=0;
	char temp_string[128];
	
	if(status_string==NULL) status_string=temp_string;
	
	*status_string='\0';
	if(connect_info.mode==1)
	{
		if(!m_Serial.m_bOpened)
		{
			retval=0;
		}
	}
	else if(connect_info.mode==2)
	{
		if(!m_Socket.ConnectStatus)
		{
			retval=0;
		}
	}
	else retval=-1;

	if(retval!=-1)
	{
		status_string+=sprintf(status_string, "%s", conn_string[retval]);
		status_string+=sprintf(status_string, "     Rx: %-8d   Tx: %-8d   Mode: ", in_counts, out_counts);
		for(i=0;i<4;i++)
		{
			if(display_mode&(1<<i))
			{
				if(flag) status_string+=sprintf(status_string, " + ");
				status_string+=sprintf(status_string, "%s", mode_string[i]);
				flag=1;
			}
		}
	}
	else
	{
		retval=0;
	}

	return retval;
}

void CViewOutput::OnEditCopy()
{
	SendMessage(WM_COPY, 0, 0);
}

void CViewOutput::AppendString(char *str)
{
	if(disp_log_file[0]!=0)
	{
		FILE* fd = fopen(disp_log_file, "ab+");
		if (fd != NULL)
		{
			fwrite(str, 1, strlen(str), fd);
			fclose(fd);
		}
	}

	CHARRANGE cr;
	cr.cpMin = -1;
	cr.cpMax = -1;
	SendMessage(EM_EXSETSEL, 0, (LPARAM)&cr);
	SendMessage(EM_REPLACESEL, (WPARAM)FALSE, (LPARAM)str);
	SendMessage(WM_VSCROLL, MAKEWPARAM(SB_BOTTOM,0), 0);
}

static char *dirt_string[2]={"RECV", "SEND"};
void CViewOutput::DisplayData(char *data, int count, SYSTEMTIME *st, int if_send)
{
	int i, len;
	char *hex_string_ptr;
	unsigned char rest_hex_data[64];
	char hex_string_line[256];

	SaveLog(data, count);
	if((display_mode&0x07)==DISMODE_CHAR)
	{
		for(i=0;i<count;i++) if(data[i]=='\0') data[i]='.';
		data[count]='\0';
		AppendString(data);
	}
	else
	{
		if(display_mode&DISMODE_TIME)
		{
			hex_string_ptr=hex_string_line;
			hex_string_ptr+=sprintf(hex_string_ptr, "\r\n------ %s %d Bytes ", dirt_string[if_send], count);
			hex_string_ptr+=sprintf(hex_string_ptr, "%02d/%02d/%02d %02d:%02d:%02d.%03d ------\r\n",
				st->wYear, st->wMonth, st->wDay, st->wHour, st->wMinute, st->wSecond, st->wMilliseconds);
			AppendString(hex_string_line);
		}
		
		if(display_mode&DISMODE_HEX)
		{
			while(count>0)
			{
				hex_string_ptr=hex_string_line;
				len=32;
				if(count<len) len=count;
				
				memcpy(rest_hex_data, data, len);
				rest_hex_data[len]='\0';
				count-=len;
				data+=len;
				
				for(i=0;i<len;i++)
				{
					hex_string_ptr+=sprintf(hex_string_ptr, "%02X ", rest_hex_data[i]);
					if(rest_hex_data[i]<' ') rest_hex_data[i]='.';
				}

				if(display_mode&DISMODE_CHAR)
				{
					if(len<32)
					{
						len=3*(32-len);
						memset(hex_string_ptr, ' ', len);
						hex_string_ptr+=len;
					}
					sprintf(hex_string_ptr, ": %s\r\n", (char *)rest_hex_data);
				}
				else
				{
					sprintf(hex_string_ptr, "\r\n");
				}
				
				AppendString(hex_string_line);
			}
		}
		else
		{
			for(i=0;i<count;i++) if(data[i]=='\0') data[i]='.';
			data[count]='\0';
			AppendString(data);
		}
	}
}

void CViewOutput::SetLogFileName(int type, char *name_string)
{
	char *log_name_ptr;
	char *type_string;
	char temp_string[128];
	
	if(type==0)
	{
		log_name_ptr=recv_log_file;
		type_string="RECV";
	}
	else
	{
		log_name_ptr=disp_log_file;
		type_string="DISP";
	}

	if(log_name_ptr[0]=='\0')
	{
		strcpy(log_name_ptr, name_string);
		if(strcmp(log_name_ptr, "L")==0)
		{
			char dumpbuffer [12];
			char dbuffer [12];
			char tbuffer [12];
			
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

			sprintf(log_name_ptr, "%sLOG_%s_%d_%s-%s.log", type_string, connect_info.com_ip_str, connect_info.baud_port, dbuffer, tbuffer);
		}
	}
	sprintf(temp_string, "%s log file: %s\r\n", type_string, log_name_ptr);
	AppendString(temp_string);
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
				if(recv_data_count>0)
				{
					DisplayData(recv_data_temp, recv_data_count, &pre_st, 0);
					recv_data_count=0;
				}
				display_mode=(int)wParam;
				ClearOutputText();
				ClearTransCounts();
				//theApp_ptr->m_Frame.SendMessage(WM_CUSTOMIZE_STATUS, 0, 0);
			}
			return TRUE;
		
		case WM_CUSTOMIZE_LOG:
			SetLogFileName((int)wParam, (char *)lParam);
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
				debug_string_ptr+=sprintf(debug_string_ptr, "SUCCESS %1d%1d%1d\r\n", (connect_info.databits), (connect_info.stopbits), (connect_info.parity));
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
	if((recv_log_file[0]==0)||(len==0)) return;
	
	FILE* fd = fopen(recv_log_file, "ab+");
	if (fd != NULL)
	{
		fwrite(data, 1, len, fd);
		fclose(fd);
	}
}

extern "C" 
{
extern unsigned short edsn_CRC16(unsigned char *pu8Msg, int u32Len, unsigned short CRC16);
extern unsigned int crc32c(unsigned int crc, const unsigned char *data, int length);
}

LRESULT CViewOutput::OnComSend(WPARAM wParam, LPARAM lParam)
{
	int len=0, flag=0;
	char *old_data=(char *)lParam;
	char *new_data=NULL;
	char *new_data_ptr;
	char *temp_ptr=old_data;
	unsigned int crc;
	int start_pos;
	SYSTEMTIME st;

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
					if(memcmp(old_data+1, "CRC16", 5)==0)
					{
						crc=0xFFFFFFFF;
						start_pos=0;
						sscanf(old_data+1, "CRC16:%d:%x", start_pos, crc);
						len=new_data_ptr-new_data;
						if((start_pos>=len)||(start_pos<0))
						{
							*new_data_ptr++=0;
							*new_data_ptr++=0;
						}
						else
						{
							crc=edsn_CRC16((unsigned char *)(new_data+start_pos), len-start_pos, (unsigned short)crc);
							*new_data_ptr++=(crc>>8)&0xFF;
							*new_data_ptr++=(crc>>0)&0xFF;
						}
						old_data=temp_ptr;
					}
					else if(memcmp(old_data+1, "CRC32", 5)==0)
					{
						crc=0xFFFFFFFF;
						start_pos=0;
						sscanf(old_data+1, "CRC32:%d:%x", start_pos, crc);
						len=new_data_ptr-new_data;
						if((start_pos>=len)||(start_pos<0))
						{
							*new_data_ptr++=0;
							*new_data_ptr++=0;
							*new_data_ptr++=0;
							*new_data_ptr++=0;
						}
						else
						{
							crc=crc32c(crc, (unsigned char *)(new_data+start_pos), len-start_pos);
							*new_data_ptr++=(crc>>24)&0xFF;
							*new_data_ptr++=(crc>>16)&0xFF;
							*new_data_ptr++=(crc>>8)&0xFF;
							*new_data_ptr++=(crc>>0)&0xFF;
						}
						old_data=temp_ptr;
					}
					else
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

	GetLocalTime(&st);
	if(connect_info.mode==1)
	{
		m_Serial.SendData(temp_ptr, len);
		if(display_mode&DISMODE_ECHO) DisplayData(temp_ptr, len, &st, 1);
	}
	else if(connect_info.mode==2)
	{
		m_Socket.Send(temp_ptr, len, 0);
		if(display_mode&DISMODE_ECHO) DisplayData(temp_ptr, len, &st, 1);
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
	int retval;
	char temp_recv[1024];
	
	if(connect_info.mode==1)
	{
		if(m_Serial.m_bOpened)
		{
			retval=m_Serial.ReadData(&(recv_data_temp[recv_data_count]), sizeof(recv_data_temp)-recv_data_count-1);
			if(retval>0)
			{
				if(recv_data_count==0)
				{
					GetLocalTime(&pre_st);
				}
				recv_data_count+=retval;
				in_counts+=retval;
				if((display_mode&0x07)==DISMODE_CHAR)
				{
					DisplayData(recv_data_temp, recv_data_count, 0, 0);
					recv_data_count=0;
				}
				theApp_ptr->m_Frame.SendMessage(WM_CUSTOMIZE_STATUS, 0, 0);
			}
			else if(recv_data_count>0)
			{
				DisplayData(recv_data_temp, recv_data_count, &pre_st, 0);
				recv_data_count=0;
				//theApp_ptr->m_Frame.SendMessage(WM_CUSTOMIZE_STATUS, 0, 0);
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
				
				sprintf(temp_recv, "Open %s:%d SUCCESS\r\n", connect_info.com_ip_str, connect_info.baud_port);
				
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
			GetLocalTime(&pre_st);
			while((retval=m_Socket.Receive(recv_data_temp, sizeof(recv_data_temp)-1, 0))>0)
			{
				in_counts+=retval;
				DisplayData(recv_data_temp, retval, &pre_st, 0);
			}

			if((retval==0)||(WSAGetLastError()!=WSAEWOULDBLOCK))
			{
				OnConnectClose();
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

