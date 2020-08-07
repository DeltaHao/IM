
// ServerView_ServerPresenter.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "Server.h"
#include "ServerView_ServerPresenter.h"
#include "afxdialogex.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ServerView �Ի���


ServerView::ServerView(CWnd* pParent /*=NULL*/)
	: CDialogEx(ServerView::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    p_presenter = new ServerPresenter();
    p_presenter->p_view = this;
}

void ServerView::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT1, m_recvEdit);
    DDX_Control(pDX, IDC_EDIT2, m_sendEdit);
    DDX_Control(pDX, IDC_EDIT3, m_sendFileEdit);
}

BEGIN_MESSAGE_MAP(ServerView, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDC_BUTTON1, &ServerView::OnBnClickedButton1)
    ON_BN_CLICKED(IDC_BUTTON2, &ServerView::OnBnClickedButton2)
    ON_BN_CLICKED(IDC_BUTTON3, &ServerView::OnBnClickedButton3)
    ON_BN_CLICKED(IDC_BUTTON4, &ServerView::OnBnClickedButton4)
END_MESSAGE_MAP()

// ServerView ��Ϣ�������

BOOL ServerView::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO:  �ڴ���Ӷ���ĳ�ʼ������
    m_recvEdit.SetWindowText(_T("----------------������δ��----------------"));
    m_sendFileEdit.SetWindowText(_T("����ק�ļ����������ϴ���"));
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void ServerView::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

HCURSOR ServerView::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

BOOL ServerView::PreTranslateMessage(MSG* pMsg){
    //���س���������ʱ��������Ϣ
    if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN){
        p_presenter->sendMessage();
        return TRUE;
    }
    else
        return CDialog::PreTranslateMessage(pMsg);
}

void ServerView::OnBnClickedButton1()
{
    //������Ϣ
    p_presenter->sendMessage();
}

void ServerView::OnBnClickedButton2()
{
    // ���з�����
    p_presenter->startServer();
}

void ServerView::OnBnClickedButton3()
{
    // �����ļ�
    p_presenter->sendFile();
}

void ServerView::OnBnClickedButton4()
{
    //�鿴�����¼
    p_presenter->showMessageLog();
}


//ServerPresenter 

ServerPresenter::ServerPresenter(){
    p_model = new ServerModel();
}

void ServerPresenter::startServer(){
    if (!m_bListening){
        m_bListening = TRUE;
        m_strRecvText += _T("------------------���ڼ���--------------------");
        p_view->m_recvEdit.SetWindowText(m_strRecvText);
        p_model->startServer(12000, *this);
    }
    else {
        AfxMessageBox(_T("�Ѿ��򿪷�������"));
    }
}

void ServerPresenter::OnConnectSuccess(){
    m_strRecvText += _T("\r\n---------------�ͻ��˳ɹ�����----------------");
    p_view->m_recvEdit.SetWindowText(m_strRecvText);
    m_bConnected = TRUE;
}

void ServerPresenter::sendMessage(){
    if (m_bConnected){
        CString strText;
        p_view->m_sendEdit.GetWindowText(strText);
        if (!strText.GetLength()){
            AfxMessageBox(_T("�������ݲ���Ϊ�գ�"));
            return;
        }

        m_strRecvText += _T("\r\nServer:") + strText;
        p_view->m_recvEdit.SetWindowText(m_strRecvText);
        p_view->m_sendEdit.SetWindowText(_T(""));

        p_model->sendMessage(strText);
        
    }
    else{
        AfxMessageBox(_T("����ʧ�ܣ�û�пͻ������ӣ�"));
    }
}

void ServerPresenter::OnRecvMessage(CString strMessage){
    m_strRecvText += _T("\r\nClient:") + strMessage;
    p_view->m_recvEdit.SetWindowTextW(m_strRecvText);
}

void ServerPresenter::sendFile(){
    if (m_bConnected){
        CString strFilePath;
        p_view->m_sendFileEdit.GetWindowText(strFilePath);
        p_model->sendFile(strFilePath);

        m_strRecvText += _T("\r\n����ͻ��˷����ļ���") + strFilePath;
        p_view->m_recvEdit.SetWindowText(m_strRecvText);
    }
    else{
        AfxMessageBox(_T("����ʧ�ܣ�û�пͻ������ӣ�"));
    }
}

void ServerPresenter::OnRecvFile(CString strFileName){
    m_strRecvText += _T("\r\n���յ��ͻ��˷������ļ���") + strFileName;
    p_view->m_recvEdit.SetWindowTextW(m_strRecvText);
}

void ServerPresenter::OnConnectClose(){
    m_strRecvText += _T("\r\n--------------�ͻ����ѶϿ�����--------------");
    p_view->m_recvEdit.SetWindowTextW(m_strRecvText);
    m_bConnected = false;
}

void ServerPresenter::showMessageLog(){
    if (m_bConnected){
        p_view->m_messageLogDlg = new CMessageLogDlg();
        p_view->m_messageLogDlg->Create(IDD_DIALOG1);
        p_view->m_messageLogDlg->ShowWindow(SW_SHOWNORMAL);
        CString strText = p_model->showMessageLog();
        p_view->m_messageLogDlg->m_mainEdit.SetWindowTextW(strText);
    }
    else{
        AfxMessageBox(_T("û�пͻ������ӣ�"));
    }
    
}