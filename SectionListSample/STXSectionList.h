#pragma once

#include "STXAnimationControl.h"

//////////////////////////////////////////////////////////////////////////

class CSTXSectionListTitleNode : public CSTXAnimationControlChildNode
{
public:
	std::wstring _caption;

public:
	static CSTXGraphicsBrush* _sbkBrush[1];

protected:
	virtual void DrawNode(CSTXGraphics *pGraphics);
	int GetTotalHeight();
	virtual BOOL IsDelayRemove();
};

class CSTXSectionListContentNode : public CSTXAnimationControlChildNode
{
public:
	CSTXSectionListContentNode();
	virtual ~CSTXSectionListContentNode();

public:
	std::wstring _caption;
	int _type;

protected:
	CSTXGraphicsBrush* _bkBrush;


protected:
	virtual void DrawNode(CSTXGraphics *pGraphics);
	virtual BOOL IsDelayRemove();

protected:
	virtual void OnLButtonDown(int x, int y, UINT nFlags, BOOL bForRButton = FALSE);
	virtual void OnLButtonUp(int x, int y, UINT nFlags, BOOL bForRButton = FALSE);
	virtual void OnMouseMove(int x, int y, UINT nFlags);
	virtual void OnMouseLeave();
	virtual void OnLButtonClick();
	virtual void OnRButtonClick();
	virtual void OnLButtonDblClk(int x, int y, UINT nFlags);
	virtual void OnMouseEnter();
	virtual void OnQueryToolTipsText(LPTSTR pszBuffer, int cchBufferSize, POINT ptLocation, LPCTSTR *ppszToolTips);
	virtual void OnBeginMouseDragging(POINT ptGlobal);
	virtual void OnEndMouseDragging(POINT ptGlobal, POINT ptOffset);

};

class CSTXSectionListTipNode : public CSTXAnimationControlChildNode
{
public:
	std::wstring _caption;
	int _location;	//0 to 3

protected:
	virtual void DrawNode(CSTXGraphics *pGraphics);
	virtual void OnQueryToolTipsText(LPTSTR pszBuffer, int cchBufferSize, POINT ptLocation, LPCTSTR *ppszToolTips);
};


//////////////////////////////////////////////////////////////////////////

class CSTXSectionList :	public CSTXAnimationControl
{
public:
	CSTXSectionList();
	virtual ~CSTXSectionList();

public:
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) { return E_NOTIMPL; }
	virtual ULONG STDMETHODCALLTYPE AddRef(void) { return 1; }
	virtual ULONG STDMETHODCALLTYPE Release(void) { return 1; }

protected:
	void CalculateItemsPerLine();
	POINT CalculateItemLocation(int nSectionIndex, int nItemIndex);
	void ResetItemLocationsInSectionFromIndex(int nSectionIndex, int nItemIndexStart);
	int CalculateLineCount(int nSectionItemCount);
	int CalculateLineCount(int nSectionItemCount, int nItemsPerLine);
protected:
	int _itemsPerLine;

protected:
	virtual int OnQueryNodeFinalHeight();
	virtual LPCTSTR GetControlClassName();

protected:
	virtual void OnSize(UINT nType, int cx, int cy);


public:
	int GetTotalHeightBefore(int nSectionIndex);
	int AddSection(LPCTSTR lpszTitle);
	void RemoveSection(int nSectionIndex);
	int AddItemToSection(int nSectionIndex, LPCTSTR lpszItemText);
	void AdjustSectionLocationsFromIndex(int nSectionIndex, int nOffsetChanged);
	void SetItemTipCount(int nSectionIndex, int nItemIndex, int nCount);
	void SetItemTipLeftBottom(int nSectionIndex, int nItemIndex, LPCTSTR lpszTipText);
	void SetItemTipLeftTop(int nSectionIndex, int nItemIndex, LPCTSTR lpszTipText);
	void RemoveItemFromSection(int nSectionIndex, int nItemIndex);
};

