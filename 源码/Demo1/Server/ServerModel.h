#ifndef MODEL_H
#define MODEL_H
#include "sqlite3.h"

class CallBack{
public:
    virtual ~CallBack(){};
    virtual void OnConnectSuccess() = 0;
    virtual void OnRecvMessage(CString) = 0;
    virtual void OnRecvFile(CString) = 0;
    virtual void OnConnectClose() = 0;
};

class CCommuSocket : public CAsyncSocket{
private:      
    virtual void OnReceive(int nErrorCode);
    virtual void OnSend(int nErrorCode);
public:
    char* m_pBuffer;
    int m_intLen;
    sqlite3* m_db = nullptr;
    CallBack &m_callback;   
    CCommuSocket(CallBack &);   
};

class CFileCommuSocket : public CCommuSocket{
private:
    FILE* fp;
    virtual void OnReceive(int nErrorCode);
    virtual void OnSend(int nErrorCode);
public:  
    CFileCommuSocket(CallBack&);    
};

class CListenSocket: public CAsyncSocket{
private:   
    virtual void OnAccept(int nErrorCode);
public:    
    CallBack &m_callback;
    CListenSocket(CallBack &);
    CCommuSocket* m_pSocket;    
};

class CFileListenSocket : public CListenSocket{
public:
    CFileCommuSocket *m_pFileSocket;
    CFileListenSocket(CallBack&);
    virtual void OnAccept(int nErrorCode);
};

class ServerModel{
private:
    CListenSocket* m_ListenSocket;
    CFileListenSocket* m_FileListenSocket;
public:  
    void startServer(int intPort, CallBack &callback);
    void sendMessage(CString);
    void sendFile(CString);
    CString showMessageLog();
};

#endif 