
// GUIDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "GUI.h"
#include "GUIDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CGUIDlg 对话框



CGUIDlg::CGUIDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_GUI_DIALOG, pParent)
	, path(_T(""))
	, stat(_T("请选择驱动文件"))
	, rPipe(NULL)
	, wPipe(NULL)
	, task(NULL)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CGUIDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_PATH, path);
	DDX_Text(pDX, IDC_STAT, stat);
	DDX_Control(pDX, IDC_LOGS, logs);
}

BEGIN_MESSAGE_MAP(CGUIDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_SELECT, &CGUIDlg::OnBnClickedSelect)
	ON_BN_CLICKED(IDC_FREE, &CGUIDlg::OnBnClickedFree)
	ON_BN_CLICKED(IDC_PAGE, &CGUIDlg::OnBnClickedPage)
	ON_BN_CLICKED(IDC_LCLR, &CGUIDlg::OnBnClickedLclr)
	ON_BN_CLICKED(IDC_LOAD, &CGUIDlg::OnBnClickedLoad)
	ON_EN_CHANGE(IDC_PATH, &CGUIDlg::OnEnChangePath)
	ON_MESSAGE(WM_LOAD_DONE, &CGUIDlg::OnLoadDone)
	ON_MESSAGE(WM_POST_LOG, &CGUIDlg::OnPostLog)
	ON_WM_GETMINMAXINFO()
	ON_WM_CLOSE()
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CGUIDlg 消息处理程序

BOOL CGUIDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
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

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);			// 设置小图标

	SECURITY_ATTRIBUTES sa = { sizeof(sa), NULL, TRUE };
	if (!CreatePipe(&rPipe, &wPipe, &sa, 0))
	{
		OnLogMessage(_T("无法创建通信管道"));
	}
	else
	{
		_dup2(_open_osfhandle((intptr_t)wPipe, _O_WRONLY), _fileno(stdout));
		setvbuf(stdout, NULL, _IONBF, 0);
	}

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CGUIDlg::OnClose()
{
	OnLoadDone(NULL, NULL);
	if (rPipe)
	{
		CloseHandle(rPipe);
		rPipe = NULL;
	}
	if (wPipe)
	{
		CloseHandle(wPipe);
		wPipe = NULL;
	}
	CDialogEx::OnClose();
}

void CGUIDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 1)
	{
		OnLoadDone(NULL, NULL);
	}
	else if (nIDEvent == 2)
	{
		char buffer[4096] = { 0 };
		DWORD bytes;
		CString msg, tmp;
		if (PeekNamedPipe(rPipe, NULL, 0, NULL, &bytes, NULL) && bytes > 0 &&
			ReadFile(rPipe, buffer, sizeof(buffer) - 1, &bytes, NULL))
		{
			buffer[bytes] = 0;
			msg.Format(_T("%s"), buffer);
			for (int iPos = 0; AfxExtractSubString(tmp, msg, iPos, '\n'); iPos++)
			{
				OnLogMessage(tmp.Trim());
			}
		}
	}
	CDialogEx::OnTimer(nIDEvent);
}

void CGUIDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CGUIDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CGUIDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CGUIDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	lpMMI->ptMinTrackSize.x = 920;
	lpMMI->ptMinTrackSize.y = 828;

	CDialogEx::OnGetMinMaxInfo(lpMMI);
}

void CGUIDlg::OnLogMessage(const CString& msg)
{
	if (!msg.GetLength()) return;
	CString tstr;
	SYSTEMTIME ts;
	GetLocalTime(&ts);
	tstr.Format(
		_T("[%02d:%02d:%02d.%03d] "),
		ts.wHour,
		ts.wMinute,
		ts.wSecond,
		ts.wMilliseconds
	);
	tstr += msg;
	logs.SetSel(-1, -1);
	logs.ReplaceSel(tstr.Trim() + _T("\r\n"));
	logs.PostMessageW(WM_VSCROLL, SB_BOTTOM, 0);
}

LRESULT CGUIDlg::OnPostLog(WPARAM wParam, LPARAM lParam)
{
	CString* msg = reinterpret_cast<CString*>(wParam);
	OnLogMessage(*msg);
	delete msg;
	return 0;
}

