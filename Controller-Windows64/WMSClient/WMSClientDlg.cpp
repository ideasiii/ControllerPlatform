
// WMSClientDlg.cpp : 實作檔
//

#include "stdafx.h"
#include "WMSClient.h"
#include "WMSClientDlg.h"
#include "afxdialogex.h"
#include "WinSocket.h"
#include "packet.h"
#include <winsock2.h>
#include <iphlpapi.h>
#include <memory>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace std;

// 對 App About 使用 CAboutDlg 對話方塊

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 對話方塊資料
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支援

// 程式碼實作
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CWMSClientDlg 對話方塊



CWMSClientDlg::CWMSClientDlg(CWnd* pParent /*=NULL*/)
: CDialogEx(CWMSClientDlg::IDD, pParent), pWinSocket(new CWinSocket)
, m_rbCommand(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	pWinSocket->pCWMSClientDlg = this;
}

void CWMSClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_STATUS, listBoxStatus);
	DDX_Control(pDX, IDC_IPADDRESS_WMS, ipCtlIP);
	DDX_Control(pDX, IDC_EDIT_PORT, editPort);
	DDX_Control(pDX, IDC_EDIT_DATA, m_editData);
	DDX_Radio(pDX, IDC_RADIO_ENQUIRE_LINK, m_rbCommand);
	DDV_MinMaxInt(pDX, m_rbCommand, 0, INT_MAX);
}

BEGIN_MESSAGE_MAP(CWMSClientDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_CONNECT, &CWMSClientDlg::OnBnClickedButtonConnect)
	ON_BN_CLICKED(IDC_BUTTON_DISCONNECT, &CWMSClientDlg::OnBnClickedButtonDisconnect)
	ON_BN_CLICKED(IDOK, &CWMSClientDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CWMSClientDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTON_SEND, &CWMSClientDlg::OnBnClickedButtonSend)
END_MESSAGE_MAP()


// CWMSClientDlg 訊息處理常式

BOOL CWMSClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 將 [關於...] 功能表加入系統功能表。

	// IDM_ABOUTBOX 必須在系統命令範圍之中。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 設定此對話方塊的圖示。當應用程式的主視窗不是對話方塊時，
	// 框架會自動從事此作業
	SetIcon(m_hIcon, TRUE);			// 設定大圖示
	SetIcon(m_hIcon, FALSE);		// 設定小圖示

	// TODO:  在此加入額外的初始設定
	GetDlgItem(IDC_EDIT_PORT)->SetWindowText(_T("1414"));
	GetDlgItem(IDC_IPADDRESS_WMS)->SetWindowText(_T("140.92.142.125"));
	GetDlgItem(IDC_BUTTON_SEND)->EnableWindow(false);
//	GetDlgItem(IDC_IPADDRESS_WMS)->SetWindowText(_T("10.0.129.21"));
//	GetDlgItem(IDC_EDIT_CLIENT_MAC)->SetWindowTextW(_T("00:00:00:00:00:00"));

	//optionEnable(0);


	


	return TRUE;  // 傳回 TRUE，除非您對控制項設定焦點
}

void CWMSClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果將最小化按鈕加入您的對話方塊，您需要下列的程式碼，
// 以便繪製圖示。對於使用文件/檢視模式的 MFC 應用程式，
// 框架會自動完成此作業。

void CWMSClientDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 繪製的裝置內容

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 將圖示置中於用戶端矩形
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 描繪圖示
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 當使用者拖曳最小化視窗時，
// 系統呼叫這個功能取得游標顯示。
HCURSOR CWMSClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CWMSClientDlg::OnBnClickedButtonConnect()
{

	GetDlgItem(IDC_BUTTON_CONNECT)->EnableWindow(false);
	
	UpdateData();

	CString csIpAddress, csPort;
	GetDlgItem(IDC_IPADDRESS_WMS)->GetWindowText(csIpAddress);
	GetDlgItem(IDC_EDIT_PORT)->GetWindowText(csPort);
	
	UINT unPort = _ttoi(csPort);

	closeSocket();
	if (pWinSocket->Connect(unPort, csIpAddress))
	{
		addStatus(_T("Socket Connect Success"));
		GetDlgItem(IDC_BUTTON_SEND)->EnableWindow(true);
	}
	else
	{
		addStatus(_T("Socket Connect Fail"));
		GetDlgItem(IDC_BUTTON_CONNECT)->EnableWindow(true);
	}

}


