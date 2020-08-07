#ifndef MODEL_H
#define MODEL_H

class CallBack{
public:
    virtual ~CallBack(){};
    virtual void OnConnectSuccess() = 0;
    virtual void OnRecvMessage(CString) = 0;
    virtual void OnRecvFile(CString) = 0;
};


class CClientSocket:public CAsyncSocket{
private:
    virtual void OnConnect(int nErrorCode);
    virtual void OnReceive(int nErrorCode);
    virtual void OnSend(int nErrorCode);
public:
    CallBack& m_callback;
    char* m_pBuffer;
    int m_intLen;

    CClientSocket(CallBack&); 
};

class CFileSocket :public CClientSocket{
private:
    FILE* fp;
    virtual void OnConnect(int nErrorCode);
    virtual void OnReceive(int nErrorCode);
    virtual void OnSend(int nErrorCode);
public:    
    CFileSocket(CallBack&);   
};

class ClientModel{
private:
    CClientSocket* m_ClientSocket;
    CFileSocket* m_FileSocket;
public:
   
    void connectToServer(CString strAddr, int nPort, CallBack &callback);
    void sendMessage(CString);
    void sendFile(CString);
    void closeConnect();
};

#endif 