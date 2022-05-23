#include "STXAnimationControl.h"
#include <commctrl.h>
#include <windowsx.h>

#pragma comment(lib,"imm32.lib")

//////////////////////////////////////////////////////////////////////////

DEFINE_GUID(IID_ITextServices2, 0x8D33F741, 0xCF58, 0x11CE, 0xA8, 0x9D, 0x00, 0xAA, 0x00, 0x6C, 0xAD, 0xC5);

EXTERN_C const GUID DECLSPEC_SELECTANY IID_ITextServices2 = { 0x8D33F741, 0xCF58, 0x11CE, { 0xA8, 0x9D, 0x00, 0xAA, 0x00, 0x6C, 0xAD, 0xC5 } };

//////////////////////////////////////////////////////////////////////////

#define  STXANIMATIONCONTROL_TIMER_ID_ANIMATION 14547

//////////////////////////////////////////////////////////////////////////

#ifndef GET_X_LPARAM
#define GET_X_LPARAM(lp)                        ((int)(short)LOWORD(lp))
#endif

#ifndef GET_Y_LPARAM
#define GET_Y_LPARAM(lp)                        ((int)(short)HIWORD(lp))
#endif

//////////////////////////////////////////////////////////////////////////

CSTXAnimationObjectBase::CSTXAnimationObjectBase()
{
	m_nRef = 1;
}

