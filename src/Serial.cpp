// Serial.cpp: implementation of the CSerial class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Serial.h"
//#include "stdio.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSerial::CSerial()
{
	m_hComDev=INVALID_HANDLE_VALUE;
	m_bOpened=false;
}


BOOL CSerial::Open( int nPort, int nBaud )
{
//	if( m_bOpened ) return( TRUE );

	char szPort[15];
	DCB dcb;
	DWORD errorno;

	wsprintf( szPort, "COM%d", nPort );
	//wsprintf( szPort, "\\\\.\\COM%d", nPort );
	m_hComDev = CreateFile( szPort, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL );
	//m_hComDev = CreateFile( szPort, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL );
	//m_hComDev = OpenFile( szPort, &lpReOpenBuff, OF_READWRITE);
	
	if( m_hComDev == INVALID_HANDLE_VALUE )
	{
		errorno=GetLastError();
		if(errorno==ERROR_GEN_FAILURE)
		{
			wsprintf( szPort, "\\\\.\\COM%d", nPort );
			//m_hComDev = CreateFile( szPort, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL );
			m_hComDev = CreateFile( szPort, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL );
			//OpenFile( szPort, &lpReOpenBuff, OF_READWRITE);
			if( m_hComDev == INVALID_HANDLE_VALUE ) return( FALSE );
		}
		else return( FALSE );
	}

	open_mode=1;
	//printf("open com sucess\n");
	m_hIDComDev=m_hComDev;

	memset( &m_OverlappedRead, 0, sizeof( OVERLAPPED ) );
	memset( &m_OverlappedWrite, 0, sizeof( OVERLAPPED ) );

	COMMTIMEOUTS CommTimeOuts;
	CommTimeOuts.ReadIntervalTimeout = 0xFFFFFFFF;
	CommTimeOuts.ReadTotalTimeoutMultiplier = 0;
	CommTimeOuts.ReadTotalTimeoutConstant = 0;
	CommTimeOuts.WriteTotalTimeoutMultiplier = 0;
	CommTimeOuts.WriteTotalTimeoutConstant = 5000;
	SetCommTimeouts( m_hComDev, &CommTimeOuts );

	m_OverlappedRead.hEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
	m_OverlappedWrite.hEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

	dcb.DCBlength = sizeof( DCB );
	GetCommState( m_hComDev, &dcb );
	dcb.BaudRate = nBaud;
	dcb.ByteSize = 8;
	if( !SetCommState( m_hComDev, &dcb ) ||
		!SetupComm( m_hComDev, 10000, 10000 ) ||
		m_OverlappedRead.hEvent == NULL ||
		m_OverlappedWrite.hEvent == NULL ){
		DWORD dwError = GetLastError();
		if( m_OverlappedRead.hEvent != NULL ) CloseHandle( m_OverlappedRead.hEvent );
		if( m_OverlappedWrite.hEvent != NULL ) CloseHandle( m_OverlappedWrite.hEvent );
		CloseHandle( m_hComDev );
		return FALSE;
	}

	m_bOpened = TRUE;

	return m_bOpened;

}

BOOL CSerial::Open( const char *com_name, int nBaud )
{
//	if( m_bOpened ) return( TRUE );

	char szPort[15];
	DCB dcb;

	wsprintf( szPort, "%s", com_name );
	m_hComDev = CreateFile( szPort, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL );
	//m_hComDev = OpenFile( szPort, &lpReOpenBuff, OF_READWRITE);
	
	if( m_hComDev == INVALID_HANDLE_VALUE ) return( FALSE );
	//printf("open com sucess\n");
	m_hIDComDev=m_hComDev;

	memset( &m_OverlappedRead, 0, sizeof( OVERLAPPED ) );
	memset( &m_OverlappedWrite, 0, sizeof( OVERLAPPED ) );

	COMMTIMEOUTS CommTimeOuts;
	CommTimeOuts.ReadIntervalTimeout = 0xFFFFFFFF;
	CommTimeOuts.ReadTotalTimeoutMultiplier = 0;
	CommTimeOuts.ReadTotalTimeoutConstant = 0;
	CommTimeOuts.WriteTotalTimeoutMultiplier = 0;
	CommTimeOuts.WriteTotalTimeoutConstant = 5000;
	SetCommTimeouts( m_hComDev, &CommTimeOuts );

	m_OverlappedRead.hEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
	m_OverlappedWrite.hEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

	dcb.DCBlength = sizeof( DCB );
	GetCommState( m_hComDev, &dcb );
	dcb.BaudRate = nBaud;
	dcb.ByteSize = 8;
	if( !SetCommState( m_hComDev, &dcb ) ||
		!SetupComm( m_hComDev, 10000, 10000 ) ||
		m_OverlappedRead.hEvent == NULL ||
		m_OverlappedWrite.hEvent == NULL ){
		DWORD dwError = GetLastError();
		if( m_OverlappedRead.hEvent != NULL ) CloseHandle( m_OverlappedRead.hEvent );
		if( m_OverlappedWrite.hEvent != NULL ) CloseHandle( m_OverlappedWrite.hEvent );
		CloseHandle( m_hComDev );
		return FALSE;
	}

	m_bOpened = TRUE;

	return m_bOpened;

}

