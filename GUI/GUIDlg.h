
// GUIDlg.h: 头文件
//

#pragma once

#define WM_LOAD_DONE		(WM_USER + 1)
#define WM_POST_LOG		(WM_USER + 2)

// CGUIDlg 对话框
class CGUIDlg : public CDialogEx
{
// 构造
public:
	CGUIDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_GUI_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;
	CWinThread* task;
	HANDLE rPipe, wPipe;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg LRESULT OnLoadDone(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnPostLog(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnPaint();
	afx_msg void OnClose();
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	afx_msg void OnBnClickedSelect();
	afx_msg void OnBnClickedFree();
	afx_msg void OnBnClickedPage();
	afx_msg void OnBnClickedLclr();
	afx_msg void OnBnClickedLoad();
	afx_msg void OnEnChangePath();
	afx_msg void OnLogMessage(const CString& msg);
	CString path;
	CString stat;
	CEdit logs;
};
