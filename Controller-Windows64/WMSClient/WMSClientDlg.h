
// WMSClientDlg.h : ���Y��
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include <vector>
#include <string>

class CWinSocket;

// CWMSClientDlg ��ܤ��
class CWMSClientDlg : public CDialogEx
{
	enum {ENQUIRE=0,DEIDENTIFY,SEMANTIC};
// �غc
public:
	CWMSClientDlg(CWnd* pParent = NULL);	// �зǫغc�禡

// ��ܤ�����
	enum { IDD = IDD_WMSCLIENT_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV �䴩


// �{���X��@
public:
	void addStatus(LPCTSTR strMsg);

protected:
	HICON m_hIcon;

	// ���ͪ��T�������禡
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CListBox listBoxStatus;
	afx_msg void OnBnClickedButtonConnect();
	afx_msg void OnBnClickedButtonDisconnect();

private:
	CWinSocket *pWinSocket;
	int formatPacket(int nCommand, void **pPacket );
	int parseBody(char *pData, std::vector<std::string> &vData);

public:
	CIPAddressCtrl ipCtlIP;
	CEdit editPort;
	afx_msg void OnBnClickedOk();
	int Receive(char **buf, int buflen);
	afx_msg void OnBnClickedCancel();
	CEdit m_editData;
	// CMP Command ID
	int m_rbCommand;
	afx_msg void OnBnClickedButtonSend();
};
