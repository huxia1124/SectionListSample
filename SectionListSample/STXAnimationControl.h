
#pragma once
#include "STXGraphics.h"
#include "framework.h"
#include <map>
#include <string>
#include <vector>
#include <memory>
#include <commctrl.h>
#include <queue>

#include <Richedit.h>
#include <Textserv.h>
#include <atlbase.h>

//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////

#define ANI_VAR(xVar, xKey)\
	CComPtr<IUIAnimationVariable> &xVar = _animationVariables[_T(#xKey)];

#define ANI_VAR_VALUE(xVar, xKey)\
	DOUBLE xVar = 0;\
				{\
	CComPtr<IUIAnimationVariable> &xxspVariant = _animationVariables[_T(#xKey)];\
	if(xxspVariant)\
		xxspVariant->GetValue(&xVar);\
				}

#define ANI_VAR_VALUE_DEFAULT(xVar, xKey, xDefault)\
	DOUBLE xVar = xDefault;\
																{\
	CComPtr<IUIAnimationVariable> &xxspVariant = _animationVariables[_T(#xKey)];\
	if(xxspVariant)\
		xxspVariant->GetValue(&xVar);\
																}

#define ANI_VAR_VALUE_INT(xVar, xKey)\
	INT xVar = 0;\
				{\
	DOUBLE xVarDouble = 0;\
	CComPtr<IUIAnimationVariable> &xxspVariant = _animationVariables[_T(#xKey)];\
	if(xxspVariant)\
		xxspVariant->GetValue(&xVarDouble);\
	xVar = static_cast<INT>(xVarDouble);\
				}

#define ANI_VAR_VALUE_INT_MUL(xVar, xKey, xMul)\
	INT xVar = 0;\
																{\
	DOUBLE xVarDouble = 0;\
	CComPtr<IUIAnimationVariable> &xxspVariant = _animationVariables[_T(#xKey)];\
	if(xxspVariant)\
		xxspVariant->GetValue(&xVarDouble);\
	xVar = static_cast<INT>(xVarDouble * xMul);\
																}



#define ANI_VAR_FINAL_VALUE(xVar, xKey)\
	DOUBLE xVar = 0;\
				{\
	CComPtr<IUIAnimationVariable> &xxspVariant = _animationVariables[_T(#xKey)];\
	if(xxspVariant)\
		xxspVariant->GetFinalValue(&xVar);\
				}

#define ANI_VAR_FINAL_VALUE_INT(xVar, xKey)\
	INT xVar = 0;\
				{\
	DOUBLE xVarDouble = 0;\
	CComPtr<IUIAnimationVariable> &xxspVariant = _animationVariables[_T(#xKey)];\
	if(xxspVariant)\
		xxspVariant->GetFinalValue(&xVarDouble);\
	xVar = static_cast<INT>(xVarDouble);\
				}

//////////////////////////////////////////////////////////////////////////

class CSTXAnimationControl;
class CSTXAnimationControlChildNode;

class CSTXAnimationLayout
{
public:
	virtual POINT OnQueryNodePosition(CSTXAnimationControlChildNode *pNode, int nIndex) = 0;
};

class CSTXAnimationObjectBase : public IUnknown
{
	friend class CSTXAnimationControlWindow;

public:
	CSTXAnimationObjectBase();
	virtual ~CSTXAnimationObjectBase();

protected:
#ifdef UNICODE
	std::map<std::wstring, CComPtr<IUIAnimationVariable> > _animationVariables;
#else
	std::map<std::string, CComPtr<IUIAnimationVariable> > _animationVariables;
#endif

protected:
	LONG m_nRef;

public:
	// IUnknown
	STDMETHOD_(ULONG, AddRef)(void);
	STDMETHOD_(ULONG, Release)(void);
	STDMETHOD(QueryInterface)(const IID &, void **);

public:
	IUIAnimationVariable* GetAnimationVariable(LPCTSTR lpszKey);
};

class  CSTXAnimationControlChildNode : public CSTXAnimationObjectBase
{
	friend class CSTXAnimationControlWindow;
public:
	CSTXAnimationControlChildNode();
	virtual ~CSTXAnimationControlChildNode();

protected:
	CSTXAnimationControlChildNode *_mouseEnterNode;
	CSTXAnimationControlChildNode *_mouseDownNode;
	BOOL _thisNodeMouseDown;
	CSTXAnimationControlChildNode *_mouseDownChildNode;
	BOOL _mouseDownNodeRButton;
	CSTXAnimationControlChildNode *_parentNode;
	CSTXAnimationControlWindow *_parentControl;
	int _order;
	std::vector<CSTXAnimationControlChildNode*> _childNodes;
	std::queue<CSTXAnimationControlChildNode*> _nodeToRemove;
	BOOL _visible;
	BOOL _deletePending;
	LONG _drawingCountAfterDelete;
	CComPtr<IUIAnimationStoryboard> _storyBoard;		//See BeginSetValue/EndSetValue
	long _pendingTransition;
	long *_pPendingTransition;
	long _nestedSetValue;
	POINT m_ptLButtonDown;
	CSTXAnimationControlChildNode *_mouseDraggingNode;
	CSTXAnimationControlChildNode *_focusNode;

protected:
	void ClearChildren();
	virtual void ClearPendingDeleteNodes();

protected:
	virtual void OnMouseMove(int x, int y, UINT nFlags);								//x,y are local coord
	virtual void OnLButtonDown(int x, int y, UINT nFlags, BOOL bForRButton = FALSE);
	virtual void OnLButtonUp(int x, int y, UINT nFlags, BOOL bForRButton = FALSE);
	virtual void OnMouseLeave();
	virtual void OnRButtonDown(int x, int y, UINT nFlags);
	virtual void OnRButtonUp(int x, int y, UINT nFlags);
	virtual void OnLButtonDblClk(int x, int y, UINT nFlags);
	virtual void OnSize(UINT nType, int cx, int cy);
	virtual void OnBeginMouseDragging(POINT ptGlobal);
	virtual void OnMouseDragging(POINT ptGlobal, POINT ptOffset);
	virtual void OnEndMouseDragging(POINT ptGlobal, POINT ptOffset);
	virtual BOOL OnSetCursor(HWND hWnd, int x, int y, UINT nHitTest, UINT message);

	virtual void OnMouseEnter();
	virtual void OnLButtonClick();
	virtual void OnRButtonClick();
	virtual void OnQueryToolTipsText(LPTSTR pszBuffer, int cchBufferSize, POINT ptLocation, LPCTSTR *ppszToolTips);
	virtual UI_ANIMATION_SECONDS OnQueryDefaultAnimationDuration();
	virtual LRESULT OnPreWndProc(int x, int y, HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual void OnAddedToNode(CSTXAnimationControlChildNode *pNodeParent);
	virtual void OnPositionChangeStart();

protected:
	virtual int OnQueryNodeHeight();
	virtual int OnQueryNodeFinalHeight();

protected:
	void SetCurrentFocusNode(CSTXAnimationControlChildNode *pNodeFocus);
	virtual void SetValueIntegerChangeEventHandler(LPCTSTR lpszKey, IUIAnimationVariableIntegerChangeHandler *pHandler);

public:
	virtual int AddChildNode(CSTXAnimationControlChildNode *pNode);
	virtual BOOL RemoveChildNode(int nIndex);
	BOOL RemoveChildNode(CSTXAnimationControlChildNode * pNode);
	virtual LONG GetMaxDrawingCountAfterDelete();
	virtual BOOL IsReadyToClear();
	virtual void DrawNode(CSTXGraphics *pGraphics);
	virtual void PostDrawNode(CSTXGraphics *pGraphics);
	virtual BOOL IsDelayRemove();		//return TRUE to move to a to-delete list
	virtual BOOL BeginSetValue(IUIAnimationStoryboard *pStory = NULL);
	virtual BOOL EndSetValue();
	IUIAnimationStoryboard* GetCurrentStoryboard();
	IUIAnimationStoryboard* GetAncestorCurrentStoryboard();
	void SetValue(LPCTSTR lpszKey, DOUBLE fValue);
	void SetValue(LPCTSTR lpszKey, DOUBLE fValue, DOUBLE fDuration);
	void SetValueInstantly(LPCTSTR lpszKey, DOUBLE fValue);
	DOUBLE GetValue(LPCTSTR lpszKey);
	BOOL IsValueExists(LPCTSTR lpszKey);
	DOUBLE GetFinalValue(LPCTSTR lpszKey);
	int GetFinalValueInt(LPCTSTR lpszKey);
	POINT ConvertToNodeLocalPoint(POINT ptGlobal, BOOL *pbInNode = NULL);
	POINT ConvertToGlobalPoint(POINT ptLocal);
	size_t GetChildrenCount();
	CSTXAnimationControlChildNode* GetChildNodeAtIndex(int nIndex);
	CSTXAnimationControlWindow* GetParentControl();
	virtual DOUBLE GetAncestorOpacity();
	virtual byte GetAncestorOpacityByte();
	virtual DOUBLE GetAncestorScale();
	virtual int GetIndexInParentNode();
	CSTXAnimationControlChildNode* GetParentNode();
	CSTXAnimationControlChildNode* GetAncestorNode();
	virtual void LayoutChildren(CSTXAnimationLayout *pLayout);
	virtual void GetBoundsRect(LPRECT lpRect);
	virtual void GetFinalBoundsRect(LPRECT lpRect);
};

class CSTXAnimationControlWindow : public CSTXAnimationControlChildNode, public IUIAnimationManagerEventHandler
{
	friend class CSTXAnimationControlChildNode;
	friend class CSTXAnimationControlEdit;
public:
	struct AnimationControlStyle
	{
		unsigned int drawNodesInViewOnly : 1;
		unsigned int autoCalculateContentSize : 1;
	};

public:
	CSTXAnimationControlWindow();
	~CSTXAnimationControlWindow();

protected:
	CComPtr<IUIAnimationManager> _animationManager;
	CComPtr<IUIAnimationTimer> _animationTimer;
	CComPtr<IUIAnimationTransitionLibrary> _animationTransitionLibrary;

protected:
	HWND _hwndControl;
	HWND _hwndToolTips;
	AnimationControlStyle _style;
	BOOL _trackingMouse;
	TOOLINFO g_toolItem;
	UINT _graphicsCacheId;

protected:
	static LRESULT CALLBACK STXAnimatedControlWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:
	void OnPaint(HDC hDC);
	void OnTimer(UINT nIDEvent);
	void OnHScroll(UINT nSBCode, UINT nPos, HWND hWndScrollBar);
	void OnVScroll(UINT nSBCode, UINT nPos, HWND hWndScrollBar);
	virtual void OnMouseMove(int x, int y, UINT nFlags);
	virtual void OnLButtonDown(int x, int y, UINT nFlags, BOOL bForRButton = FALSE);
	virtual void OnLButtonUp(int x, int y, UINT nFlags, BOOL bForRButton = FALSE);
	virtual void OnRButtonDown(int x, int y, UINT nFlags);
	virtual void OnRButtonUp(int x, int y, UINT nFlags);
	virtual void OnLButtonDblClk(int x, int y, UINT nFlags);
	virtual void OnMouseWheel(UINT nFlags, short zDelta, int x, int y);
	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	void OnKeyDown_Left();
	void OnKeyDown_Right();
	virtual void OnMouseLeave();
	void OnSetFocus(HWND hWndOldFocus);
	void OnKillFocus(HWND hWndNewFocus);
	virtual void OnDestroy();
	BOOL OnSetCursor(HWND hWnd, UINT nHitTest, UINT message);
	UINT OnGetDlgCode();
	LRESULT OnGetObject(DWORD dwFlags, DWORD dwObjId);
	virtual void OnSize(UINT nType, int cx, int cy);
	virtual void OnEndEdit();
	virtual LRESULT OnNotify(UINT nID, NMHDR *pNMHDR);
	virtual BOOL OnEraseBackground(HDC hDC);
	virtual void OnQueryToolTipsText(LPTSTR pszBuffer, int cchBufferSize, POINT ptLocation, LPCTSTR *ppszToolTips);
	virtual void OnWindowCreated();

protected:
	virtual void DrawControl(CSTXGraphics *pGraphics);
	virtual void PostDrawControl(CSTXGraphics *pGraphics);
	virtual void DrawBackground(CSTXGraphics *pGraphics);
	virtual void UpdateAnimationManager();
	virtual void StartAnimationTimer();
	virtual void StopAnimationTimer();

protected:
	virtual HRESULT OnManagerStatusChanged(UI_ANIMATION_MANAGER_STATUS newStatus, UI_ANIMATION_MANAGER_STATUS previousStatus);

protected:
	void ResetScrollBars();
	void ResetHorizontalScrollBar();


public:

	//Override this method to provide a class name for your control
	virtual LPCTSTR GetControlClassName();
	virtual BOOL IsSystemControl();

	// Register the window class for this control.
	static void RegisterAnimationControlClass(LPCTSTR lpszClassName);

	//Create window.
	//Be sure to call RegisterAnimatedTreeCtrlClass before create window
	BOOL Create(LPCTSTR lpszWindowText, DWORD dwStyle, int x, int y, int cx, int cy, HWND hWndParent, UINT nID);

	HWND GetSafeHwnd();

	void ModifyStyle(DWORD dwRemove, DWORD dwAdd);
	BOOL ModifyStyle(int nStyleOffset, DWORD dwRemove, DWORD dwAdd, UINT nFlags);

	int GetScrollLimit(int nBar);
	virtual void InitializeChildNode(CSTXAnimationControlChildNode *pNode, std::map<std::wstring, DOUBLE> *initialValues = NULL);

	virtual int AddChildNode(CSTXAnimationControlChildNode *pNode);
	void RedrawWindow();

};

class CSTXAnimationControlEdit : public CSTXAnimationControlChildNode, public IUIAnimationVariableIntegerChangeHandler
	, public ITextHost
{
public:
	CSTXAnimationControlEdit();
	virtual ~CSTXAnimationControlEdit();

public:
	// IUnknown
	STDMETHOD_(ULONG, AddRef)(void);
	STDMETHOD_(ULONG, Release)(void);
	STDMETHOD(QueryInterface)(const IID &, void **);

protected:
	LONG m_nRef;
	WNDPROC _oldWndProc;
	CHARFORMAT2 _cf;
	PARAFORMAT _pf;
	IID _iidTextHost;
	CComPtr<ITextServices> _textService;
	SIZEL _sizelExtent;
	HINSTANCE _msfteditDLL;
	DWORD _textMaxLength;
	std::wstring _tipText;


protected:
	virtual void DrawNode(CSTXGraphics *pGraphics);

protected:


protected:

	virtual BOOL OnEraseBackground(HDC hDC);
	virtual BOOL OnSetCursor(HWND hWnd, int x, int y, UINT nHitTest, UINT message);
	virtual void OnMouseMove(int x, int y, UINT nFlags);
	virtual void OnLButtonDown(int x, int y, UINT nFlags, BOOL bForRButton = FALSE);
	virtual void OnLButtonUp(int x, int y, UINT nFlags, BOOL bForRButton = FALSE);
	virtual LRESULT OnPreWndProc(int x, int y, HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual void OnAddedToNode(CSTXAnimationControlChildNode *pNodeParent);

protected:

	virtual HRESULT OnIntegerValueChanged(
		IUIAnimationStoryboard * /*storyboard*/,
		IUIAnimationVariable * /*variable*/,
		INT32 newValue,
		INT32 previousValue
		);

	virtual HDC TxGetDC() override;
	virtual INT TxReleaseDC(HDC hdc) override;
	virtual BOOL TxShowScrollBar(INT fnBar, BOOL fShow) override;
	virtual BOOL TxEnableScrollBar(INT fuSBFlags, INT fuArrowflags) override;
	virtual BOOL TxSetScrollRange(INT fnBar, LONG nMinPos, INT nMaxPos, BOOL fRedraw) override;
	virtual BOOL TxSetScrollPos(INT fnBar, INT nPos, BOOL fRedraw) override;
	virtual void TxInvalidateRect(LPCRECT prc, BOOL fMode) override;
	virtual void TxViewChange(BOOL fUpdate) override;
	virtual BOOL TxCreateCaret(HBITMAP hbmp, INT xWidth, INT yHeight) override;
	virtual BOOL TxShowCaret(BOOL fShow) override;
	virtual BOOL TxSetCaretPos(INT x, INT y) override;
	virtual BOOL TxSetTimer(UINT idTimer, UINT uTimeout) override;
	virtual void TxKillTimer(UINT idTimer) override;
	virtual void TxScrollWindowEx(INT dx, INT dy, LPCRECT lprcScroll, LPCRECT lprcClip, HRGN hrgnUpdate, LPRECT lprcUpdate, UINT fuScroll) override;
	virtual void TxSetCapture(BOOL fCapture) override;
	virtual HRESULT TxGetPasswordChar(TCHAR *pch) override;
	virtual void TxSetFocus() override;
	virtual void TxSetCursor(HCURSOR hcur, BOOL fText) override;
	virtual BOOL TxScreenToClient(LPPOINT lppt) override;
	virtual BOOL TxClientToScreen(LPPOINT lppt) override;
	virtual HRESULT TxActivate(LONG * plOldState) override;
	virtual HRESULT TxDeactivate(LONG lNewState) override;
	virtual HRESULT TxGetClientRect(LPRECT prc) override;
	virtual HRESULT TxGetViewInset(LPRECT prc) override;
	virtual HRESULT TxGetCharFormat(const CHARFORMATW **ppCF) override;
	virtual HRESULT TxGetParaFormat(const PARAFORMAT **ppPF) override;
	virtual COLORREF TxGetSysColor(int nIndex) override;
	virtual HRESULT TxGetBackStyle(TXTBACKSTYLE *pstyle) override;
	virtual HRESULT TxGetMaxLength(DWORD *plength) override;
	virtual HRESULT TxGetScrollBars(DWORD *pdwScrollBar) override;
	virtual HRESULT TxGetAcceleratorPos(LONG *pcp) override;
	virtual HRESULT TxGetExtent(LPSIZEL lpExtent) override;
	virtual HRESULT OnTxCharFormatChange(const CHARFORMATW * pCF) override;
	virtual HRESULT OnTxParaFormatChange(const PARAFORMAT * pPF) override;
	virtual HRESULT TxGetPropertyBits(DWORD dwMask, DWORD *pdwBits) override;
	virtual HRESULT TxNotify(DWORD iNotify, void *pv) override;
	virtual HIMC TxImmGetContext() override;
	virtual void TxImmReleaseContext(HIMC himc) override;
	virtual HRESULT TxGetSelectionBarWidth(LONG *lSelBarWidth) override;

private:

	HRESULT InitDefaultCharFormat(CHARFORMATW * pcf, HFONT hfont);
	HRESULT InitDefaultParaFormat(PARAFORMAT * ppf);

public:
	void SetMaxLength(DWORD dwMaxLength);
	DWORD GetTextLength();
	void GetText(LPTSTR pBuffer, UINT cchBufferLength);
	void SetText(LPCTSTR lpszText);
	void SetTipText(LPCTSTR lpszTipText);
};

class CSTXAnimationControl : public CSTXAnimationControlWindow
{
	friend class  CSTXAnimationControlChildNode;
public:
	CSTXAnimationControl();
	virtual ~CSTXAnimationControl();

//protected:
//	IUIAnimationManager _animationManager;
//	IUIAnimationTimer _animationTimer;
//	IUIAnimationTransitionLibrary _animationTransitionLibrary;

//protected:
//	UI_ANIMATION_SECONDS _defaultAnimationDuration;

protected:
	//virtual void UpdateAnimationManager();
	//virtual HRESULT OnManagerStatusChanged(UI_ANIMATION_MANAGER_STATUS newStatus, UI_ANIMATION_MANAGER_STATUS previousStatus);

protected:

protected:
	//void ResetScrollBars();
	//void ResetHorizontalScrollBar();
	//virtual void ClearPendingDeleteNodes();


public:
	//virtual int AddChildNode(CSTXAnimationControlChildNode *pNode);
	//void InitializeChildNode(CSTXAnimationControlChildNode *pNode, std::map<std::wstring, DOUBLE> *initialValues = NULL);
};