STDMETHODIMP CSTXAnimationObjectBase::QueryInterface(const IID & iid, void **ppv)
{
	return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CSTXAnimationObjectBase::Release(void)
{
	ULONG l;
	l = InterlockedDecrement(&m_nRef);
	if (0 == l)
		delete this;
	return l;
}

STDMETHODIMP_(ULONG) CSTXAnimationObjectBase::AddRef(void)
{
	return InterlockedIncrement(&m_nRef);
}

CSTXAnimationObjectBase::~CSTXAnimationObjectBase()
{

}

IUIAnimationVariable* CSTXAnimationObjectBase::GetAnimationVariable(LPCTSTR lpszKey)
{
	std::map<std::wstring, CComPtr<IUIAnimationVariable> >::iterator it = _animationVariables.find(lpszKey);
	if (it == _animationVariables.end())
		return NULL;

	return it->second;
}

//////////////////////////////////////////////////////////////////////////


CSTXAnimationControlChildNode::CSTXAnimationControlChildNode()
{
	_parentControl = NULL;
	_parentNode = NULL;
	_mouseDownNode = NULL;
	_mouseEnterNode = NULL;
	_visible = TRUE;
	_drawingCountAfterDelete = 0;
	_deletePending = FALSE;
	_mouseDownNodeRButton = FALSE;
	_pendingTransition = 0;
	_pPendingTransition = &_pendingTransition;
	_nestedSetValue = 0;
	_mouseDraggingNode = NULL;
	m_ptLButtonDown.x = m_ptLButtonDown.y = 0;
	_mouseDownChildNode = NULL;
	_thisNodeMouseDown = FALSE;
	_focusNode = NULL;
}

CSTXAnimationControlChildNode::~CSTXAnimationControlChildNode()
{
	ClearChildren();
}

void CSTXAnimationControlChildNode::DrawNode(CSTXGraphics *pGraphics)
{
	if (_deletePending)
	{
		_drawingCountAfterDelete++;
	}

	ANI_VAR_VALUE(x, x);
	ANI_VAR_VALUE(y, y);
	//ANI_VAR_VALUE(w, width);
	//ANI_VAR_VALUE(h, height);

	////CSTXGraphicsBrush *pBrush = pGraphics->CreateSolidBrush(255, 0, 0, 255);
	//CSTXGraphicsBrush *pBrush = pGraphics->CreateSimpleLinearGradientBrush(x,y, 255, 0, 0, 255, x+w, y+h, 255,255,255,255);

	//pGraphics->FillRectangle(x, y, w, h, pBrush);

	//delete pBrush;

	CSTXAnimationControlWindow *pWnd = dynamic_cast<CSTXAnimationControlWindow*>(_parentControl);
	if (pWnd)
	{
		if (pWnd->_style.drawNodesInViewOnly)
		{
			int iVScrollPos = GetScrollPos(pWnd->_hwndControl, SB_VERT);

			RECT rcClient;
			GetClientRect(pWnd->_hwndControl, &rcClient);
			int viewHeight = rcClient.bottom - rcClient.top;

			//Draw removing nodes
			std::queue<CSTXAnimationControlChildNode*> nodeToDraw;		
			size_t nDrawDelete = _nodeToRemove.size();
			for (size_t i = 0; i < nDrawDelete; i++)
			{
				CSTXAnimationControlChildNode *pItem = _nodeToRemove.front();
				_nodeToRemove.pop();

				if (!pItem->IsReadyToClear())
				{
					_nodeToRemove.push(pItem);
					nodeToDraw.push(pItem);
				}
			}

			//Draw alive nodes
			std::vector<CSTXAnimationControlChildNode*>::iterator it = _childNodes.begin();
			for (; it != _childNodes.end(); it++)
			{
				nodeToDraw.push(*it);
			}

			// Set the transformation matrix of the Graphics object.
			pGraphics->TranslateTransform(static_cast<int>(x), static_cast<int>(y), 0);
			int nDraw = 0;
			while (nodeToDraw.size())
			{
				CSTXAnimationControlChildNode *pItem = nodeToDraw.front();
				nodeToDraw.pop();


				DOUBLE y = pItem->GetValue(_T("y"));
				DOUBLE h = pItem->GetValue(_T("height"));
				POINT ptLocal = { 0, static_cast<int>(y) };
				POINT ptGlobal = ConvertToGlobalPoint(ptLocal);

				if (ptGlobal.y + h < iVScrollPos || ptGlobal.y - iVScrollPos > viewHeight)
					continue;

				pItem->DrawNode(pGraphics);
				nDraw++;
			}
			pGraphics->TranslateTransform(static_cast<int>(-x), static_cast<int>(-y), 0);

			return;
		}
	}


	// Set the transformation matrix of the Graphics object.
	//Draw removing nodes
	std::queue<CSTXAnimationControlChildNode*> nodeToDraw;

	size_t nDrawDelete = _nodeToRemove.size();
	for (size_t i = 0; i < nDrawDelete; i++)
	{
		CSTXAnimationControlChildNode *pItem = _nodeToRemove.front();
		_nodeToRemove.pop();

		if (!pItem->IsReadyToClear())
		{
			_nodeToRemove.push(pItem);
			nodeToDraw.push(pItem);
		}
	}

	pGraphics->TranslateTransform(static_cast<int>(x), static_cast<int>(y), 0);
	int nDraw = 0;
	while (nodeToDraw.size())
	{
		CSTXAnimationControlChildNode *pItem = nodeToDraw.front();
		nodeToDraw.pop();
		pItem->DrawNode(pGraphics);
		nDraw++;
	}
	pGraphics->TranslateTransform(static_cast<int>(-x), static_cast<int>(-y), 0);

}

int CSTXAnimationControlChildNode::AddChildNode(CSTXAnimationControlChildNode *pNode)
{
	if (pNode == NULL)
		return -1;

	pNode->_parentNode = this;
	pNode->_parentControl = _parentControl;
	_childNodes.push_back(pNode);
	pNode->AddRef();
	pNode->OnAddedToNode(this);
	return static_cast<int>(_childNodes.size() - 1);
}

void CSTXAnimationControlChildNode::SetValue(LPCTSTR lpszKey, DOUBLE fValue)
{
	SetValue(lpszKey, fValue, OnQueryDefaultAnimationDuration());
}

void CSTXAnimationControlChildNode::SetValue(LPCTSTR lpszKey, DOUBLE fValue, DOUBLE fDuration)
{
	std::map<std::wstring, CComPtr<IUIAnimationVariable> >::iterator it = _animationVariables.find(lpszKey);
	if (it != _animationVariables.end())
	{
		CComPtr<IUIAnimationStoryboard> pStory;

		if (_storyBoard)
			pStory = _storyBoard;
		else
			_parentControl->_animationManager->CreateStoryboard(&pStory);

		CComPtr<IUIAnimationTransition> pTrans;
		_parentControl->_animationTransitionLibrary->CreateSmoothStopTransition(fDuration, fValue, &pTrans);
		pStory->AddTransition(it->second, pTrans);

		if (_storyBoard)
		{
			InterlockedIncrement(_pPendingTransition);
		}
		else
		{
			UI_ANIMATION_SECONDS secTime;
			_parentControl->_animationTimer->GetTime(&secTime);
			pStory->Schedule(secTime, NULL);
		}
	}

	if (_tcscmp(lpszKey, _T("x")) == 0 || _tcscmp(lpszKey, _T("y")) == 0)
	{
		OnPositionChangeStart();
	}
}

void CSTXAnimationControlChildNode::SetValueInstantly(LPCTSTR lpszKey, DOUBLE fValue)
{
	std::map<std::wstring, CComPtr<IUIAnimationVariable> >::iterator it = _animationVariables.find(lpszKey);
	if (it != _animationVariables.end())
	{
		CComPtr<IUIAnimationStoryboard> pStory;

		if (_storyBoard)
			pStory = _storyBoard;
		else
			_parentControl->_animationManager->CreateStoryboard(&pStory);

		CComPtr<IUIAnimationTransition> pTrans;
		_parentControl->_animationTransitionLibrary->CreateInstantaneousTransition(fValue, &pTrans);
		pStory->AddTransition(it->second, pTrans);

		if (_storyBoard)
		{
			InterlockedIncrement(_pPendingTransition);
		}
		else
		{
			UI_ANIMATION_SECONDS secTime;
			_parentControl->_animationTimer->GetTime(&secTime);
			pStory->Schedule(secTime, NULL);
		}
	}
}

POINT CSTXAnimationControlChildNode::ConvertToNodeLocalPoint(POINT ptGlobal, BOOL *pbInNode)
{
	POINT ptResult = ptGlobal;
	if (_parentNode)
	{
		ptResult = _parentNode->ConvertToNodeLocalPoint(ptGlobal, pbInNode);
		ANI_VAR_VALUE(x, x);
		ANI_VAR_VALUE(y, y);

		ptResult.x -= static_cast<int>(x);
		ptResult.y -= static_cast<int>(y);

		if (pbInNode)
		{
			if (ptResult.x < 0 || ptResult.y < 0)
			{
				*pbInNode = FALSE;
			}
			else
			{
				ANI_VAR_VALUE(w, width);
				ANI_VAR_VALUE(h, height);

				if (ptResult.x > static_cast<int>(w) || ptResult.y > static_cast<int>(h))
				{
					*pbInNode = FALSE;
				}
				else
				{
					*pbInNode = TRUE;
				}
			}
		}

		return ptResult;
	}
	else
	{
		if (pbInNode)
			*pbInNode = TRUE;
		return ptGlobal;
	}
}

POINT CSTXAnimationControlChildNode::ConvertToGlobalPoint(POINT ptLocal)
{
	DOUBLE fParentY = GetValue(_T("y"));
	ptLocal.y += static_cast<int>(fParentY);
	if (_parentNode)
		return _parentNode->ConvertToGlobalPoint(ptLocal);

	return ptLocal;
}


void CSTXAnimationControlChildNode::OnMouseMove(int x, int y, UINT nFlags)
{
	POINT pt = { x, y };

	if (_mouseDraggingNode)
	{
		POINT ptOffset = { x - m_ptLButtonDown.x, y - m_ptLButtonDown.y };
		_mouseDraggingNode->OnMouseDragging(pt, ptOffset);
		return;
	}

	std::vector<CSTXAnimationControlChildNode*>::iterator it = _childNodes.begin();
	std::queue<CSTXAnimationControlChildNode*> queueNodes;

	for (; it != _childNodes.end(); it++)
	{
		(*it)->AddRef();
		queueNodes.push(*it);
	}

	BOOL bInAnyChild = FALSE;

	while (queueNodes.size())
	{
		CSTXAnimationControlChildNode *pNode = queueNodes.front();
		queueNodes.pop();

		BOOL bInNode = FALSE;
		POINT ptLocal = pNode->ConvertToNodeLocalPoint(pt, &bInNode);
		if (bInNode)
		{
			bInAnyChild = TRUE;
			if (_mouseEnterNode != pNode)
			{
				if (_mouseEnterNode)
				{
					_mouseEnterNode->OnMouseLeave();
					_mouseEnterNode->Release();
					_mouseEnterNode = NULL;
				}
				_mouseEnterNode = pNode;
				_mouseEnterNode->AddRef();
				_mouseEnterNode->OnMouseEnter();
			}
			pNode->OnMouseMove(pt.x, pt.y, nFlags);
		}

		pNode->Release();
	}

	if (!bInAnyChild && !_thisNodeMouseDown)
	{
		if (_mouseEnterNode)
		{
			_mouseEnterNode->OnMouseLeave();
			_mouseEnterNode->Release();
			_mouseEnterNode = NULL;
		}
	}

	if (!bInAnyChild && _thisNodeMouseDown && _mouseDownChildNode == NULL && _mouseDraggingNode == NULL && (abs(m_ptLButtonDown.y - y) > 5 || abs(m_ptLButtonDown.x - x) > 5))
	{
		_mouseDraggingNode = this;
		_mouseDraggingNode->AddRef();
		_mouseDraggingNode->OnBeginMouseDragging(pt);
	}
}

DOUBLE CSTXAnimationControlChildNode::GetValue(LPCTSTR lpszKey)
{
	DOUBLE fValue = 0.0f;
	std::map<std::wstring, CComPtr<IUIAnimationVariable> >::iterator it = _animationVariables.find(lpszKey);
	if (it != _animationVariables.end())
	{
		it->second->GetValue(&fValue);
	}

	return fValue;
}

DOUBLE CSTXAnimationControlChildNode::GetFinalValue(LPCTSTR lpszKey)
{
	DOUBLE fValue = 0.0f;
	std::map<std::wstring, CComPtr<IUIAnimationVariable> >::iterator it = _animationVariables.find(lpszKey);
	if (it != _animationVariables.end())
	{
		it->second->GetFinalValue(&fValue);
	}

	return fValue;
}

int CSTXAnimationControlChildNode::GetFinalValueInt(LPCTSTR lpszKey)
{
	return static_cast<int>(GetFinalValue(lpszKey));
}

size_t CSTXAnimationControlChildNode::GetChildrenCount()
{
	return _childNodes.size();
}

void CSTXAnimationControlChildNode::OnLButtonDown(int x, int y, UINT nFlags, BOOL bForRButton /*= FALSE*/)
{
	_thisNodeMouseDown = TRUE;

	std::vector<CSTXAnimationControlChildNode*>::iterator it = _childNodes.begin();
	std::queue<CSTXAnimationControlChildNode*> queueNodes;

	m_ptLButtonDown.x = x;
	m_ptLButtonDown.y = y;

	for (; it != _childNodes.end(); it++)
	{
		(*it)->AddRef();
		queueNodes.push(*it);
	}

	while (queueNodes.size())
	{
		CSTXAnimationControlChildNode *pNode = queueNodes.front();
		queueNodes.pop();

		POINT pt = { x, y };
		BOOL bInNode = FALSE;
		POINT ptLocal = pNode->ConvertToNodeLocalPoint(pt, &bInNode);
		if (bInNode)
		{
			_mouseDownChildNode = pNode;
			if (_mouseDownNode)
			{
				_mouseDownNode->Release();
				_mouseDownNode = NULL;
			}
			_mouseDownNode = pNode;
			_mouseDownNode->AddRef();
			_mouseDownNodeRButton = bForRButton;
			pNode->OnLButtonDown(pt.x, pt.y, nFlags, bForRButton);
		}
		pNode->Release();
	}
}

void CSTXAnimationControlChildNode::OnLButtonUp(int x, int y, UINT nFlags, BOOL bForRButton /*= FALSE*/)
{
	_thisNodeMouseDown = FALSE;
	_mouseDownChildNode = NULL;

	POINT pt = { x, y };
	CSTXAnimationControlChildNode *pOldMouseDownNode = _mouseDownNode;
	if (_mouseDownNode)
	{
		_mouseDownNode->OnLButtonUp(x, y, nFlags, bForRButton);
		_mouseDownNode->Release();
	}
	_mouseDownNode = NULL;

	BOOL bInThisNode = FALSE;
	ConvertToNodeLocalPoint(pt, &bInThisNode);

	if (!bInThisNode && !_thisNodeMouseDown)
	{
		if (_mouseEnterNode)
		{
			_mouseEnterNode->OnMouseLeave();
			_mouseEnterNode->Release();
			_mouseEnterNode = NULL;
		}
	}

	if (_mouseDraggingNode)
	{
		POINT ptOffset = { x - m_ptLButtonDown.x, y - m_ptLButtonDown.y };
		_mouseDraggingNode->OnEndMouseDragging(pt, ptOffset);
		_mouseDraggingNode->Release();
		_mouseDraggingNode = NULL;
	}

	std::vector<CSTXAnimationControlChildNode*>::iterator it = _childNodes.begin();
	std::queue<CSTXAnimationControlChildNode*> queueNodes;

	for (; it != _childNodes.end(); it++)
	{
		(*it)->AddRef();
		queueNodes.push(*it);
	}

	while (queueNodes.size())
	{
		CSTXAnimationControlChildNode *pNode = queueNodes.front();
		queueNodes.pop();

		BOOL bInNode = FALSE;
		POINT ptLocal = pNode->ConvertToNodeLocalPoint(pt, &bInNode);
		if (bInNode)
		{
			pNode->OnLButtonUp(pt.x, pt.y, nFlags, bForRButton);
			if (pOldMouseDownNode == pNode)
			{
				if (!bForRButton)
				{
					pNode->OnLButtonClick();
				}
				else
				{
					pNode->OnRButtonClick();
				}
			}
		}

		pNode->Release();
	}
}

void CSTXAnimationControlChildNode::OnMouseLeave()
{
	if (_mouseDownNode)
	{
		POINT ptGlobal;
		GetCursorPos(&ptGlobal);
		ScreenToClient(_parentControl->GetSafeHwnd(), &ptGlobal);
		POINT ptLocal = _mouseDownNode->ConvertToNodeLocalPoint(ptGlobal);
		_mouseDownNode->OnLButtonUp(ptLocal.x, ptLocal.y, 0, _mouseDownNodeRButton);
		_mouseDownNode->Release();
		_mouseDownNode = NULL;
	}

	if (_mouseEnterNode)
	{
		_mouseEnterNode->OnMouseLeave();
		_mouseEnterNode->Release();
	}
	_mouseEnterNode = NULL;

	::SendMessage(_parentControl->_hwndToolTips, TTM_ACTIVATE, FALSE, 0);
	::SendMessage(_parentControl->_hwndToolTips, TTM_ACTIVATE, TRUE, 0);
}

void CSTXAnimationControlChildNode::OnLButtonClick()
{

}

void CSTXAnimationControlChildNode::OnRButtonClick()
{

}

CSTXAnimationControlChildNode* CSTXAnimationControlChildNode::GetChildNodeAtIndex(int nIndex)
{
	return _childNodes[nIndex];
}

void CSTXAnimationControlChildNode::OnMouseEnter()
{
	::SendMessage(_parentControl->_hwndToolTips, TTM_ACTIVATE, FALSE, 0);
	::SendMessage(_parentControl->_hwndToolTips, TTM_ACTIVATE, TRUE, 0);
}

void CSTXAnimationControlChildNode::OnRButtonDown(int x, int y, UINT nFlags)
{
	OnLButtonDown(x, y, nFlags, TRUE);
}

void CSTXAnimationControlChildNode::OnRButtonUp(int x, int y, UINT nFlags)
{
	OnLButtonUp(x, y, nFlags, TRUE);
}

void CSTXAnimationControlChildNode::OnSize(UINT nType, int cx, int cy)
{

}

void CSTXAnimationControlChildNode::OnLButtonDblClk(int x, int y, UINT nFlags)
{
	std::vector<CSTXAnimationControlChildNode*>::iterator it = _childNodes.begin();
	std::queue<CSTXAnimationControlChildNode*> queueNodes;

	for (; it != _childNodes.end(); it++)
	{
		(*it)->AddRef();
		queueNodes.push(*it);
	}

	while (queueNodes.size())
	{
		CSTXAnimationControlChildNode *pNode = queueNodes.front();
		queueNodes.pop();

		POINT pt = { x, y };
		BOOL bInNode = FALSE;
		POINT ptLocal = pNode->ConvertToNodeLocalPoint(pt, &bInNode);
		if (bInNode)
		{
			pNode->OnLButtonDblClk(pt.x, pt.y, nFlags);
		}

		pNode->Release();
	}

}

int CSTXAnimationControlChildNode::OnQueryNodeHeight()
{
	ANI_VAR_VALUE(h, height);
	return static_cast<int>(h);
}

int CSTXAnimationControlChildNode::OnQueryNodeFinalHeight()
{
	ANI_VAR_FINAL_VALUE(h, height);
	return static_cast<int>(h);
}

CSTXAnimationControlWindow* CSTXAnimationControlChildNode::GetParentControl()
{
	return _parentControl;
}

void CSTXAnimationControlChildNode::OnQueryToolTipsText(LPTSTR pszBuffer, int cchBufferSize, POINT ptLocation, LPCTSTR *ppszToolTips)
{
	if (_mouseEnterNode)
	{
		*ppszToolTips = NULL;
		_mouseEnterNode->OnQueryToolTipsText(pszBuffer, cchBufferSize, ptLocation, ppszToolTips);
	}
}

void CSTXAnimationControlChildNode::ClearChildren()
{
	std::vector<CSTXAnimationControlChildNode*>::iterator it = _childNodes.begin();
	for (; it != _childNodes.end(); it++)
	{
		(*it)->ClearChildren();
		(*it)->Release();
	}
	_childNodes.clear();

	while (_nodeToRemove.size())
	{
		CSTXAnimationControlChildNode *pNode = _nodeToRemove.front();
		_nodeToRemove.pop();

		pNode->ClearChildren();
		pNode->Release();
	}
}

BOOL CSTXAnimationControlChildNode::RemoveChildNode(int nIndex)
{
	if (nIndex < 0 || nIndex >= static_cast<int>(_childNodes.size()))
		return FALSE;

	CSTXAnimationControlChildNode *pNodeToRemove = _childNodes[nIndex];
	pNodeToRemove->_deletePending = TRUE;
	if (pNodeToRemove == _focusNode)
		_focusNode = NULL;

	if (pNodeToRemove->IsDelayRemove())
	{
		_nodeToRemove.push(pNodeToRemove);
	}
	else
	{
		_childNodes[nIndex]->Release();
	}
	_childNodes.erase(_childNodes.begin() + nIndex);
	return TRUE;
}

BOOL CSTXAnimationControlChildNode::RemoveChildNode(CSTXAnimationControlChildNode * pNode)
{
	int nIndex = 0;
	std::vector<CSTXAnimationControlChildNode*>::iterator it = _childNodes.begin();
	for (; it != _childNodes.end(); it++)
	{
		if (*it == pNode)
		{
			RemoveChildNode(nIndex);
			return TRUE;
		}
		nIndex++;
	}
	return FALSE;
}

BOOL CSTXAnimationControlChildNode::IsDelayRemove()
{
	return FALSE;
}

BOOL CSTXAnimationControlChildNode::IsReadyToClear()
{
	if (_deletePending && _drawingCountAfterDelete > GetMaxDrawingCountAfterDelete())
		return TRUE;

	//ANI_VAR_VALUE_DEFAULT(opacity, opacity, -1000);
	//if (static_cast<int>(opacity) == -1000)
	//{
	//	return TRUE;
	//}

	//return opacity < 0.02;

	return FALSE;
}

LONG CSTXAnimationControlChildNode::GetMaxDrawingCountAfterDelete()
{
	return 1000;
}

void CSTXAnimationControlChildNode::ClearPendingDeleteNodes()
{
	std::vector<CSTXAnimationControlChildNode*>::iterator it = _childNodes.begin();
	for (; it != _childNodes.end(); it++)
	{
		(*it)->ClearPendingDeleteNodes();
	}

	while (_nodeToRemove.size())
	{
		CSTXAnimationControlChildNode *pNode = _nodeToRemove.front();
		_nodeToRemove.pop();

		pNode->ClearChildren();
		pNode->Release();
	}
}

DOUBLE CSTXAnimationControlChildNode::GetAncestorOpacity()
{
	DOUBLE fOpacity = 1.0f;
	CSTXAnimationControlChildNode *pTravelNode = _parentNode;
	while (pTravelNode)
	{
		if (pTravelNode->IsValueExists(_T("opacity")))
		{
			DOUBLE fParentOpacity = pTravelNode->GetValue(_T("opacity"));
			fOpacity *= fParentOpacity;
		}
		else
		{
			// fOpacity *= 1;
		}
		pTravelNode = pTravelNode->_parentNode;
	}
	return fOpacity;
}

BOOL CSTXAnimationControlChildNode::IsValueExists(LPCTSTR lpszKey)
{
	std::map<std::wstring, CComPtr<IUIAnimationVariable> >::iterator it = _animationVariables.find(lpszKey);
	return it != _animationVariables.end();
}

DOUBLE CSTXAnimationControlChildNode::GetAncestorScale()
{
	DOUBLE fScale = 1.0f;
	CSTXAnimationControlChildNode *pTravelNode = _parentNode;
	while (pTravelNode)
	{
		if (pTravelNode->IsValueExists(_T("scale")))
		{
			DOUBLE fParentOpacity = pTravelNode->GetValue(_T("scale"));
			fScale *= fParentOpacity;
		}
		else
		{
			// fOpacity *= 1;
		}
		pTravelNode = pTravelNode->_parentNode;
	}
	return fScale;
}

int CSTXAnimationControlChildNode::GetIndexInParentNode()
{
	if (_parentNode == NULL)
		return -1;

	std::vector<CSTXAnimationControlChildNode*>::iterator it = std::find(_parentNode->_childNodes.begin(), _parentNode->_childNodes.end(), this);
	if (it == _parentNode->_childNodes.end())
		return -1;

	return it - _parentNode->_childNodes.begin();
}

byte CSTXAnimationControlChildNode::GetAncestorOpacityByte()
{
	return static_cast<byte>(GetAncestorOpacity() * 255);
}

CSTXAnimationControlChildNode* CSTXAnimationControlChildNode::GetParentNode()
{
	return _parentNode;
}

BOOL CSTXAnimationControlChildNode::BeginSetValue(IUIAnimationStoryboard *pStory /*= NULL*/)
{
	if (_parentNode)
	{
		CSTXAnimationControlChildNode *pAncestorNode = GetAncestorNode();
		_pPendingTransition = &(pAncestorNode->_pendingTransition);

		return pAncestorNode->BeginSetValue(pStory);
	}

	if (_storyBoard)
	{
		InterlockedIncrement(&_nestedSetValue);
		return TRUE;
	}

	if (pStory)
		_storyBoard = pStory;
	else
		_parentControl->_animationManager->CreateStoryboard(&_storyBoard);

	InterlockedIncrement(&_nestedSetValue);
	_pendingTransition = 0;
	return TRUE;
}

BOOL CSTXAnimationControlChildNode::EndSetValue()
{
	if (_parentNode)
	{
		return GetAncestorNode()->EndSetValue();
	}

	if (_storyBoard)
	{
		InterlockedDecrement(&_nestedSetValue);
		if (_nestedSetValue == 0)
		{
			if (_pendingTransition)
			{
				UI_ANIMATION_SECONDS secTime;
				_parentControl->_animationTimer->GetTime(&secTime);
				_storyBoard->Schedule(secTime, NULL);
				_pendingTransition = 0;
				_storyBoard = NULL;
			}
		}
		return TRUE;
	}

	return FALSE;
}

IUIAnimationStoryboard* CSTXAnimationControlChildNode::GetCurrentStoryboard()
{
	return _storyBoard;
}

IUIAnimationStoryboard* CSTXAnimationControlChildNode::GetAncestorCurrentStoryboard()
{
	IUIAnimationStoryboard *pStoryboard = _storyBoard;
	CSTXAnimationControlChildNode *pTravelNode = _parentNode;
	while (pTravelNode)
	{
		pStoryboard = pTravelNode->_storyBoard;
		pTravelNode = pTravelNode->_parentNode;
	}
	return pStoryboard;
}

void CSTXAnimationControlChildNode::LayoutChildren(CSTXAnimationLayout *pLayout)
{
	int nIndex = 0;
	std::vector<CSTXAnimationControlChildNode*>::iterator it = _childNodes.begin();
	BeginSetValue();
	for (; it != _childNodes.end(); it++)
	{
		CSTXAnimationControlChildNode *pNode = *it;
		POINT ptLocation = pLayout->OnQueryNodePosition(pNode, nIndex);
		pNode->SetValue(_T("x"), ptLocation.x);
		pNode->SetValue(_T("y"), ptLocation.y);
		nIndex++;
	}
	EndSetValue();
}

CSTXAnimationControlChildNode* CSTXAnimationControlChildNode::GetAncestorNode()
{
	CSTXAnimationControlChildNode *pTravelNode = _parentNode;
	CSTXAnimationControlChildNode *pAncestor = NULL;
	while (pTravelNode)
	{
		pAncestor = pTravelNode;
		pTravelNode = pTravelNode->_parentNode;
	}
	return pAncestor;
}

void CSTXAnimationControlChildNode::GetBoundsRect(LPRECT lpRect)
{
	if (lpRect == NULL)
		return;

	lpRect->left = 0;
	lpRect->top = 0;
	lpRect->right = 0;
	lpRect->bottom = 0;
	std::vector<CSTXAnimationControlChildNode*>::iterator it = _childNodes.begin();
	for (; it != _childNodes.end(); it++)
	{
		CSTXAnimationControlChildNode *pNode = *it;
		int x = static_cast<int>(pNode->GetValue(_T("x")));
		int y = static_cast<int>(pNode->GetValue(_T("y")));
		int w = static_cast<int>(pNode->GetValue(_T("width")));
		int h = static_cast<int>(pNode->GetValue(_T("height")));
		int r = x + w;
		int b = y + h;

		if (x < lpRect->left)
			lpRect->left = x;
		if (y < lpRect->top)
			lpRect->top = y;

		if (r > lpRect->right)
			lpRect->right = r;
		if (b > lpRect->bottom)
			lpRect->bottom = b;
	}
}

void CSTXAnimationControlChildNode::GetFinalBoundsRect(LPRECT lpRect)
{
	if (lpRect == NULL)
		return;

	lpRect->left = 0;
	lpRect->top = 0;
	lpRect->right = 0;
	lpRect->bottom = 0;
	std::vector<CSTXAnimationControlChildNode*>::iterator it = _childNodes.begin();
	for (; it != _childNodes.end(); it++)
	{
		CSTXAnimationControlChildNode *pNode = *it;
		int x = static_cast<int>(pNode->GetFinalValue(_T("x")));
		int y = static_cast<int>(pNode->GetFinalValue(_T("y")));
		int w = static_cast<int>(pNode->GetFinalValue(_T("width")));
		int h = static_cast<int>(pNode->GetFinalValue(_T("height")));
		int r = x + w;
		int b = y + h;

		if (x < lpRect->left)
			lpRect->left = x;
		if (y < lpRect->top)
			lpRect->top = y;

		if (r > lpRect->right)
			lpRect->right = r;
		if (b > lpRect->bottom)
			lpRect->bottom = b;
	}
}

void CSTXAnimationControlChildNode::OnMouseDragging(POINT ptGlobal, POINT ptOffset)
{

}

void CSTXAnimationControlChildNode::OnBeginMouseDragging(POINT ptGlobal)
{

}

void CSTXAnimationControlChildNode::OnEndMouseDragging(POINT ptGlobal, POINT ptOffset)
{

}

UI_ANIMATION_SECONDS CSTXAnimationControlChildNode::OnQueryDefaultAnimationDuration()
{
	return 0.4;
}

void CSTXAnimationControlChildNode::PostDrawNode(CSTXGraphics *pGraphics)
{

}

BOOL CSTXAnimationControlChildNode::OnSetCursor(HWND hWnd, int x, int y, UINT nHitTest, UINT message)
{
	POINT pt = { x, y };

	std::vector<CSTXAnimationControlChildNode*>::iterator it = _childNodes.begin();
	std::queue<CSTXAnimationControlChildNode*> queueNodes;

	for (; it != _childNodes.end(); it++)
	{
		(*it)->AddRef();
		queueNodes.push(*it);
	}

	BOOL bResult = FALSE;
	BOOL bInAnyChild = FALSE;
	while (queueNodes.size())
	{
		CSTXAnimationControlChildNode *pNode = queueNodes.front();
		queueNodes.pop();

		BOOL bInNode = FALSE;
		POINT ptLocal = pNode->ConvertToNodeLocalPoint(pt, &bInNode);
		if (bInNode)
		{
			bInAnyChild = TRUE;
			bResult = pNode->OnSetCursor(hWnd, pt.x, pt.y, nHitTest, message);
		}

		pNode->Release();
	}

	return bResult;
}

LRESULT CSTXAnimationControlChildNode::OnPreWndProc(int x, int y, HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	POINT pt = { x, y };

	std::vector<CSTXAnimationControlChildNode*>::iterator it = _childNodes.begin();
	std::queue<CSTXAnimationControlChildNode*> queueNodes;

	for (; it != _childNodes.end(); it++)
	{
		(*it)->AddRef();
		queueNodes.push(*it);
	}

	LRESULT res = S_FALSE;
	BOOL bInAnyChild = FALSE;
	while (queueNodes.size())
	{
		CSTXAnimationControlChildNode *pNode = queueNodes.front();
		queueNodes.pop();

		BOOL bInNode = FALSE;
		POINT ptLocal = pNode->ConvertToNodeLocalPoint(pt, &bInNode);

		if (uMsg == WM_LBUTTONDBLCLK)
		{
			if (!bInNode)
			{
				pNode->Release();
				continue;
			}
		}
		if (dynamic_cast<CSTXAnimationControlEdit*>(pNode))
		{
			bInAnyChild = TRUE;
			res = pNode->OnPreWndProc(x, y, hwnd, uMsg, wParam, lParam);
		}
		pNode->Release();
	}
	return res;
}

void CSTXAnimationControlChildNode::OnAddedToNode(CSTXAnimationControlChildNode *pNodeParent)
{

}

void CSTXAnimationControlChildNode::SetCurrentFocusNode(CSTXAnimationControlChildNode *pNodeFocus)
{
	_focusNode = pNodeFocus;
}

void CSTXAnimationControlChildNode::OnPositionChangeStart()
{

}

void CSTXAnimationControlChildNode::SetValueIntegerChangeEventHandler(LPCTSTR lpszKey, IUIAnimationVariableIntegerChangeHandler *pHandler)
{
	std::map<std::wstring, CComPtr<IUIAnimationVariable> >::iterator it = _animationVariables.find(lpszKey);
	if (it != _animationVariables.end())
	{
		it->second->SetVariableIntegerChangeHandler(pHandler);
	}
}


//////////////////////////////////////////////////////////////////////////

CSTXAnimationControlWindow::CSTXAnimationControlWindow()
{
	_animationManager.CoCreateInstance(CLSID_UIAnimationManager);
	_animationTimer.CoCreateInstance(CLSID_UIAnimationTimer);
	_animationTransitionLibrary.CoCreateInstance(CLSID_UIAnimationTransitionLibrary);

	_hwndControl = NULL;
	_hwndToolTips = NULL;
	_style.drawNodesInViewOnly = TRUE;
	_style.autoCalculateContentSize = TRUE;
	_trackingMouse = FALSE;
	_graphicsCacheId = 19999901;
}

CSTXAnimationControlWindow::~CSTXAnimationControlWindow()
{
}

LPCTSTR CSTXAnimationControlWindow::GetControlClassName()
{
	//Override this method to provide a class name for your control
	OutputDebugString(_T("Be sure to override GetControlClassName() to provide a class name for your control!\n"));
	DebugBreak();

	return _T("CSTXAnimationControlWindow");
}

LRESULT CALLBACK CSTXAnimationControlWindow::STXAnimatedControlWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CSTXAnimationControlWindow *pThis = (CSTXAnimationControlWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if (pThis == NULL)
		return DefWindowProc(hwnd, uMsg, wParam, lParam);

	if (uMsg == WM_IME_STARTCOMPOSITION )
	{
		HIMC himc = ImmGetContext(hwnd);
		COMPOSITIONFORM cf;
		cf.dwStyle = CFS_POINT;
		cf.ptCurrentPos.x = 200;
		cf.ptCurrentPos.y = 300;
		GetCaretPos(&cf.ptCurrentPos);
		ImmSetCompositionWindow(himc, &cf);
	}

	LPARAM pos = GetMessagePos();
	POINT ptScreen = { GET_X_LPARAM(pos), GET_Y_LPARAM(pos) };
	ScreenToClient(hwnd, &ptScreen);
	switch (uMsg)
	{
	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	case WM_CHAR:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_LBUTTONDBLCLK:
	case WM_MOUSEMOVE:
	case WM_SETFOCUS:
	case WM_SETCURSOR:
	case WM_KILLFOCUS:
	//case WM_IME_STARTCOMPOSITION:
	//case WM_IME_COMPOSITION:
	//case WM_IME_ENDCOMPOSITION:
	//case WM_IME_CHAR:
		pThis->OnPreWndProc(ptScreen.x, ptScreen.y, hwnd, uMsg, wParam, lParam);
		break;
	default:
		break;
	}
	//LRESULT res = pThis->OnPreWndProc(GET_X_LPARAM(pos), GET_Y_LPARAM(pos), hwnd, uMsg, wParam, lParam);

	//if (res != S_FALSE)
	//	return res;

	switch (uMsg)
	{
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);
		pThis->OnPaint(hdc);
		EndPaint(hwnd, &ps);
	}
	break;
	case WM_TIMER:
		pThis->OnTimer(static_cast<UINT>(wParam));
		break;
	case WM_MOUSEMOVE:
		pThis->OnMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), wParam);
		break;
	case WM_LBUTTONDOWN:
		pThis->OnLButtonDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), wParam);
		break;
	case WM_LBUTTONUP:
		pThis->OnLButtonUp(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), wParam);
		break;
	case WM_RBUTTONDOWN:
		pThis->OnRButtonDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), wParam);
		break;
	case WM_RBUTTONUP:
		pThis->OnRButtonUp(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), wParam);
		break;
	case WM_LBUTTONDBLCLK:
		pThis->OnLButtonDblClk(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), wParam);
		break;
	case WM_MOUSELEAVE:
		pThis->OnMouseLeave();
		break;
	case WM_SIZE:
		pThis->OnSize(wParam, LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_HSCROLL:
		pThis->OnHScroll(LOWORD(wParam), HIWORD(wParam), (HWND)lParam);
		break;
	case WM_VSCROLL:
		pThis->OnVScroll(LOWORD(wParam), HIWORD(wParam), (HWND)lParam);
		break;
	case WM_DESTROY:
		pThis->OnDestroy();
		break;
	case WM_NOTIFY:
		pThis->OnNotify((UINT)wParam, (NMHDR*)lParam);
		break;
	case WM_SETCURSOR:
		if (pThis->OnSetCursor( (HWND)wParam, LOWORD(lParam), HIWORD(lParam)))
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
		break;
	case WM_CTLCOLOREDIT:
		//SetBkMode((HDC)wParam, TRANSPARENT);
		//SetTextColor((HDC)wParam, RGB(255, 0, 0));



		//{
		//	HDC hdc = (HDC)wParam;
		//	SetBkMode(hdc, TRANSPARENT); // Ensure that "static" text doesn't use a solid fill
		//	POINT pt;
		//	pt.x = 0; pt.y = 0;
		//	MapWindowPoints((HWND)lParam, hwnd, &pt, 1);
		//	SetBrushOrgEx(hdc, -pt.x, -pt.y, NULL);
		//}

		////return (LRESULT)GetSysColorBrush(COLOR_HIGHLIGHT);
		//return (LRESULT)GetStockObject(HOLLOW_BRUSH);
		//OutputDebugString(_T("ASDFF\n"));
		break;
	case WM_ERASEBKGND:
		return pThis->OnEraseBackground((HDC)wParam);
		break;
	//case WM_NCPAINT:
	//	{
	//		HDC hdc;
	//		hdc = GetDCEx(hwnd, (HRGN)wParam, DCX_WINDOW | DCX_INTERSECTRGN);
	//		// Paint into this DC 
	//		ReleaseDC(hwnd, hdc);
	//		return NULL;
	//	}
	//	break;
	}



	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