void CGUIDlg::OnBnClickedSelect()
{
	TCHAR szFilter[] = _T("驱动文件(*.sys)|*.sys|所有文件(*.*)|*.*||");
	CFileDialog fileDlg(TRUE, _T("sys"), NULL, OFN_FILEMUSTEXIST, szFilter);
	if (IDOK == fileDlg.DoModal())
	{
		SetDlgItemText(IDC_PATH, fileDlg.GetPathName());
	}
}

void CGUIDlg::OnEnChangePath()
{
	if (task)
	{
		SetDlgItemText(IDC_STAT, _T("正在加载中"));
		return;
	}
	UpdateData(TRUE);
	CFileStatus st;
	if (CFile::GetStatus(path, st) && st.m_size)
	{
		((CButton*)GetDlgItem(IDC_LOAD))->EnableWindow(TRUE);
		SetDlgItemText(IDC_STAT, _T("就绪"));
	}
	else
	{
		((CButton*)GetDlgItem(IDC_LOAD))->EnableWindow(FALSE);
		SetDlgItemText(IDC_STAT, path.GetLength() ? _T("无效文件: ") + path : _T("请选择驱动文件"));
	}
}

void CGUIDlg::OnBnClickedFree()
{
	if (IsDlgButtonChecked(IDC_FREE) && IsDlgButtonChecked(IDC_PAGE))
	{
		((CButton *)GetDlgItem(IDC_PAGE))->SetCheck(0);
	}
}

void CGUIDlg::OnBnClickedPage()
{
	if (IsDlgButtonChecked(IDC_FREE) && IsDlgButtonChecked(IDC_PAGE))
	{
		((CButton*)GetDlgItem(IDC_FREE))->SetCheck(0);
	}
}

void CGUIDlg::OnBnClickedLclr()
{
	logs.SetWindowText(_T(""));
}

struct ThreadParams {
	bool freeMem, indPage, passPtr, copyHdr;
	std::wstring drvPath;
};

static UINT OnLoadDriver(LPVOID pParam);

void CGUIDlg::OnBnClickedLoad()
{
	static ThreadParams params;
	params.freeMem = IsDlgButtonChecked(IDC_FREE);
	params.indPage = IsDlgButtonChecked(IDC_PAGE);
	params.passPtr = IsDlgButtonChecked(IDC_PPTR);
	params.copyHdr = IsDlgButtonChecked(IDC_COPY);
	params.drvPath = path.GetString();
	if (!(task = AfxBeginThread(OnLoadDriver, &params)))
	{
		OnLogMessage(_T("无法创建任务线程"));
		return;
	}
	((CButton*)GetDlgItem(IDC_LOAD))->EnableWindow(FALSE);
	SetTimer(1, 15000, NULL);	// tiemout: 15s
	SetTimer(2, 100, NULL);		// read from pipe every 100ms
}

LRESULT CGUIDlg::OnLoadDone(WPARAM wParam, LPARAM lParam)
{
	if (!task) return 0;
	if (WaitForSingleObject(task->m_hThread, 1000) == WAIT_TIMEOUT)
	{
		TerminateThread(task->m_hThread, 0);
		CString err = _T("加载超时！驱动没有及时退出，入口函数中可能有死循环或加载参数配置错误");
		SetDlgItemText(IDC_STAT, err);
		OnLogMessage(err);
	}
	else
	{
		DWORD rc = 0;
		GetExitCodeThread(task->m_hThread, &rc);
		CString msg;
		msg.Format(_T("加载完成：%d"), rc);
		OnLogMessage(msg);
	}
	task = NULL;
	KillTimer(1);
	KillTimer(2);
	((CButton*)GetDlgItem(IDC_LOAD))->EnableWindow(TRUE);
	return 0;
}

#ifndef NTSTATUS
#define NTSTATUS LONG
#endif
#include <Windows.h>
#include <iostream>
#include <vector>
#include <TlHelp32.h>
#include <kdmapper.hpp>
#include <utils.hpp>
#include <intel_driver.hpp>

