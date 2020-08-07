#pragma once


// CDragEdit 用来实现文本框拖拽文件显示路径

class CDragEdit : public CEdit
{
	DECLARE_DYNAMIC(CDragEdit)

protected:
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnDropFiles(HDROP hDropInfo);
};


