///////////////////////////////////////////////////////
// Output.h - Declaration of the CViewOutput, CContainOutput, 
//          and CDockOutput classes

#ifndef OUTPUT_H
#define OUTPUT_H


#include <richedit.h>
#include "serial.h"

typedef struct
{
	char com_ip_str[64];
	int baud_port;
	int databits;
	int stopbits;
	int parity;
	char dump_file[128];
	int mode;
}CONNECTINFO;


// Declaration of the CViewText class
class CViewOutput : public CRichEdit
{
public:
    CViewOutput();
    virtual ~CViewOutput();
	void OnEditCopy();
	void AppendString(char *str);
	LRESULT OnComSend(WPARAM,LPARAM);
	void OnSerialTimer();
	void OnConnectOpen();
	void OnConnectClose();
	int GetStatusString(char *status_string);
	void ClearTransCounts();
	void ClearOutputText();
	void DisplayData(char *data, int count, SYSTEMTIME *st, int if_send);
	char connect_str[128];
	char recv_data_temp[2048];
	int recv_data_count;
	SYSTEMTIME pre_st;

protected:
    virtual void OnAttach();
    virtual void PreCreate(CREATESTRUCT& cs); 
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//virtual void OnInitialUpdate(); 
	virtual LRESULT WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
	//virtual LRESULT OnTimer(WPARAM wParam, LPARAM lParam);
	LRESULT On1Timer(WPARAM wParam, LPARAM lParam);
	void SaveLog(char *data, int len);

private:
    CFont m_Font;
	
	CSerial m_Serial;
	CONNECTINFO connect_info;
	int in_counts;
	int out_counts;
	int display_mode;

	class VCCSocket : public CSocket
	{
	public:
		CViewOutput *parent_ptr;
		int ConnectStatus;
		
	protected:
		//virtual void OnDisconnect();
		//virtual void OnConnect();
		//virtual void OnReceive();
	};
	
private:
	VCCSocket m_Socket;
};

// Declaration of the CContainText class
class CContainOutput : public CDockContainer
{
public:
    CContainOutput();
    ~CContainOutput() {}

private:
    CViewOutput m_ViewOutput;
};

// Declaration of the CDockText class
class CDockOutput : public CDocker
{
public:
    CDockOutput();
    virtual ~CDockOutput() {}

private:
    CContainOutput m_View;

};


#endif // OUTPUT_H

