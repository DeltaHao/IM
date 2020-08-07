#include "stdafx.h"
#include "Client.h"
#include "ClientModel.h"
#include "ClientView_ClientPresenter.h"
#include <string>

void CString2Char(CString str, char ch[]){
    int i;
    char *tmpch;
    int wLen = WideCharToMultiByte(CP_ACP, 0, str, -1, NULL, 0, NULL, NULL);//得到Char的长度
    tmpch = new char[wLen + 1];                                             //分配变量的地址大小
    WideCharToMultiByte(CP_ACP, 0, str, -1, tmpch, wLen, NULL, NULL);       //将CString转换成char*

    for (i = 0; tmpch[i] != '\0'; i++) ch[i] = tmpch[i];
    ch[i] = '\0';
}


CClientSocket::CClientSocket(CallBack& callback):m_callback(callback){
    m_pBuffer = new char[1024];
}
void CClientSocket::OnConnect(int nErrorCode){
    if (nErrorCode == 0){
        //连接成功
        m_callback.OnConnectSuccess();//利用回调函数通知Presenter
    }
    AsyncSelect(FD_READ);
    CAsyncSocket::OnConnect(nErrorCode);
}
void CClientSocket::OnReceive(int nErrorCode){  
    m_intLen = Receive(m_pBuffer, sizeof(m_pBuffer), 0);
    m_pBuffer[m_intLen] = 0;
    CString strMessage(m_pBuffer);
    while (m_intLen > 0){
        memset(m_pBuffer, 0, sizeof(m_pBuffer));
        m_intLen = Receive(m_pBuffer, sizeof(m_pBuffer), 0);
        m_pBuffer[m_intLen] = 0;
        strMessage += m_pBuffer;
    }
    m_callback.OnRecvMessage(strMessage);
    
    
    CAsyncSocket::OnReceive(nErrorCode);
}
void CClientSocket::OnSend(int nErrorCode){
    Send(m_pBuffer, m_intLen, 0);
    m_intLen = 0;
    memset(m_pBuffer, 0, sizeof(m_pBuffer));
    AsyncSelect(FD_READ);
    CAsyncSocket::OnSend(nErrorCode);
}



CFileSocket::CFileSocket(CallBack& callback) :CClientSocket(callback){}
void CFileSocket::OnConnect(int nErrorCode){
    AsyncSelect(FD_READ);
    CAsyncSocket::OnConnect(nErrorCode);
}
void CFileSocket::OnReceive(int nErrorCode){

    char fName[64] = {0};
    Receive((char*)&fName, sizeof(fName), 0);//接受文件名
    long long siz;
    Receive((char*)&siz, sizeof(siz), 0);//接受文件长度
    siz = siz / 1024;
    std::string strPath = "FileCache/";
    strPath += fName;
    fp = fopen(const_cast<char*>(strPath.c_str()), "wb+");//创建文件
    long long int index = 0;
    int num;
    while (index <= siz){//分块接收文件
        num = Receive(m_pBuffer, 1024, 0);
        if (num <= 0)
            break;
        fwrite(m_pBuffer, (int)num, 1, fp);
        index++;
    }
    fclose(fp);
    //通知presenter 收到文件了
    CString s(fName);
    m_callback.OnRecvFile(s);

    CAsyncSocket::OnReceive(nErrorCode);
}
void CFileSocket::OnSend(int nErrorCode){
    char fName[64] = { 0 };
    //打开文件
    char name[32], ext[16];
    _splitpath(m_pBuffer, NULL, NULL, name, ext);
    strcat_s(name, 32, ext);
    strcpy_s(fName, 32, name);
    fp = fopen(m_pBuffer, "rb");
    //发送文件
    Send((char*)&fName, sizeof(fName), 0);//发送文件名
    fseek(fp, 0, SEEK_END);
    long long siz = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    Send((CHAR*)&siz, sizeof(siz), 0);//发送文件长度
    siz = siz / 1024;
    long long index = 0;
    int num;
    while (1) { // 分块发送文件
        num = fread(m_pBuffer, 1, 1024, fp);
        if (num == 0)
            break;
        Send(m_pBuffer, num, 0);
        index++;
    }
    fclose(fp);
    AsyncSelect(FD_READ);
    CAsyncSocket::OnSend(nErrorCode);
}



void ClientModel::connectToServer(CString strAddr, int nPort, CallBack& callback){
    m_ClientSocket = new CClientSocket(callback);
    if (m_ClientSocket->m_hSocket == INVALID_SOCKET){
        if (!m_ClientSocket->Create(0, SOCK_STREAM, FD_CONNECT)){
            AfxMessageBox(_T("Socket创建失败！"));
            m_ClientSocket->Close();
            PostQuitMessage(0);//退出窗口
            return;
        }
    }
    m_ClientSocket->Connect(strAddr, nPort);

    m_FileSocket = new CFileSocket(callback);
    if (m_FileSocket->m_hSocket == INVALID_SOCKET){
        if (!m_FileSocket->Create(0, SOCK_STREAM, FD_CONNECT)){
            AfxMessageBox(_T("Socket创建失败！"));
            m_FileSocket->Close();
            PostQuitMessage(0);//退出窗口
            return;
        }
    }
    m_FileSocket->Connect(strAddr, nPort+1);//连接到另一个端口
}
void ClientModel::sendMessage(CString strText){
    //将CString 转为char*
    char* szMessage = new char[1024];
    CString2Char(strText, szMessage);
    int nLen = strlen(szMessage);

    //传递给socket发送，发送的是
    m_ClientSocket->m_pBuffer = szMessage;
    m_ClientSocket->m_intLen = nLen;
    m_ClientSocket->AsyncSelect(FD_WRITE);//触发通信套接字的OnSend函数
}
void ClientModel::sendFile(CString strFilePath){
    //将CString 转为char*
    char* szFileName = new char[1024];
    CString2Char(strFilePath, szFileName);
    int nLen = strlen(szFileName);
    //触发文件发送消息
    m_FileSocket->m_pBuffer = szFileName;
    m_FileSocket->m_intLen = nLen;
    m_FileSocket->AsyncSelect(FD_WRITE);
}