void CWMSClientDlg::OnBnClickedButtonDisconnect()
{
	closeSocket();
}

void CWMSClientDlg::closeSocket()
{
	GetDlgItem(IDC_BUTTON_SEND)->EnableWindow(false);
	GetDlgItem(IDC_BUTTON_CONNECT)->EnableWindow(true);
	pWinSocket->Close();
}

void CWMSClientDlg::OnBnClickedOk()
{
	// TODO:  在此加入控制項告知處理常式程式碼
	
	//CDialogEx::OnOK();
}

void CWMSClientDlg::addStatus(LPCTSTR strMsg)
{
	listBoxStatus.AddString(strMsg);
	listBoxStatus.SetTopIndex(listBoxStatus.GetCount() - 1);

	CString str;
	CSize sz;
	int dx = 0;
	CDC* pDC = listBoxStatus.GetDC();
	for (int i = 0; i < listBoxStatus.GetCount(); ++i)
	{
		listBoxStatus.GetText(i, str);
		sz = pDC->GetTextExtent(str);

		if (sz.cx > dx)
			dx = sz.cx;
	}
	listBoxStatus.ReleaseDC(pDC);

	if (listBoxStatus.GetHorizontalExtent() < dx)
	{
		listBoxStatus.SetHorizontalExtent(dx);
		ASSERT(listBoxStatus.GetHorizontalExtent() == dx);
	}
}


int CWMSClientDlg::formatPacket(int nCommand, void **pPacket )
{
	int nLen;
	int nBody_len = 0;
	int nTotal_len;
	CMP_PACKET packet;
	char * pIndex;
	CString strData;
	CString strMsg;

	packet.cmpHeader.command_id = htonl(nCommand);
	packet.cmpHeader.command_status = htonl(0);
	packet.cmpHeader.sequence_number = htonl(getSequence());

	pIndex = packet.cmpBody.cmpdata;
	memset(packet.cmpBody.cmpdata, 0, sizeof(packet.cmpBody.cmpdata));

	UpdateData();
	m_editData.GetWindowTextW(strData);
	nLen = strData.GetLength();

	if (0 < nLen)
	{
		strData.Replace(_T("\r"), _T(""));
		strData.Replace(_T("\n"),_T( ""));
		strData.Replace(_T(" "), _T(""));
		nLen = strData.GetLength();
		memcpy(pIndex, (LPCSTR)CT2A(strData), nLen);
		pIndex += nLen;
		nBody_len += nLen;
		memcpy(pIndex, "\0", 1);
		pIndex += 1;
		nBody_len += 1;
	}

	nTotal_len = sizeof(CMP_HEADER)+nBody_len;
	packet.cmpHeader.command_length = htonl(nTotal_len);

	memcpy(*pPacket, &packet, nTotal_len);

	strMsg = printPacket(nCommand, 0, ntohl(packet.cmpHeader.sequence_number), nTotal_len);
	addStatus(strMsg);
	if (0 < strData.GetLength())
	{
		addStatus(strData);
	}
	return nTotal_len;
}