HANDLE iqvw64e_device_handle;

LONG WINAPI SimplestCrashHandler(EXCEPTION_POINTERS* ExceptionInfo)
{
	if (ExceptionInfo && ExceptionInfo->ExceptionRecord)
		Log(L"[!] Crash at addr 0x" << ExceptionInfo->ExceptionRecord->ExceptionAddress << L" by 0x" << std::hex << ExceptionInfo->ExceptionRecord->ExceptionCode << std::endl);
	else
		Log(L"[!] Crash" << std::endl);

	if (iqvw64e_device_handle)
		intel_driver::Unload(iqvw64e_device_handle);

	return EXCEPTION_EXECUTE_HANDLER;
}

int paramExists(const int argc, wchar_t** argv, const wchar_t* param) {
	size_t plen = wcslen(param);
	for (int i = 1; i < argc; i++) {
		if (wcslen(argv[i]) == plen + 1ull && _wcsicmp(&argv[i][1], param) == 0 && argv[i][0] == '/') { // with slash
			return i;
		}
		else if (wcslen(argv[i]) == plen + 2ull && _wcsicmp(&argv[i][2], param) == 0 && argv[i][0] == '-' && argv[i][1] == '-') { // with double dash
			return i;
		}
	}
	return -1;
}

bool callbackExample(ULONG64* param1, ULONG64* param2, ULONG64 allocationPtr, ULONG64 allocationSize) {
	UNREFERENCED_PARAMETER(param1);
	UNREFERENCED_PARAMETER(param2);
	UNREFERENCED_PARAMETER(allocationPtr);
	UNREFERENCED_PARAMETER(allocationSize);
	Log("[+] Callback example called" << std::endl);

	/*
	This callback occurs before call driver entry and
	can be useful to pass more customized params in
	the last step of the mapping procedure since you
	know now the mapping address and other things
	*/
	return true;
}

static UINT OnLoadDriver(LPVOID pParam)
{
	int rc = 0;
	std::vector<uint8_t> raw_image = { 0 };
	ThreadParams* params = (ThreadParams*)pParam;
	if (params->freeMem) Log(L"[+] Free pool memory after usage enabled" << std::endl);
	if (params->indPage) Log(L"[+] Allocate Independent Pages mode enabled" << std::endl);
	if (params->passPtr) Log(L"[+] Pass Allocation Ptr as first param enabled" << std::endl);
	if (params->copyHdr) Log(L"[+] Copying driver header enabled" << std::endl);

	SetUnhandledExceptionFilter(SimplestCrashHandler);

	iqvw64e_device_handle = intel_driver::Load();
	if (iqvw64e_device_handle == INVALID_HANDLE_VALUE) goto error;

	if (!utils::ReadFileToMemory(params->drvPath, &raw_image))
	{
		Log(L"[-] Failed to read image to memory" << std::endl);
		intel_driver::Unload(iqvw64e_device_handle);
		goto error;
	}

	kdmapper::AllocationMode mode = params->indPage
		? kdmapper::AllocationMode::AllocateIndependentPages
		: kdmapper::AllocationMode::AllocatePool;

	NTSTATUS exitCode = 0;
	if (!kdmapper::MapDriver(
			iqvw64e_device_handle, raw_image.data(), 0, 0,
			params->freeMem, !params->copyHdr, mode,
			params->passPtr, callbackExample, &exitCode)
	) {
		Log(L"[-] Failed to map " << params->drvPath << std::endl);
		intel_driver::Unload(iqvw64e_device_handle);
		goto error;
	}

	if (!intel_driver::Unload(iqvw64e_device_handle))
	{
		Log(L"[-] Warning failed to fully unload vulnerable driver " << std::endl);
	}
	else
	{
		Log(L"[+] success" << std::endl);
		goto exit;
	}

error:
	rc = -1;
exit:
	fflush(stdout);
	Sleep(200);
	CWnd* pMainWnd = AfxGetMainWnd();
	if (pMainWnd)
	{
		pMainWnd->PostMessage(WM_LOAD_DONE, NULL, NULL);
	}
	return rc;
}