BOOL CSerial::Open( const char *com_name, int nBaud, int databits, int stopbits, int parity )
{
//	if( m_bOpened ) return( TRUE );

	char szPort[15];
	DCB dcb;

	wsprintf( szPort, "%s", com_name );
	m_hComDev = CreateFile( szPort, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL );
	//m_hComDev = OpenFile( szPort, &lpReOpenBuff, OF_READWRITE);
	
	if( m_hComDev == INVALID_HANDLE_VALUE ) return( FALSE );
	//printf("open com sucess\n");
	m_hIDComDev=m_hComDev;

	memset( &m_OverlappedRead, 0, sizeof( OVERLAPPED ) );
	memset( &m_OverlappedWrite, 0, sizeof( OVERLAPPED ) );

	COMMTIMEOUTS CommTimeOuts;
	CommTimeOuts.ReadIntervalTimeout = 0xFFFFFFFF;
	CommTimeOuts.ReadTotalTimeoutMultiplier = 0;
	CommTimeOuts.ReadTotalTimeoutConstant = 0;
	CommTimeOuts.WriteTotalTimeoutMultiplier = 0;
	CommTimeOuts.WriteTotalTimeoutConstant = 5000;
	SetCommTimeouts( m_hComDev, &CommTimeOuts );

	m_OverlappedRead.hEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
	m_OverlappedWrite.hEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

	dcb.DCBlength = sizeof( DCB );
	GetCommState( m_hComDev, &dcb );
	
	dcb.BaudRate = nBaud;
	dcb.ByteSize = databits;
	
	dcb.StopBits = ONESTOPBIT;
	if(stopbits==2) dcb.StopBits = TWOSTOPBITS; 
	else if(stopbits==3) dcb.StopBits = ONE5STOPBITS;
	
	dcb.Parity = NOPARITY;
	if(parity==1) dcb.Parity = ODDPARITY; 
	else if(parity==2) dcb.Parity = EVENPARITY;
	else if(parity==3) dcb.Parity = MARKPARITY;
	
	if( !SetCommState( m_hComDev, &dcb ) ||
		!SetupComm( m_hComDev, 10000, 10000 ) ||
		m_OverlappedRead.hEvent == NULL ||
		m_OverlappedWrite.hEvent == NULL ){
		DWORD dwError = GetLastError();
		if( m_OverlappedRead.hEvent != NULL ) CloseHandle( m_OverlappedRead.hEvent );
		if( m_OverlappedWrite.hEvent != NULL ) CloseHandle( m_OverlappedWrite.hEvent );
		CloseHandle( m_hComDev );
		return FALSE;
	}

	m_bOpened = TRUE;

	return m_bOpened;

}

int CSerial::InBufferCount( void )
{

	if( !m_bOpened || m_hComDev == NULL ) return( 0 );

	DWORD dwErrorFlags;
	COMSTAT ComStat;

	ClearCommError( m_hIDComDev, &dwErrorFlags, &ComStat );

	return (int)ComStat.cbInQue;

}

DWORD CSerial::ReadData( void *buffer, DWORD dwBytesRead)
{

	if( !m_bOpened || m_hComDev == NULL ) return 0;

	BOOL bReadStatus;
	DWORD dwErrorFlags;
	COMSTAT ComStat;

	ClearCommError( m_hComDev, &dwErrorFlags, &ComStat );
	if( !ComStat.cbInQue ) return 0;
	dwBytesRead = min(dwBytesRead,(DWORD) ComStat.cbInQue);
	bReadStatus = ReadFile( m_hComDev, buffer, dwBytesRead, &dwBytesRead, &m_OverlappedRead );
	if( !bReadStatus ){
	//	if( GetLastError() == ERROR_IO_PENDING ){
	//		WaitForSingleObject( m_OverlappedRead.hEvent, 2000 );
	//		return dwBytesRead;
	//	}
		return 0;
	}

	return dwBytesRead;

}

DWORD CSerial::SendData( const char *buffer, DWORD dwBytesWritten)
{

	if( !m_bOpened || m_hComDev == NULL ) return( 0 );

	BOOL bWriteStat;

	bWriteStat = WriteFile( m_hComDev, buffer, dwBytesWritten, &dwBytesWritten, &m_OverlappedWrite );
	if( !bWriteStat){
	if ( GetLastError() == ERROR_IO_PENDING ) {
	WaitForSingleObject( m_OverlappedWrite.hEvent, 1000 );
	return dwBytesWritten;
	}
	return 0;
	}
	return dwBytesWritten;

}
//BOOL CSerial::Close( HANDLE comHandlevoid )
BOOL CSerial::Close()
{
	m_bOpened = FALSE;
	return CloseHandle(m_hComDev);
}

CSerial::~CSerial()
{
}