BOOL CSTXAnimationControlWindow::Create(LPCTSTR lpszWindowText, DWORD dwStyle, int x, int y, int cx, int cy, HWND hWndParent, UINT nID)
{
	if (GetSafeHwnd())
		return FALSE;

	LPCTSTR lpszClassName = GetControlClassName();
	if (!IsSystemControl())
	{
		RegisterAnimationControlClass(lpszClassName);
	}

	HWND hWnd = CreateWindow(GetControlClassName(), lpszWindowText, dwStyle, x, y, cx, cy, hWndParent, (HMENU)nID, GetModuleHandle(NULL), NULL);
	if (hWnd == NULL)
		return FALSE;

	_hwndControl = hWnd;
	OnWindowCreated();

	SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)this);

	_hwndToolTips = CreateWindowEx(WS_EX_TOPMOST,
		TOOLTIPS_CLASS,
		NULL,
		WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		hWnd,
		NULL,
		GetModuleHandle(NULL),
		NULL);

	//SetWindowPos(_hwndToolTips, HWND_TOPMOST, 0, 0, 0, 0,
	//	SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

	TOOLINFO toolInfo = { 0 };
	toolInfo.cbSize = TTTOOLINFO_V1_SIZE;
	toolInfo.hwnd = hWnd;
	toolInfo.uFlags = TTF_SUBCLASS | TTF_IDISHWND;
	toolInfo.uId = (UINT_PTR)hWnd;
	toolInfo.lpszText = LPSTR_TEXTCALLBACK;
	GetClientRect(hWnd, &toolInfo.rect);
	SendMessage(_hwndToolTips, TTM_ADDTOOL, 0, (LPARAM)&toolInfo);

	//toolInfo.cbSize = sizeof(TOOLINFO);
	//toolInfo.uFlags = TTF_SUBCLASS | TTF_IDISHWND | TTF_TRACK | TTF_ABSOLUTE;
	//toolInfo.hwnd = hWnd;
	//toolInfo.hinst = GetModuleHandle(NULL);
	//toolInfo.lpszText = _T("sasd");
	//toolInfo.uId = (UINT_PTR)hWnd;

	//GetClientRect(hWnd, &g_toolItem.rect);
	//SendMessage(_hwndToolTips, TTM_ADDTOOL, 0, (LPARAM)&toolInfo);


	return TRUE;

}