int CWMSClientDlg::Receive(char **buf, int buflen)
{
	static int nRun = 0;
	CString strMsg;
	void *pbuf;
	
	pbuf = *buf;
	

	int nLength = 0;
	int nCount = 0;

	vector<string> vData;
	
	char * pBody;
	char temp[MAX_DATA_LEN];

	if (sizeof(CMP_HEADER) <= (unsigned int)buflen)
	{
		CMP_HEADER*pHeader;
		pHeader = (CMP_HEADER*)pbuf;
		int nCommand = ntohl(pHeader->command_id);
		nLength = ntohl(pHeader->command_length);
		int nStatus = ntohl(pHeader->command_status);
		int nSequence = ntohl(pHeader->sequence_number);
		

		pBody = (char*)((char *) const_cast<void*>(pbuf)+sizeof(CMP_HEADER));

		strMsg = printPacket(nCommand, nStatus, nSequence, nLength);
		addStatus(strMsg);

		switch (nCommand)
		{
		case authentication_response:
			strMsg.Format(_T("[authentication_response]: length=%d command=%x status=%d sequence=%d"), nLength, nCommand, nStatus, nSequence);
			addStatus(strMsg);
			memset(temp, 0, sizeof(temp));
			strcpy_s(temp, pBody);
			nCount = parseBody(temp, vData);
			if (0 < nCount)
			{
				size_t nLen = _mbstrlen(vData[0].c_str()) + 1;
				size_t converted = 0;
				wchar_t * pwstr = new wchar_t[nLen];
				mbstowcs_s(&converted, pwstr, nLen, vData[0].c_str(), _TRUNCATE);

				nLen = _mbstrlen(vData[1].c_str()) + 1;
				converted = 0;
				wchar_t * pwstr2 = new wchar_t[nLen];
				mbstowcs_s(&converted, pwstr2, nLen, vData[1].c_str(), _TRUNCATE);
				strMsg.Format(_T("Client MAC:%s Auth_status:%s"), pwstr, pwstr2);
				delete [] pwstr;
				delete [] pwstr2;
				addStatus(strMsg);
			}
			break;
		}
		
	}
	else
	{
		// 不是CMP封包
		pBody = (char*)((char *) const_cast<void*>(pbuf));
		size_t nLen = buflen + 1;
		size_t converted = 0;
		wchar_t * pwstr = new wchar_t[nLen];
		mbstowcs_s(&converted, pwstr, nLen, pBody, _TRUNCATE);
		strMsg.Format(_T("%s"), pwstr);
		addStatus(strMsg);
		delete [] pwstr;
	}

	return nLength;
}

int CWMSClientDlg::parseBody(char *pData, vector<string> &vData)
{
	char * pch;
	char *next_token = NULL;

	pch = strtok_s(pData, " ", &next_token);
	while (pch != NULL)
	{
		vData.push_back(string(pch));
		pch = strtok_s(NULL, " ", &next_token);
	}

	return vData.size();
}

void CWMSClientDlg::OnBnClickedCancel()
{
	// TODO:  在此加入控制項告知處理常式程式碼
	pWinSocket->Close();
	CDialogEx::OnCancel();
}

