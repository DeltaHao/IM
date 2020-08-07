#include "stdafx.h"
#include "Client.h"
#include "ClientModel.h"
#include "ClientView_ClientPresenter.h"
#include <string>

void CString2Char(CString str, char ch[]){
    int i;
    char *tmpch;
    int wLen = WideCharToMultiByte(CP_ACP, 0, str, -1, NULL, 0, NULL, NULL);//�õ�Char�ĳ���
    tmpch = new char[wLen + 1];                                             //��������ĵ�ַ��С
    WideCharToMultiByte(CP_ACP, 0, str, -1, tmpch, wLen, NULL, NULL);       //��CStringת����char*

    for (i = 0; tmpch[i] != '\0'; i++) ch[i] = tmpch[i];
    ch[i] = '\0';
}


CClientSocket::CClientSocket(CallBack& callback):m_callback(callback){
    m_pBuffer = new char[1024];
}
void CClientSocket::OnConnect(int nErrorCode){
    if (nErrorCode == 0){
        //���ӳɹ�
        m_callback.OnConnectSuccess();//���ûص�����֪ͨPresenter
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
    Receive((char*)&fName, sizeof(fName), 0);//�����ļ���
    long long siz;
    Receive((char*)&siz, sizeof(siz), 0);//�����ļ�����
    siz = siz / 1024;
    std::string strPath = "FileCache/";
    strPath += fName;
    fp = fopen(const_cast<char*>(strPath.c_str()), "wb+");//�����ļ�
    long long int index = 0;
    int num;
    while (index <= siz){//�ֿ�����ļ�
        num = Receive(m_pBuffer, 1024, 0);
        if (num <= 0)
            break;
        fwrite(m_pBuffer, (int)num, 1, fp);
        index++;
    }
    fclose(fp);
    //֪ͨpresenter �յ��ļ���
    CString s(fName);
    m_callback.OnRecvFile(s);

    CAsyncSocket::OnReceive(nErrorCode);
}
void CFileSocket::OnSend(int nErrorCode){
    char fName[64] = { 0 };
    //���ļ�
    char name[32], ext[16];
    _splitpath(m_pBuffer, NULL, NULL, name, ext);
    strcat_s(name, 32, ext);
    strcpy_s(fName, 32, name);
    fp = fopen(m_pBuffer, "rb");
    //�����ļ�
    Send((char*)&fName, sizeof(fName), 0);//�����ļ���
    fseek(fp, 0, SEEK_END);
    long long siz = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    Send((CHAR*)&siz, sizeof(siz), 0);//�����ļ�����
    siz = siz / 1024;
    long long index = 0;
    int num;
    while (1) { // �ֿ鷢���ļ�
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
            AfxMessageBox(_T("Socket����ʧ�ܣ�"));
            m_ClientSocket->Close();
            PostQuitMessage(0);//�˳�����
            return;
        }
    }
    m_ClientSocket->Connect(strAddr, nPort);

    m_FileSocket = new CFileSocket(callback);
    if (m_FileSocket->m_hSocket == INVALID_SOCKET){
        if (!m_FileSocket->Create(0, SOCK_STREAM, FD_CONNECT)){
            AfxMessageBox(_T("Socket����ʧ�ܣ�"));
            m_FileSocket->Close();
            PostQuitMessage(0);//�˳�����
            return;
        }
    }
    m_FileSocket->Connect(strAddr, nPort+1);//���ӵ���һ���˿�
}
void ClientModel::sendMessage(CString strText){
    //��CString תΪchar*
    char* szMessage = new char[1024];
    CString2Char(strText, szMessage);
    int nLen = strlen(szMessage);

    //���ݸ�socket���ͣ����͵���
    m_ClientSocket->m_pBuffer = szMessage;
    m_ClientSocket->m_intLen = nLen;
    m_ClientSocket->AsyncSelect(FD_WRITE);//����ͨ���׽��ֵ�OnSend����
}
void ClientModel::sendFile(CString strFilePath){
    //��CString תΪchar*
    char* szFileName = new char[1024];
    CString2Char(strFilePath, szFileName);
    int nLen = strlen(szFileName);
    //�����ļ�������Ϣ
    m_FileSocket->m_pBuffer = szFileName;
    m_FileSocket->m_intLen = nLen;
    m_FileSocket->AsyncSelect(FD_WRITE);
}