HWND CSTXAnimationControlWindow::GetSafeHwnd()
{
	return _hwndControl;
}

void CSTXAnimationControlWindow::RegisterAnimationControlClass(LPCTSTR lpszClassName)
{
	WNDCLASS wc;
	memset(&wc, 0, sizeof(wc));
	wc.hInstance = GetModuleHandle(NULL);
	wc.lpfnWndProc = STXAnimatedControlWindowProc;
	wc.lpszClassName = lpszClassName;
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wc.hCursor = ::LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW));

	RegisterClass(&wc);
}

void CSTXAnimationControlWindow::OnPaint(HDC hDC)
{
	RECT rcControl;
	::GetClientRect(_hwndControl, &rcControl);

	CSTXGraphics *pGraphics = NULL;

	pGraphics = CSTXGraphics::CreateAutoGraphics(_hwndControl, hDC, rcControl.right - rcControl.left, rcControl.bottom - rcControl.top, _graphicsCacheId);
	//pGraphics = CSTXGraphics::CreateGdiPlusGraphics(_hwndControl, hDC, rcControl.right - rcControl.left, rcControl.bottom - rcControl.top, _graphicsCacheId);

	pGraphics->BeginDraw();

	DrawBackground(pGraphics);
	DrawControl(pGraphics);

	HRESULT hr = pGraphics->EndDraw();

	//PostDrawControl(pGraphics);

#if _WIN32_WINNT >= 0x0601
	if (hr == D2DERR_RECREATE_TARGET)
	{
		pGraphics->DiscardDeviceDependentResource();
	}
#endif

	delete pGraphics;
}

void CSTXAnimationControlWindow::DrawControl(CSTXGraphics *pGraphics)
{
	RECT rcClient;
	GetClientRect(_hwndControl, &rcClient);
	int viewHeight = rcClient.bottom - rcClient.top;

	int iHScrollPos = GetScrollPos(_hwndControl, SB_HORZ);
	int iVScrollPos = GetScrollPos(_hwndControl, SB_VERT);

	std::queue<CSTXAnimationControlChildNode*> nodeToDraw;
	size_t nDrawDelete = _nodeToRemove.size();
	for (size_t i = 0; i < nDrawDelete; i++)
	{
		CSTXAnimationControlChildNode *pItem = _nodeToRemove.front();
		_nodeToRemove.pop();

		if (!pItem->IsReadyToClear())
		{
			_nodeToRemove.push(pItem);
			nodeToDraw.push(pItem);
		}
		else
		{
			pItem->Release();
		}
	}

	std::vector<CSTXAnimationControlChildNode*>::iterator it = _childNodes.begin();
	for (; it != _childNodes.end(); it++)
	{
		nodeToDraw.push(*it);
	}


	pGraphics->TranslateTransform(-iHScrollPos, -iVScrollPos, 0);
	if (_style.drawNodesInViewOnly)
	{
		while (nodeToDraw.size())
		{
			CSTXAnimationControlChildNode *pItem = nodeToDraw.front();
			nodeToDraw.pop();


			DOUBLE y = pItem->GetValue(_T("y"));
			DOUBLE h = pItem->GetValue(_T("height"));
			POINT ptLocal = { 0, static_cast<int>(y) };
			POINT ptGlobal = ConvertToGlobalPoint(ptLocal);

			if (y + h < iVScrollPos || y - iVScrollPos > viewHeight)
				continue;

			pItem->DrawNode(pGraphics);
		}
	}
	else
	{
		while (nodeToDraw.size())
		{
			CSTXAnimationControlChildNode *pItem = nodeToDraw.front();
			nodeToDraw.pop();
			pItem->DrawNode(pGraphics);
		}
	}

	pGraphics->TranslateTransform(iHScrollPos, iVScrollPos, 0);

}

void CSTXAnimationControlWindow::OnTimer(UINT nIDEvent)
{
	if (nIDEvent == STXANIMATIONCONTROL_TIMER_ID_ANIMATION)
	{
		UpdateAnimationManager();
		InvalidateRect(_hwndControl, NULL, TRUE);
	}
}

void CSTXAnimationControlWindow::UpdateAnimationManager()
{
	UI_ANIMATION_UPDATE_RESULT result;
	UI_ANIMATION_SECONDS secTime;
	_animationTimer->GetTime(&secTime);
	_animationManager->Update(secTime, &result);
}

void CSTXAnimationControlWindow::StartAnimationTimer()
{
	::SetTimer(_hwndControl, STXANIMATIONCONTROL_TIMER_ID_ANIMATION, 5, NULL);
}

void CSTXAnimationControlWindow::StopAnimationTimer()
{
	::KillTimer(_hwndControl, STXANIMATIONCONTROL_TIMER_ID_ANIMATION);
}

void CSTXAnimationControlWindow::DrawBackground(CSTXGraphics *pGraphics)
{
	//if (m_pImgBackground)
	//{
	//	if (!m_pImgBackgroundCached)
	//	{
	//		Gdiplus::Bitmap* pBitmap = new Gdiplus::Bitmap(rectThis->Width, rectThis->Height);
	//		Gdiplus::Graphics graphics(pBitmap);
	//		graphics.DrawImage(m_pImgBackground.get(), 0, 0, rectThis->Width, rectThis->Height);

	//		Gdiplus::Bitmap *pBitmapSrc = pBitmap;
	//		std::tr1::shared_ptr<Gdiplus::CachedBitmap> imgCached(new Gdiplus::CachedBitmap(pBitmapSrc, pGraphics));
	//		m_pImgBackgroundCached = imgCached;

	//		delete pBitmap;
	//	}
	//	if (m_pImgBackgroundCached)
	//		pGraphics->DrawCachedBitmap(m_pImgBackgroundCached.get(), 0, 0);
	//	else
	//		pGraphics->DrawImage(m_pImgBackground.get(), rectThis->X, rectThis->Y, rectThis->Width, rectThis->Height);
	//}
	//else
	{
		//Gdiplus::SolidBrush brushBk(m_clrBackground);
		CSTXGraphicsBrush *pBrush = pGraphics->CreateSolidBrush(255, 255, 255, 255);
		pBrush->SetOpacity(255);
		RECT rcControl;
		::GetClientRect(_hwndControl, &rcControl);

		pGraphics->FillRectangle(rcControl.left, rcControl.top, rcControl.right - rcControl.left, rcControl.bottom - rcControl.top, pBrush);

		pBrush->Release();
	}
}

void CSTXAnimationControlWindow::OnMouseMove(int x, int y, UINT nFlags)
{
	if (!_trackingMouse)   // The mouse has just entered the window.
	{                       // Request notification when the mouse leaves.
		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(tme);
		tme.hwndTrack = _hwndControl;
		tme.dwFlags = TME_LEAVE;
		TrackMouseEvent(&tme);
		_trackingMouse = TRUE;
	}

	int iHScrollPos = GetScrollPos(_hwndControl, SB_HORZ);
	int iVScrollPos = GetScrollPos(_hwndControl, SB_VERT);

	UpdateAnimationManager();
	__super::OnMouseMove(x + iHScrollPos, y + iVScrollPos, nFlags);
}

void CSTXAnimationControlWindow::OnLButtonDown(int x, int y, UINT nFlags, BOOL bForRButton /*= FALSE*/)
{
	SetFocus(_hwndControl);
	SetCapture(_hwndControl);

	int iHScrollPos = GetScrollPos(_hwndControl, SB_HORZ);
	int iVScrollPos = GetScrollPos(_hwndControl, SB_VERT);
	__super::OnLButtonDown(x + iHScrollPos, y + iVScrollPos, nFlags, bForRButton);
}

void CSTXAnimationControlWindow::OnLButtonUp(int x, int y, UINT nFlags, BOOL bForRButton /*= FALSE*/)
{
	ReleaseCapture();

	int iHScrollPos = GetScrollPos(_hwndControl, SB_HORZ);
	int iVScrollPos = GetScrollPos(_hwndControl, SB_VERT);
	__super::OnLButtonUp(x + iHScrollPos, y + iVScrollPos, nFlags, bForRButton);
}

