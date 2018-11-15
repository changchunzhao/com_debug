// Serial.h: interface for the CSerial class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SERIAL_H__FABA21F2_6F33_40BF_A682_6180473D4EF4__INCLUDED_)
#define AFX_SERIAL_H__FABA21F2_6F33_40BF_A682_6180473D4EF4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//class CSerial : public CObject  
class CSerial
{
public:
	CSerial();
	virtual ~CSerial();
	BOOL Open( int nPort, int nBaud );
	BOOL Open( const char *com_name, int nBaud );
	BOOL Open( const char *com_name, int nBaud, int databits, int stopbits, int parity );
	int InBufferCount( void );
	DWORD SendData( const char *buffer, DWORD dwBytesWritten);
	DWORD ReadData( void *buffer, DWORD dwBytesRead);
	BOOL Close(  );
public:
	BOOL  m_bOpened;
	HANDLE m_hComDev;
	HANDLE m_hIDComDev;
	OVERLAPPED m_OverlappedRead;
	OVERLAPPED m_OverlappedWrite;
	int open_mode;
};
#endif // !defined(AFX_SERIAL_H__FABA21F2_6F33_40BF_A682_6180473D4EF4__INCLUDED_)