/*
void CWMSClientDlg::OnBnClickedButtonUnbind()
{
	// TODO:  在此加入控制項告知處理常式程式碼
	char buf[MAX_DATA_LEN];
	void *pbuf;
	pbuf = buf;

	memset(buf, 0, sizeof(buf));
	int nPacketLen = formatPacket(unbind_request, &pbuf);
	if (sizeof(WMP_HEADER) <= nPacketLen)
	{
		int nSendLen = pWinSocket->Send(pbuf, nPacketLen);
		if (nSendLen == nPacketLen)
		{
			WMP_HEADER *pHeader;
			pHeader = (WMP_HEADER *)pbuf;
			int nCommand = ntohl(pHeader->command_id);
			int nLength = ntohl(pHeader->command_length);
			int nStatus = ntohl(pHeader->command_status);
			int nSequence = ntohl(pHeader->sequence_number);
			CString strMsg;
			strMsg.Format(_T("[unbind_request]: length=%d command=%x status=%d sequence=%d"), nLength, nCommand, nStatus, nSequence);
			addStatus(strMsg);
			return;
		}
	}
	addStatus(_T("[unbind_request]: Fail"));
}


void CWMSClientDlg::OnBnClickedButtonAuthentication()
{
	// TODO:  在此加入控制項告知處理常式程式碼
	char buf[MAX_DATA_LEN];
	void *pbuf;
	pbuf = buf;

	memset(buf, 0, sizeof(buf));
	int nPacketLen = formatPacket(authentication_request, &pbuf);
	if (sizeof(WMP_HEADER) <= nPacketLen)
	{
		int nSendLen = pWinSocket->Send(pbuf, nPacketLen);
		if (nSendLen == nPacketLen)
		{
			WMP_HEADER *pHeader;
			pHeader = (WMP_HEADER *)pbuf;
			int nCommand = ntohl(pHeader->command_id);
			int nLength = ntohl(pHeader->command_length);
			int nStatus = ntohl(pHeader->command_status);
			int nSequence = ntohl(pHeader->sequence_number);
			CString strMsg;
			strMsg.Format(_T("[authentication_request]: length=%d command=%x status=%d sequence=%d"), nLength, nCommand, nStatus, nSequence);
			addStatus(strMsg);
			return;
		}
	}
	addStatus(_T("[authentication_request]: Fail"));
}


void CWMSClientDlg::OnBnClickedButtonEnquireLink()
{
	// TODO:  在此加入控制項告知處理常式程式碼
	char buf[MAX_DATA_LEN];
	void *pbuf;
	pbuf = buf;

	memset(buf, 0, sizeof(buf));
	int nPacketLen = formatPacket(enquire_link_request, &pbuf);
	if (sizeof(WMP_HEADER) <= nPacketLen)
	{
		int nSendLen = pWinSocket->Send(pbuf, nPacketLen);
		if (nSendLen == nPacketLen)
		{
			WMP_HEADER *pHeader;
			pHeader = (WMP_HEADER *)pbuf;
			int nCommand = ntohl(pHeader->command_id);
			int nLength = ntohl(pHeader->command_length);
			int nStatus = ntohl(pHeader->command_status);
			int nSequence = ntohl(pHeader->sequence_number);
			CString strMsg;
			strMsg.Format(_T("[enquire_link_request]: length=%d command=%x status=%d sequence=%d"), nLength, nCommand, nStatus, nSequence);
			addStatus(strMsg);
			return;
		}
	}
	addStatus(_T("[enquire_link_request]: Fail"));
}

void CWMSClientDlg::OnBnClickedButtonBind()
{
	char buf[MAX_DATA_LEN];
	void *pbuf;
	pbuf = buf;

	memset(buf, 0, sizeof(buf));
	int nPacketLen = formatPacket(bind_request, &pbuf);
	if (sizeof(WMP_HEADER) <= nPacketLen)
	{
		int nSendLen = pWinSocket->Send(pbuf, nPacketLen);
		if (nSendLen == nPacketLen)
		{
			WMP_HEADER *pHeader;
			pHeader = (WMP_HEADER *)pbuf;
			int nCommand = ntohl(pHeader->command_id);
			int nLength = ntohl(pHeader->command_length);
			int nStatus = ntohl(pHeader->command_status);
			int nSequence = ntohl(pHeader->sequence_number);
			CString strMsg;
			strMsg.Format(_T("[bind_request]: length=%d command=%x status=%d sequence=%d"), nLength, nCommand, nStatus, nSequence);
			addStatus(strMsg);
			return;
		}
	}
	addStatus(_T("Bind request Fail"));

}
*/


void CWMSClientDlg::OnBnClickedButtonSend()
{
	UpdateData();
	int nCommand;
	int nPacketLen;
	int nSendLen;
	char buf[MAX_DATA_LEN];
	void* pbuf;
	CString strMsg;

	pbuf = buf;

	switch (m_rbCommand)
	{
	case ENQUIRE:
		nCommand = enquire_link_request;
		break;
	case DEIDENTIFY:
		nCommand = deidentify_request;
		break;
	case SEMANTIC:
		nCommand = semantic_word_request;
		break;
	default:
		nCommand = enquire_link_request;
		break;
	}

	nPacketLen = formatPacket(nCommand, &pbuf);
	nSendLen = pWinSocket->Send(pbuf, nPacketLen);
	if (nSendLen == nPacketLen)
	{
		addStatus(_T("Data Send Success!!"));
	}
}