void CSTXAnimationControlWindow::OnMouseLeave()
{
	_trackingMouse = FALSE;

	__super::OnMouseLeave();
}

void CSTXAnimationControlWindow::OnRButtonDown(int x, int y, UINT nFlags)
{
	OnLButtonDown(x, y, nFlags, TRUE);
}

void CSTXAnimationControlWindow::OnRButtonUp(int x, int y, UINT nFlags)
{
	OnLButtonUp(x, y, nFlags, TRUE);
}

void CSTXAnimationControlWindow::OnSize(UINT nType, int cx, int cy)
{
#if _WIN32_WINNT >= 0x0601
	CSTXD2DGraphics::OnSize(_hwndControl, cx, cy, _graphicsCacheId);
#endif

	__super::OnSize(nType, cx, cy);
	ResetScrollBars();
	UpdateAnimationManager();
}

void CSTXAnimationControlWindow::OnLButtonDblClk(int x, int y, UINT nFlags)
{
	int iHScrollPos = GetScrollPos(_hwndControl, SB_HORZ);
	int iVScrollPos = GetScrollPos(_hwndControl, SB_VERT);
	__super::OnLButtonDblClk(x + iHScrollPos, y + iVScrollPos, nFlags);
}

BOOL CSTXAnimationControlWindow::ModifyStyle(int nStyleOffset, DWORD dwRemove, DWORD dwAdd, UINT nFlags)
{
	DWORD dwStyle = ::GetWindowLong(_hwndControl, nStyleOffset);
	DWORD dwNewStyle = (dwStyle & ~dwRemove) | dwAdd;
	if (dwStyle == dwNewStyle)
		return FALSE;

	::SetWindowLong(_hwndControl, nStyleOffset, dwNewStyle);
	if (nFlags != 0)
	{
		::SetWindowPos(_hwndControl, NULL, 0, 0, 0, 0,
			SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | nFlags);
	}
	return TRUE;

}

void CSTXAnimationControlWindow::ModifyStyle(DWORD dwRemove, DWORD dwAdd)
{
	if (GetSafeHwnd() == NULL)
		return;

	ModifyStyle(GWL_STYLE, dwRemove, dwAdd, 0);
	InvalidateRect(_hwndControl, NULL, FALSE);

}


void CSTXAnimationControlWindow::OnHScroll(UINT nSBCode, UINT nPos, HWND hWndScrollBar)
{
	OnEndEdit();

	int minpos;
	int maxpos;
	GetScrollRange(_hwndControl, SB_HORZ, &minpos, &maxpos);
	maxpos = GetScrollLimit(SB_HORZ);

	// Get the current position of scroll box.
	int curpos = GetScrollPos(_hwndControl, SB_HORZ);
	int oldpos = curpos;

	// Determine the new position of scroll box.
	switch (nSBCode)
	{
	case SB_LEFT:      // Scroll to far left.
		curpos = minpos;
		break;

	case SB_RIGHT:      // Scroll to far right.
		curpos = maxpos;
		break;

	case SB_ENDSCROLL:   // End scroll.
		break;

	case SB_LINELEFT:      // Scroll left.
		if (curpos > minpos)
			curpos -= 5;
		break;

	case SB_LINERIGHT:   // Scroll right.
		if (curpos < maxpos)
			curpos += 5;
		break;

	case SB_PAGELEFT:    // Scroll one page left.
	{
		// Get the page size. 
		SCROLLINFO   info;
		info.fMask = SIF_ALL;
		GetScrollInfo(_hwndControl, SB_HORZ, &info);

		if (curpos > minpos)
			curpos = max(minpos, curpos - (int)info.nPage);
	}
	break;

	case SB_PAGERIGHT:      // Scroll one page right.
	{
		// Get the page size. 
		SCROLLINFO   info;
		info.fMask = SIF_ALL;
		GetScrollInfo(_hwndControl, SB_HORZ, &info);

		if (curpos < maxpos)
			curpos = min(maxpos, curpos + (int)info.nPage);
	}
	break;

	case SB_THUMBPOSITION: // Scroll to absolute position. nPos is the position
		curpos = nPos;      // of the scroll box at the end of the drag operation.
		break;

	case SB_THUMBTRACK:   // Drag scroll box to specified position. nPos is the
		curpos = nPos;     // position that the scroll box has been dragged to.
		break;
	}

	// Set the new position of the thumb (scroll box).
	SetScrollPos(_hwndControl, SB_HORZ, curpos, TRUE);

	InvalidateRect(_hwndControl, NULL, TRUE);
}

void CSTXAnimationControlWindow::OnVScroll(UINT nSBCode, UINT nPos, HWND hWndScrollBar)
{
	OnEndEdit();

	int minpos;
	int maxpos;
	GetScrollRange(_hwndControl, SB_VERT, &minpos, &maxpos);
	maxpos = GetScrollLimit(SB_VERT);

	// Get the current position of scroll box.
	int curpos = GetScrollPos(_hwndControl, SB_VERT);
	int oldpos = curpos;

	// Determine the new position of scroll box.
	switch (nSBCode)
	{
	case SB_LEFT:      // Scroll to far left.
		curpos = minpos;
		break;

	case SB_RIGHT:      // Scroll to far right.
		curpos = maxpos;
		break;

	case SB_ENDSCROLL:   // End scroll.
		break;

	case SB_LINELEFT:      // Scroll left.
		if (curpos > minpos)
			curpos -= 5;
		break;

	case SB_LINERIGHT:   // Scroll right.
		if (curpos < maxpos)
			curpos += 5;
		break;

	case SB_PAGELEFT:    // Scroll one page left.
	{
		// Get the page size. 
		SCROLLINFO   info;
		info.fMask = SIF_ALL;
		GetScrollInfo(_hwndControl, SB_VERT, &info);

		if (curpos > minpos)
			curpos = max(minpos, curpos - (int)info.nPage);
	}
	break;

	case SB_PAGERIGHT:      // Scroll one page right.
	{
		// Get the page size. 
		SCROLLINFO   info;
		info.fMask = SIF_ALL;
		GetScrollInfo(_hwndControl, SB_VERT, &info);

		if (curpos < maxpos)
			curpos = min(maxpos, curpos + (int)info.nPage);
	}
	break;

	case SB_THUMBPOSITION: // Scroll to absolute position. nPos is the position
		curpos = nPos;      // of the scroll box at the end of the drag operation.
		break;

	case SB_THUMBTRACK:   // Drag scroll box to specified position. nPos is the
		curpos = nPos;     // position that the scroll box has been dragged to.
		break;
	}

	// Set the new position of the thumb (scroll box).
	SetScrollPos(_hwndControl, SB_VERT, curpos, TRUE);

	InvalidateRect(_hwndControl, NULL, TRUE);
}

void CSTXAnimationControlWindow::OnEndEdit()
{

}

int CSTXAnimationControlWindow::GetScrollLimit(int nBar)
{
	int nMin, nMax;
	GetScrollRange(_hwndControl, nBar, &nMin, &nMax);
	SCROLLINFO info;
	info.fMask = SIF_PAGE;
	if (GetScrollInfo(_hwndControl, nBar, &info))
	{
		nMax -= __max(info.nPage - 1, 0);
	}
	return nMax;
}

void CSTXAnimationControlWindow::OnDestroy()
{
	CSTXGraphics::ClearCachedGraphicsObjects(_graphicsCacheId);
#if _WIN32_WINNT >= 0x0601
	CSTXD2DGraphics::Clear(_graphicsCacheId);
#endif
	_animationManager->SetManagerEventHandler(nullptr);
}

LRESULT CSTXAnimationControlWindow::OnNotify(UINT nID, NMHDR *pNMHDR)
{
	if (pNMHDR->code == TTN_GETDISPINFO)		//TTN_NEEDTEXT is equal to TTN_GETDISPINFO
	{
		LPNMTTDISPINFO pDispInfo = (LPNMTTDISPINFO)pNMHDR;

		POINT ptCursor;
		GetCursorPos(&ptCursor);
		::ScreenToClient(_hwndControl, &ptCursor);

		LPCTSTR lpszToolTips = NULL;
		OnQueryToolTipsText(pDispInfo->szText, 80, ptCursor, &lpszToolTips);
		pDispInfo->lpszText = pDispInfo->szText;

		if (lpszToolTips)
			pDispInfo->lpszText = (LPTSTR)lpszToolTips;

		return 1;
	}

	return 0;
}

void CSTXAnimationControlWindow::OnQueryToolTipsText(LPTSTR pszBuffer, int cchBufferSize, POINT ptLocation, LPCTSTR *ppszToolTips)
{
	pszBuffer[0] = 0;
	if (ppszToolTips)
		*ppszToolTips = NULL;

	__super::OnQueryToolTipsText(pszBuffer, cchBufferSize, ptLocation, ppszToolTips);
}

void CSTXAnimationControlWindow::ResetScrollBars()
{
	if (!IsWindow(_hwndControl))
		return;

	RECT rcClient;
	::GetClientRect(_hwndControl, &rcClient);

	//Vertical Scroll Bar
	int iTotalHeightAvailable = rcClient.bottom - rcClient.top;

	int iCurPos = 0;
	BOOL bVScrollExist;
	if ((GetWindowLong(_hwndControl, GWL_STYLE) & WS_VSCROLL) == WS_VSCROLL)
	{
		iCurPos = ::GetScrollPos(_hwndControl, SB_VERT);
		bVScrollExist = TRUE;
	}
	else
		bVScrollExist = FALSE;

	int iOldPos = ::GetScrollPos(_hwndControl, SB_VERT);

	int nTotalHeight = static_cast<int>(OnQueryNodeFinalHeight());
	if (_childNodes.size() > 0 && nTotalHeight > iTotalHeightAvailable)	//Need H-ScrollBar
	{
		SCROLLINFO si;
		si.cbSize = sizeof(si);
		si.fMask = SIF_PAGE | SIF_POS | SIF_RANGE;
		si.nPage = iTotalHeightAvailable;
		si.nMin = 0;
		si.nMax = nTotalHeight;
		si.nPos = min(iCurPos, si.nMax);

		SetScrollPos(_hwndControl, SB_VERT, si.nPos, FALSE);
		SetScrollInfo(_hwndControl, SB_VERT, &si, TRUE);
		::ShowScrollBar(_hwndControl, SB_VERT, TRUE);
		ModifyStyle(0, WS_VSCROLL);
	}
	else
	{
		int iCurPos = GetScrollPos(_hwndControl, SB_VERT);
		//ScrollWindow(_hwndControl, 0, iCurPos, NULL, NULL);
		SetScrollPos(_hwndControl, SB_VERT, 0, TRUE);
		::ShowScrollBar(_hwndControl, SB_VERT, FALSE);
		ModifyStyle(WS_VSCROLL, 0);
	}

	//Horizontal Scroll Bar
	ResetHorizontalScrollBar();

	InvalidateRect(_hwndControl, NULL, FALSE);
}

void CSTXAnimationControlWindow::ResetHorizontalScrollBar()
{

}

HRESULT CSTXAnimationControlWindow::OnManagerStatusChanged(UI_ANIMATION_MANAGER_STATUS newStatus, UI_ANIMATION_MANAGER_STATUS previousStatus)
{
	if (newStatus == UI_ANIMATION_MANAGER_BUSY)
	{
		StartAnimationTimer();
	}
	else
	{
		StopAnimationTimer();
		ClearPendingDeleteNodes();
		InvalidateRect(_hwndControl, NULL, TRUE);
	}

	return S_OK;
}

void CSTXAnimationControlWindow::InitializeChildNode(CSTXAnimationControlChildNode *pNode, std::map<std::wstring, DOUBLE> *initialValues /*= NULL*/)
{
	if (initialValues)
	{
		std::map<std::wstring, DOUBLE>::iterator itInitial = initialValues->begin();
		for (; itInitial != initialValues->end(); itInitial++)
		{
			CComPtr<IUIAnimationVariable> spVar;
			_animationManager->CreateAnimationVariable(itInitial->second, &spVar);
			pNode->_animationVariables[itInitial->first] = spVar;

			IUIAnimationVariableIntegerChangeHandler *pVarIntChangeHandler = dynamic_cast<IUIAnimationVariableIntegerChangeHandler*>(pNode);
			if (pVarIntChangeHandler)
			{
				spVar->SetVariableIntegerChangeHandler(pVarIntChangeHandler);
			}
		}
	}

	std::vector<std::pair<std::wstring, double>> arrInitialVariables;
	arrInitialVariables.push_back(std::pair<std::wstring, double>(_T("x"), 0));
	arrInitialVariables.push_back(std::pair<std::wstring, double>(_T("y"), 0));
	arrInitialVariables.push_back(std::pair<std::wstring, double>(_T("width"), 1));
	arrInitialVariables.push_back(std::pair<std::wstring, double>(_T("height"), 1));

	std::vector<std::pair<std::wstring, double>>::iterator it = arrInitialVariables.begin();
	for (; it != arrInitialVariables.end(); it++)
	{
		if (initialValues && initialValues->find(it->first) != initialValues->end())
			continue;

		CComPtr<IUIAnimationVariable> spVar;
		_animationManager->CreateAnimationVariable(it->second, &spVar);
		pNode->_animationVariables[it->first] = spVar;

		IUIAnimationVariableIntegerChangeHandler *pVarIntChangeHandler = dynamic_cast<IUIAnimationVariableIntegerChangeHandler*>(pNode);
		if (pVarIntChangeHandler)
		{
			spVar->SetVariableIntegerChangeHandler(pVarIntChangeHandler);
		}
	}

	pNode->_parentControl = this;
}

int CSTXAnimationControlWindow::AddChildNode(CSTXAnimationControlChildNode *pNode)
{
	int nResult = __super::AddChildNode(pNode);

	DOUBLE fHeightTotal = 0;
	if (_style.autoCalculateContentSize)
	{
		std::vector<CSTXAnimationControlChildNode*>::iterator it = _childNodes.begin();
		for (; it != _childNodes.end(); it++)
		{
			DOUBLE y = (*it)->GetFinalValue(_T("y"));
			DOUBLE h = (*it)->GetFinalValue(_T("height"));

			if (fHeightTotal < y + h)
				fHeightTotal = y + h;
		}

		SetValueInstantly(_T("height"), fHeightTotal);
	}

	ResetScrollBars();
	return nResult;
}

BOOL CSTXAnimationControlWindow::IsSystemControl()
{
	return FALSE;
}

BOOL CSTXAnimationControlWindow::OnEraseBackground(HDC hDC)
{
	SetBkMode(hDC, TRANSPARENT);
	SetTextColor(hDC, RGB(255,0,0));

	return TRUE;
}

void CSTXAnimationControlWindow::OnWindowCreated()
{
	_animationManager->SetManagerEventHandler(this);
	InitializeChildNode(this);
}

void CSTXAnimationControlWindow::PostDrawControl(CSTXGraphics *pGraphics)
{
	RECT rcClient;
	GetClientRect(_hwndControl, &rcClient);
	int viewHeight = rcClient.bottom - rcClient.top;

	int iHScrollPos = GetScrollPos(_hwndControl, SB_HORZ);
	int iVScrollPos = GetScrollPos(_hwndControl, SB_VERT);

	std::queue<CSTXAnimationControlChildNode*> nodeToDraw;
	size_t nDrawDelete = _nodeToRemove.size();
	for (size_t i = 0; i < nDrawDelete; i++)
	{
		CSTXAnimationControlChildNode *pItem = _nodeToRemove.front();
		_nodeToRemove.pop();

		if (!pItem->IsReadyToClear())
		{
			_nodeToRemove.push(pItem);
			nodeToDraw.push(pItem);
		}
	}

	std::vector<CSTXAnimationControlChildNode*>::iterator it = _childNodes.begin();
	for (; it != _childNodes.end(); it++)
	{
		nodeToDraw.push(*it);
	}

	pGraphics->TranslateTransform(-iHScrollPos, -iVScrollPos, 0);
	if (_style.drawNodesInViewOnly)
	{
		while (nodeToDraw.size())
		{
			CSTXAnimationControlChildNode *pItem = nodeToDraw.front();
			nodeToDraw.pop();


			DOUBLE y = pItem->GetValue(_T("y"));
			DOUBLE h = pItem->GetValue(_T("height"));
			POINT ptLocal = { 0, static_cast<int>(y) };
			POINT ptGlobal = ConvertToGlobalPoint(ptLocal);

			if (y + h < iVScrollPos || y - iVScrollPos > viewHeight)
				continue;

			pItem->PostDrawNode(pGraphics);
		}
	}
	else
	{
		while (nodeToDraw.size())
		{
			CSTXAnimationControlChildNode *pItem = nodeToDraw.front();
			nodeToDraw.pop();
			pItem->PostDrawNode(pGraphics);
		}
	}

	pGraphics->TranslateTransform(iHScrollPos, iVScrollPos, 0);
}

BOOL CSTXAnimationControlWindow::OnSetCursor(HWND hWnd, UINT nHitTest, UINT message)
{
	POINT point;
	GetCursorPos(&point);
	ScreenToClient(_hwndControl, &point);

	int iHScrollPos = GetScrollPos(_hwndControl, SB_HORZ);
	int iVScrollPos = GetScrollPos(_hwndControl, SB_VERT);

	return __super::OnSetCursor(hWnd, point.x + iHScrollPos, point.y + iVScrollPos, nHitTest, message);
}

void CSTXAnimationControlWindow::RedrawWindow()
{
	InvalidateRect(_hwndControl, NULL, TRUE);
}

//////////////////////////////////////////////////////////////////////////

CSTXAnimationControlEdit::CSTXAnimationControlEdit()
{
	InitDefaultCharFormat(&_cf, NULL);
	InitDefaultParaFormat(&_pf);

	_tipText = _T("Please Enter!");

	_textMaxLength = 64 * 1024;
	_msfteditDLL = NULL;
	_oldWndProc = NULL;
	m_nRef = 1;

	HRESULT hr;

	IUnknown* pUnk = NULL;
	ITextServices* pTextServices = NULL;

	// Create an instance of the application-defined object that implements the ITextHost interface.

	typedef HRESULT (__stdcall*CreateTextServicesPtr)(
		_In_   IUnknown *punkOuter,
		_In_   ITextHost *pITextHost,
		_Out_  IUnknown **ppUnk
		);

	_msfteditDLL = LoadLibrary(_T("Msftedit.dll"));
	if (_msfteditDLL)
	{
		CreateTextServicesPtr CreateTextServicesFunc = (CreateTextServicesPtr)GetProcAddress(_msfteditDLL, "CreateTextServices");
		IID* pIID_ITH = (IID*)(VOID*)GetProcAddress(_msfteditDLL, "IID_ITextHost");
		_iidTextHost = *pIID_ITH;
		// Create an instance of the text services object.
		hr = CreateTextServicesFunc(NULL, this, &pUnk);

		if (FAILED(hr))
		{

		}

		// Retrieve the IID_ITextServices interface identifier from Msftedit.dll.
		IID* pIID_ITS = (IID*)(VOID*)GetProcAddress(_msfteditDLL, "IID_ITextServices");
		IID* pIID_ITS2 = (IID*)(VOID*)GetProcAddress(_msfteditDLL, "IID_ITextServices2");

		// Retrieve the ITextServices interface.    
		hr = pUnk->QueryInterface(*pIID_ITS, (void **)&_textService);

		if (FAILED(hr))
		{

		}

	}

}

CSTXAnimationControlEdit::~CSTXAnimationControlEdit()
{
	if (_msfteditDLL)
	{
		FreeLibrary(_msfteditDLL);
	}
}

STDMETHODIMP CSTXAnimationControlEdit::QueryInterface(const IID & iid, void **ppv)
{
	if (iid == _iidTextHost)
	{
		*ppv = this;
		return S_OK;
	}
	return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CSTXAnimationControlEdit::Release(void)
{
	ULONG l;
	l = InterlockedDecrement(&m_nRef);
	if (0 == l)
		delete this;
	return l;
}

STDMETHODIMP_(ULONG) CSTXAnimationControlEdit::AddRef(void)
{
	return InterlockedIncrement(&m_nRef);
}

HRESULT CSTXAnimationControlEdit::OnIntegerValueChanged(IUIAnimationStoryboard * storyboard, IUIAnimationVariable * variable, INT32 newValue, INT32 previousValue)
{
	HideCaret(GetParentControl()->GetSafeHwnd());
	return S_OK;
}

BOOL CSTXAnimationControlEdit::OnEraseBackground(HDC hDC)
{
	return TRUE;
}

BOOL CSTXAnimationControlEdit::TxSetScrollRange(INT fnBar, LONG nMinPos, INT nMaxPos, BOOL fRedraw)
{
	return FALSE;
}

BOOL CSTXAnimationControlEdit::TxSetScrollPos(INT fnBar, INT nPos, BOOL fRedraw)
{
	return FALSE;
}

void CSTXAnimationControlEdit::TxInvalidateRect(LPCRECT prc, BOOL fMode)
{
	if (GetParentControl())
		::InvalidateRect(GetParentControl()->_hwndControl, prc, fMode);
}

void CSTXAnimationControlEdit::TxViewChange(BOOL fUpdate)
{
}

BOOL CSTXAnimationControlEdit::TxCreateCaret(HBITMAP hbmp, INT xWidth, INT yHeight)
{
	return ::CreateCaret(GetParentControl()->_hwndControl, hbmp, xWidth, yHeight);
}

BOOL CSTXAnimationControlEdit::TxShowCaret(BOOL fShow)
{
	if (fShow)
	{
		return ::ShowCaret(GetParentControl()->_hwndControl);
	}
	else
	{
		return ::HideCaret(GetParentControl()->_hwndControl);
	}
}

BOOL CSTXAnimationControlEdit::TxSetCaretPos(INT x, INT y)
{
	return ::SetCaretPos(x, y);
}

BOOL CSTXAnimationControlEdit::TxSetTimer(UINT idTimer, UINT uTimeout)
{
	::SetTimer(GetParentControl()->_hwndControl, idTimer, uTimeout, NULL);
	return FALSE;
}

void CSTXAnimationControlEdit::TxKillTimer(UINT idTimer)
{
	::KillTimer(GetParentControl()->_hwndControl, idTimer);
}

void CSTXAnimationControlEdit::TxScrollWindowEx(INT dx, INT dy, LPCRECT lprcScroll, LPCRECT lprcClip, HRGN hrgnUpdate, LPRECT lprcUpdate, UINT fuScroll)
{
}

void CSTXAnimationControlEdit::TxSetCapture(BOOL fCapture)
{
	if (fCapture)
	{
		SetCapture(GetParentControl()->_hwndControl);
	}
	else
	{
		ReleaseCapture();
	}
}

void CSTXAnimationControlEdit::TxSetFocus()
{
	SetFocus(GetParentControl()->_hwndControl);
}

void CSTXAnimationControlEdit::TxSetCursor(HCURSOR hcur, BOOL fText)
{
}

BOOL CSTXAnimationControlEdit::TxScreenToClient(LPPOINT lppt)
{
	return ::ScreenToClient(GetParentControl()->_hwndControl, lppt);
}

BOOL CSTXAnimationControlEdit::TxClientToScreen(LPPOINT lppt)
{
	return ::ClientToScreen(GetParentControl()->_hwndControl, lppt);
}

HRESULT	CSTXAnimationControlEdit::TxActivate(LONG * plOldState)
{
	return S_OK;
}

HRESULT	CSTXAnimationControlEdit::TxDeactivate(LONG lNewState)
{
	return S_OK;
}

HRESULT	CSTXAnimationControlEdit::TxGetClientRect(LPRECT prc)
{
	ANI_VAR_VALUE_INT(x, x);
	ANI_VAR_VALUE_INT(y, y);
	ANI_VAR_VALUE_INT(w, width);
	ANI_VAR_VALUE_INT(h, height);

	int nPadding = 2;
	RECT rect = { x + nPadding, y + nPadding, x + w - nPadding * 2, y + h - nPadding * 2};
	//RECT rect = { 0,0,w,h };

	*prc = rect;
	//*prc = m_rcClient;
	return S_OK;
}

HRESULT	CSTXAnimationControlEdit::TxGetViewInset(LPRECT prc)
{
	//ANI_VAR_FINAL_VALUE_INT(x, x);
	//ANI_VAR_FINAL_VALUE_INT(y, y);
	//ANI_VAR_FINAL_VALUE_INT(w, width);
	//ANI_VAR_FINAL_VALUE_INT(h, height);

	RECT rect = { 0, 0, 0,0 };

	*prc = rect;

	//*prc = m_rcViewInset;
	return S_OK;
}

HRESULT CSTXAnimationControlEdit::TxGetCharFormat(const CHARFORMATW **ppCF)
{
	*ppCF = &_cf;
	return S_OK;
}

HRESULT	CSTXAnimationControlEdit::TxGetParaFormat(const PARAFORMAT **ppPF)
{
	*ppPF = &_pf;
	return S_OK;
}

COLORREF CSTXAnimationControlEdit::TxGetSysColor(int nIndex)
{
	return GetSysColor(nIndex);
}

HRESULT	CSTXAnimationControlEdit::TxGetBackStyle(TXTBACKSTYLE *pstyle)
{
	*pstyle = TXTBACK_TRANSPARENT;
	return S_OK;
}

HRESULT	CSTXAnimationControlEdit::TxGetMaxLength(DWORD *plength)
{
	*plength = _textMaxLength;
	return S_OK;
}

HRESULT	CSTXAnimationControlEdit::TxGetScrollBars(DWORD *pdwScrollBar)
{
	*pdwScrollBar = 0;
	return S_OK;
}

HRESULT	CSTXAnimationControlEdit::TxGetPasswordChar(TCHAR *pch)
{
	return S_FALSE;
}

HRESULT	CSTXAnimationControlEdit::TxGetAcceleratorPos(LONG *pcp)
{
	*pcp = -1;
	return S_OK;
}

#define HIMETRIC_PER_INCH 2540

LONG DXtoHimetricX(LONG dx, LONG xPerInch)
{
	return (LONG)MulDiv(dx, HIMETRIC_PER_INCH, xPerInch);
}

// Convert Pixels on the Y axis to Himetric
LONG DYtoHimetricY(LONG dy, LONG yPerInch)
{
	return (LONG)MulDiv(dy, HIMETRIC_PER_INCH, yPerInch);
}

HRESULT	CSTXAnimationControlEdit::TxGetExtent(LPSIZEL lpExtent)
{
//	ANI_VAR_FINAL_VALUE_INT(x, x);
//	ANI_VAR_FINAL_VALUE_INT(y, y);
	ANI_VAR_FINAL_VALUE_INT(w, width);
	ANI_VAR_FINAL_VALUE_INT(h, height);

	HDC hdc = GetDC(GetParentControl()->_hwndControl);

	SelectObject(hdc, GetStockObject(SYSTEM_FONT));

	TEXTMETRIC tm;
	GetTextMetrics(hdc, &tm);

	int xWidthSys = (INT)tm.tmAveCharWidth;
	int yHeightSys = (INT)tm.tmHeight;
	int xPerInch = GetDeviceCaps(hdc, LOGPIXELSX);
	int yPerInch = GetDeviceCaps(hdc, LOGPIXELSY);

	ReleaseDC(GetParentControl()->_hwndControl, hdc);


	_sizelExtent.cx = DXtoHimetricX(w, xPerInch);
	_sizelExtent.cy = DYtoHimetricY(h, yPerInch);

	*lpExtent = _sizelExtent;

	return S_OK;
}

HRESULT CSTXAnimationControlEdit::OnTxCharFormatChange(const CHARFORMATW * pcf)
{
	memcpy(&_cf, pcf, pcf->cbSize);
	return S_OK;
}

HRESULT	CSTXAnimationControlEdit::OnTxParaFormatChange(const PARAFORMAT * ppf)
{
	memcpy(&_pf, ppf, ppf->cbSize);
	return S_OK;
}

HRESULT	CSTXAnimationControlEdit::TxGetPropertyBits(DWORD dwMask, DWORD *pdwBits)
{
	DWORD bits = TXTBIT_MULTILINE | TXTBIT_RICHTEXT | TXTBIT_WORDWRAP;
	*pdwBits = bits & dwMask;
	return S_OK;
}

HRESULT	CSTXAnimationControlEdit::TxNotify(DWORD iNotify, void *pv)
{
	return S_OK;
}

HIMC CSTXAnimationControlEdit::TxImmGetContext()
{
	HIMC himc;

	himc = ImmGetContext(GetParentControl()->_hwndControl);
	COMPOSITIONFORM cf;
	cf.dwStyle = CFS_POINT;
	cf.ptCurrentPos.x = 200;
	cf.ptCurrentPos.y = 300;
	GetCaretPos(&cf.ptCurrentPos);
	ImmSetCompositionWindow(himc, &cf);
	return himc;
//	return NULL;
}

void CSTXAnimationControlEdit::TxImmReleaseContext(HIMC himc)
{
	ImmReleaseContext(GetParentControl()->_hwndControl, himc);

}

HRESULT	CSTXAnimationControlEdit::TxGetSelectionBarWidth(LONG *lSelBarWidth)
{
	*lSelBarWidth = 1;
	return S_OK;
}

HDC CSTXAnimationControlEdit::TxGetDC()
{
	return ::GetDC(GetParentControl()->_hwndControl);
}

INT CSTXAnimationControlEdit::TxReleaseDC(HDC hdc)
{
	return ReleaseDC(GetParentControl()->_hwndControl, hdc);
}

BOOL CSTXAnimationControlEdit::TxShowScrollBar(INT fnBar, BOOL fShow)
{
	return FALSE;
}

BOOL CSTXAnimationControlEdit::TxEnableScrollBar(INT fuSBFlags, INT fuArrowflags)
{
	return FALSE;
}

BOOL CSTXAnimationControlEdit::OnSetCursor(HWND hWnd, int x, int y, UINT nHitTest, UINT message)
{
	__super::OnSetCursor(hWnd, x, y, nHitTest, message);

	return TRUE;

	//LRESULT res;
	//_textService->TxSendMessage(WM_SETCURSOR, (WPARAM)hWnd, MAKELPARAM(nHitTest, message), &res);
	//return res;
}

void CSTXAnimationControlEdit::OnMouseMove(int x, int y, UINT nFlags)
{
	__super::OnMouseMove(x, y, nFlags);

	POINT pt = { x, y };
	POINT ptLocal = ConvertToNodeLocalPoint(pt);
	ptLocal = pt;

	LRESULT res;
	_textService->TxSendMessage(WM_MOUSEMOVE, (WPARAM)nFlags, MAKELONG(ptLocal.x, ptLocal.y), &res);

	GetParentControl()->RedrawWindow();

}

void CSTXAnimationControlEdit::OnLButtonDown(int x, int y, UINT nFlags, BOOL bForRButton /*= FALSE*/)
{
	__super::OnLButtonDown(x, y, nFlags, bForRButton);

	POINT pt = { x, y };
	POINT ptLocal = ConvertToNodeLocalPoint(pt);
	ptLocal = pt;

	LRESULT res;
	_textService->TxSendMessage(WM_LBUTTONDOWN, (WPARAM)nFlags, MAKELONG(ptLocal.x, ptLocal.y), &res);

	GetParentControl()->SetCurrentFocusNode(this);
}

void CSTXAnimationControlEdit::OnLButtonUp(int x, int y, UINT nFlags, BOOL bForRButton /*= FALSE*/)
{
	__super::OnLButtonUp(x, y, nFlags, bForRButton);

	POINT pt = { x, y };
	POINT ptLocal = ConvertToNodeLocalPoint(pt);
	ptLocal = pt;

	LRESULT res;
	_textService->TxSendMessage(WM_LBUTTONUP, (WPARAM)nFlags, MAKELONG(ptLocal.x, ptLocal.y), &res);

}

LRESULT CSTXAnimationControlEdit::OnPreWndProc(int x, int y, HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT res = 0;
	if (uMsg != WM_LBUTTONDOWN
		&& uMsg != WM_LBUTTONUP
		&& uMsg != WM_MOUSEMOVE)
	{
		if (uMsg == WM_CHAR || uMsg == WM_LBUTTONDBLCLK
			|| uMsg == WM_KEYDOWN || uMsg == WM_KEYUP)
		{
			if (GetParentControl()->_focusNode == this)
			{
				_textService->TxSendMessage(uMsg, wParam, lParam, &res);
				GetParentControl()->RedrawWindow();
			}
			
		}
		else
		{
			_textService->TxSendMessage(uMsg, wParam, lParam, &res);
		}
	}


	if (uMsg == WM_PAINT)
	{
		GetParentControl()->RedrawWindow();
	}
	
	return res;
}

#define LY_PER_INCH   1440

HRESULT CSTXAnimationControlEdit::InitDefaultCharFormat(CHARFORMATW * pcf, HFONT hfont)
{
	HWND hwnd;
	LOGFONT lf;
	HDC hdc;
	LONG yPixPerInch;

	// Get LOGFONT for default font
	if (!hfont)
		hfont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

	// Get LOGFONT for passed hfont
	if (!GetObject(hfont, sizeof(LOGFONT), &lf))
		return E_FAIL;

	// Set CHARFORMAT structure
	pcf->cbSize = sizeof(CHARFORMAT2);
	CHARFORMAT2* pcf2 = (CHARFORMAT2*)pcf;

	hwnd = GetDesktopWindow();
	hdc = GetDC(hwnd);
	yPixPerInch = GetDeviceCaps(hdc, LOGPIXELSY);
	pcf->yHeight = lf.lfHeight * LY_PER_INCH / yPixPerInch;
	ReleaseDC(hwnd, hdc);

	pcf->yOffset = 0;
	//pcf2->crTextColor = RGB(0,0,255);
	//pcf2->crBackColor = RGB(255, 0, 0);

	pcf->dwEffects = CFM_EFFECTS | CFE_AUTOBACKCOLOR | CFE_AUTOCOLOR;
	pcf->dwEffects &= ~(CFE_PROTECTED | CFE_LINK);

	if (lf.lfWeight < FW_BOLD)
		pcf->dwEffects &= ~CFE_BOLD;
	if (!lf.lfItalic)
		pcf->dwEffects &= ~CFE_ITALIC;
	if (!lf.lfUnderline)
		pcf->dwEffects &= ~CFE_UNDERLINE;
	if (!lf.lfStrikeOut)
		pcf->dwEffects &= ~CFE_STRIKEOUT;

	pcf2->dwMask = CFM_ALL;
	pcf->bCharSet = lf.lfCharSet;
	pcf->bPitchAndFamily = lf.lfPitchAndFamily;
#ifdef UNICODE
	_tcscpy_s(pcf->szFaceName, lf.lfFaceName);
#else
	//need to thunk pcf->szFaceName to a standard char string.in this case it's easy because our thunk is also our copy
	MultiByteToWideChar(CP_ACP, 0, lf.lfFaceName, LF_FACESIZE, pcf->szFaceName, LF_FACESIZE);
#endif

	return S_OK;
}

HRESULT CSTXAnimationControlEdit::InitDefaultParaFormat(PARAFORMAT * ppf)
{
	memset(ppf, 0, sizeof(PARAFORMAT));

	ppf->cbSize = sizeof(PARAFORMAT);
	ppf->dwMask = PFM_ALL;
	ppf->wAlignment = PFA_LEFT;
	ppf->cTabCount = 1;
	ppf->rgxTabs[0] = lDefaultTab;

	return S_OK;
}

void CSTXAnimationControlEdit::OnAddedToNode(CSTXAnimationControlChildNode *pNodeParent)
{
	ANI_VAR_FINAL_VALUE_INT(x, x);
	ANI_VAR_FINAL_VALUE_INT(y, y);
	ANI_VAR_FINAL_VALUE_INT(w, width);
	ANI_VAR_FINAL_VALUE_INT(h, height);

	static int i = 0;
	i++;
	RECT rect = {x, y, x+w, y+h };
	_textService->OnTxInPlaceActivate(&rect);
	//SetFocus(::GetParent(GetParentControl()->_hwndControl));
	//SetFocus(GetParentControl()->_hwndControl);

	//_textService->TxSetText(_T("Retrieve the IID_ITextServices interface identifier from Msftedit.dll."));
	//_textService->TxSetText(_T("{\\rtf\\ansi\\deff0{\\fonttbl{\\f0\\froman Tms Rmn;}{\\f1\\fdecorSymbol;}{\\f2\\fswiss Helv; }}{\\colortbl; \\red0\\green0\\blue0;\\red0\\green0\\blue255; \\red0\\green255\\blue255; \\red0\\green255\\blue0; \\red255\\green0\\blue255; \\red255\\green0\\blue0; \\red255\\green255\\blue0; \\red255\\green255\\blue255; }{\\stylesheet{ \\fs20\\snext0Normal; }}{\\info{ \\author John Doe }{\\creatim\\yr1990\\mo7\\dy30\\hr10\\min48}{\\version1}{\\edmins0}{\\nofpages1}{\\nofwords0}{\\nofchars0}{\\vern8351}}\\widoctrl\\ftnbj \\sectd\\linex0\\endnhere \\pard\\plain \\fs20 This is plain text.\\par}"));

}

void CSTXAnimationControlEdit::DrawNode(CSTXGraphics *pGraphics)
{
	ANI_VAR_VALUE_INT(x, x);
	ANI_VAR_VALUE_INT(y, y);
	ANI_VAR_VALUE_INT(w, width);
	ANI_VAR_VALUE_INT(h, height);
	CSTXGraphicsPen *pPen = pGraphics->CreateDrawingPen(255, 0, 0, 255);

	int nPadding = 2;

	pGraphics->DrawRoundedRectangle(x, y, w, h, 2, pPen);
	pPen->Release();

	if (GetTextLength() > 0)
		pGraphics->DrawRichText(_textService, x + nPadding, y + nPadding, w - 2 * nPadding, h - 2 * nPadding);
	else
	{
		CSTXGraphicsBrush *pBrushFont = pGraphics->CreateSolidBrush(128, 128, 128, static_cast<byte>(255 * GetAncestorOpacity()));
		CSTXGraphicsFont *pFont = pGraphics->CreateDrawingFont(_T("Arial"), 12, TRUE, FALSE);
		if (_tipText.size() > 0)
		{
			CSTXGraphicsTextFormat tf;
			tf._alignment = 0; //Left
			pGraphics->DrawString(_tipText.c_str(), x + nPadding, y + nPadding, w - 2 * nPadding, h - 2 * nPadding, pFont, pBrushFont, &tf);
		}
	}
}

void CSTXAnimationControlEdit::SetMaxLength(DWORD dwMaxLength)
{
	_textMaxLength = dwMaxLength;

	LRESULT res = 0;
	_textService->TxSendMessage(EM_EXLIMITTEXT, 0, dwMaxLength, &res);
}

DWORD CSTXAnimationControlEdit::GetTextLength()
{
	GETTEXTLENGTHEX lengthEx;
#ifdef UNICODE
	lengthEx.codepage = 1200;		//Unicode
#else
	lengthEx.codepage = CP_ACP;
#endif
	lengthEx.flags = GTL_DEFAULT;
	
	LRESULT res = 0;
	_textService->TxSendMessage(EM_GETTEXTLENGTHEX, (WPARAM)&lengthEx, 0, &res);
	return res;
}

void CSTXAnimationControlEdit::GetText(LPTSTR pBuffer, UINT cchBufferLength)
{
	GETTEXTEX gt;
	gt.cb = cchBufferLength * sizeof(TCHAR);
#ifdef UNICODE
	gt.codepage = 1200;		//Unicode
#else
	gt.codepage = CP_ACP;
#endif
	gt.flags = GT_DEFAULT;
	gt.lpDefaultChar = NULL;
	gt.lpUsedDefChar = NULL;
	LRESULT res = 0;
	_textService->TxSendMessage(EM_GETTEXTEX, (WPARAM)&gt, (LPARAM)pBuffer, &res);

}

void CSTXAnimationControlEdit::SetText(LPCTSTR lpszText)
{
	_textService->TxSetText(lpszText);
}

void CSTXAnimationControlEdit::SetTipText(LPCTSTR lpszTipText)
{
	if (lpszTipText)
		_tipText = lpszTipText;
}


//////////////////////////////////////////////////////////////////////////

CSTXAnimationControl::CSTXAnimationControl()
{
}

CSTXAnimationControl::~CSTXAnimationControl()
{

}
//
//void CSTXAnimationControl::InitializeChildNode(CSTXAnimationControlChildNode *pNode, std::map<std::wstring, DOUBLE> *initialValues)
//{
//	if (initialValues)
//	{
//		std::map<std::wstring, DOUBLE>::iterator itInitial = initialValues->begin();
//		for (; itInitial != initialValues->end(); itInitial++)
//		{
//			CComPtr<IUIAnimationVariable> spVar;
//			_animationManager.CreateAnimationVariable(itInitial->second, &spVar);
//			pNode->_animationVariables[itInitial->first] = spVar;
//		}
//	}
//
//	std::vector<std::pair<std::wstring, double>> arrInitialVariables;
//	arrInitialVariables.push_back(std::pair<std::wstring, double>(_T("x"), 0));
//	arrInitialVariables.push_back(std::pair<std::wstring, double>(_T("y"), 0));
//	arrInitialVariables.push_back(std::pair<std::wstring, double>(_T("width"), 1));
//	arrInitialVariables.push_back(std::pair<std::wstring, double>(_T("height"), 1));
//
//	std::vector<std::pair<std::wstring, double>>::iterator it = arrInitialVariables.begin();
//	for (; it != arrInitialVariables.end(); it++)
//	{
//		if (initialValues && initialValues->find(it->first) != initialValues->end())
//			continue;
//
//		CComPtr<IUIAnimationVariable> spVar;
//		_animationManager.CreateAnimationVariable(it->second, &spVar);
//		pNode->_animationVariables[it->first] = spVar;
//	}
//
//	pNode->_parentControl = this;
//}
//
//void CSTXAnimationControl::UpdateAnimationManager()
//{
//	UI_ANIMATION_UPDATE_RESULT result;
//	UI_ANIMATION_SECONDS secTime;
//	_animationTimer->GetTime(&secTime);
//	_animationManager->Update(secTime, &result);
//}


//
//HRESULT CSTXAnimationControl::OnManagerStatusChanged(UI_ANIMATION_MANAGER_STATUS newStatus, UI_ANIMATION_MANAGER_STATUS previousStatus)
//{
//	if (newStatus == UI_ANIMATION_MANAGER_BUSY)
//	{
//		StartAnimationTimer();
//	}
//	else
//	{
//		StopAnimationTimer();
//		ClearPendingDeleteNodes();
//		InvalidateRect(_hwndControl, NULL, TRUE);
//	}
//
//	return S_OK;
//}

//void CSTXAnimationControl::OnMouseMove(int x, int y, UINT nFlags)
//{
//	int iHScrollPos = GetScrollPos(_hwndControl, SB_HORZ);
//	int iVScrollPos = GetScrollPos(_hwndControl, SB_VERT);
//
//	CSTXAnimationControlWindow::OnMouseMove(x + iHScrollPos, y + iVScrollPos, nFlags);
//	CSTXAnimationControlChildNode::OnMouseMove(x + iHScrollPos, y + iVScrollPos, nFlags);
//
//}
//
//void CSTXAnimationControl::OnLButtonDown(int x, int y, UINT nFlags, BOOL bForRButton /*= FALSE*/)
//{
//	int iHScrollPos = GetScrollPos(_hwndControl, SB_HORZ);
//	int iVScrollPos = GetScrollPos(_hwndControl, SB_VERT);
//
//	CSTXAnimationControlWindow::OnLButtonDown(x + iHScrollPos, y + iVScrollPos, nFlags, bForRButton);
//	CSTXAnimationControlChildNode::OnLButtonDown(x + iHScrollPos, y + iVScrollPos, nFlags, bForRButton);
//}

//void CSTXAnimationControl::OnLButtonUp(int x, int y, UINT nFlags, BOOL bForRButton /*= FALSE*/)
//{
//	int iHScrollPos = GetScrollPos(_hwndControl, SB_HORZ);
//	int iVScrollPos = GetScrollPos(_hwndControl, SB_VERT);
//
//	CSTXAnimationControlWindow::OnLButtonUp(x + iHScrollPos, y + iVScrollPos, nFlags, bForRButton);
//	CSTXAnimationControlChildNode::OnLButtonUp(x + iHScrollPos, y + iVScrollPos, nFlags, bForRButton);
//
//	if (_mouseDownNode)
//	{
//		_mouseDownNode->OnLButtonUp(x + iHScrollPos, y + iVScrollPos, nFlags, bForRButton);
//		_mouseDownNode->Release();
//		_mouseDownNode = NULL;
//	}
//}

//void CSTXAnimationControl::OnMouseLeave()
//{
//	CSTXAnimationControlWindow::OnMouseLeave();
//	CSTXAnimationControlChildNode::OnMouseLeave();
//}

//void CSTXAnimationControl::OnRButtonDown(int x, int y, UINT nFlags)
//{
//	CSTXAnimationControlWindow::OnRButtonDown(x, y, nFlags);
//	CSTXAnimationControlChildNode::OnRButtonDown(x, y, nFlags);
//
//}
//
//void CSTXAnimationControl::OnRButtonUp(int x, int y, UINT nFlags)
//{
//	CSTXAnimationControlWindow::OnRButtonUp(x, y, nFlags);
//	CSTXAnimationControlChildNode::OnRButtonUp(x, y, nFlags);
//
//	if (_mouseDownNode)
//	{
//		_mouseDownNode->OnRButtonUp(x, y, nFlags);
//		_mouseDownNode->Release();
//		_mouseDownNode = NULL;
//	}
//}

//void CSTXAnimationControl::OnSize(UINT nType, int cx, int cy)
//{
//	CSTXAnimationControlWindow::OnSize(nType, cx, cy);
//	CSTXAnimationControlChildNode::OnSize(nType, cx, cy);
//}

//void CSTXAnimationControl::OnLButtonDblClk(int x, int y, UINT nFlags)
//{
//	int iHScrollPos = GetScrollPos(_hwndControl, SB_HORZ);
//	int iVScrollPos = GetScrollPos(_hwndControl, SB_VERT);
//
//	CSTXAnimationControlWindow::OnLButtonDblClk(x + iHScrollPos, y + iVScrollPos, nFlags);
//	CSTXAnimationControlChildNode::OnLButtonDblClk(x + iHScrollPos, y + iVScrollPos, nFlags);
//}

//void CSTXAnimationControl::ResetScrollBars()
//{
//	if (!IsWindow(_hwndControl))
//		return;
//
//	RECT rcClient;
//	::GetClientRect(_hwndControl, &rcClient);
//
//	//Vertical Scroll Bar
//	int iTotalHeightAvailable = rcClient.bottom - rcClient.top;
//
//	int iCurPos = 0;
//	BOOL bVScrollExist;
//	if ((GetWindowLong(_hwndControl, GWL_STYLE) & WS_VSCROLL) == WS_VSCROLL)
//	{
//		iCurPos = ::GetScrollPos(_hwndControl, SB_VERT);
//		bVScrollExist = TRUE;
//	}
//	else
//		bVScrollExist = FALSE;
//
//	int iOldPos = ::GetScrollPos(_hwndControl, SB_VERT);
//
//	int nTotalHeight = static_cast<int>(OnQueryNodeFinalHeight());
//	if (_childNodes.size() > 0 && nTotalHeight > iTotalHeightAvailable)	//Need H-ScrollBar
//	{
//		SCROLLINFO si;
//		si.cbSize = sizeof(si);
//		si.fMask = SIF_PAGE | SIF_POS | SIF_RANGE;
//		si.nPage = iTotalHeightAvailable;
//		si.nMin = 0;
//		si.nMax = nTotalHeight;
//		si.nPos = min(iCurPos, si.nMax);
//
//		SetScrollPos(_hwndControl, SB_VERT, si.nPos, FALSE);
//		SetScrollInfo(_hwndControl, SB_VERT, &si, TRUE);
//		::ShowScrollBar(_hwndControl, SB_VERT, TRUE);
//		ModifyStyle(0, WS_VSCROLL);
//	}
//	else
//	{
//		int iCurPos = GetScrollPos(_hwndControl, SB_VERT);
//		//ScrollWindow(_hwndControl, 0, iCurPos, NULL, NULL);
//		SetScrollPos(_hwndControl, SB_VERT, 0, TRUE);
//		::ShowScrollBar(_hwndControl, SB_VERT, FALSE);
//		ModifyStyle(WS_VSCROLL, 0);
//	}
//
//	//Horizontal Scroll Bar
//	ResetHorizontalScrollBar();
//
//	InvalidateRect(_hwndControl, NULL, FALSE);
//
//}
//
//void CSTXAnimationControl::ResetHorizontalScrollBar()
//{
//
//}
//
//int CSTXAnimationControl::AddChildNode(CSTXAnimationControlChildNode *pNode)
//{
//	int nResult = __super::AddChildNode(pNode);
//
//	DOUBLE fHeightTotal = 0;
//	if (_style.autoCalculateContentSize)
//	{
//		std::vector<CSTXAnimationControlChildNode*>::iterator it = _childNodes.begin();
//		for (; it != _childNodes.end(); it++)
//		{
//			DOUBLE y = (*it)->GetFinalValue(_T("y"));
//			DOUBLE h = (*it)->GetFinalValue(_T("height"));
//
//			if (fHeightTotal < y + h)
//				fHeightTotal = y + h;
//		}
//
//		SetValueInstantly(_T("height"), fHeightTotal);
//	}
//
//	ResetScrollBars();
//	return nResult;
//}

//void CSTXAnimationControl::OnDestroy()
//{
//#if _WIN32_WINNT >= 0x0601
//	CSTXD2DGraphics::Clear(_hwndControl);
//#endif
//	CSTXGraphics::ClearCachedGraphicsObjects(_hwndControl);
//}

//void CSTXAnimationControl::OnQueryToolTipsText(LPTSTR pszBuffer, int cchBufferSize, POINT ptLocation, LPCTSTR *ppszToolTips)
//{
//	if (_mouseEnterNode)
//	{
//		_mouseEnterNode->OnQueryToolTipsText(pszBuffer, cchBufferSize, ptLocation, ppszToolTips);
//	}
//}

//void CSTXAnimationControl::ClearPendingDeleteNodes()
//{
//	__super::ClearPendingDeleteNodes();
//}



