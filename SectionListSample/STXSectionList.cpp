#include "STXSectionList.h"

#define STXBOOLINGLIST_SECTION_CAPTION_HEIGHT		26
#define STXBOOLINGLIST_SECTION_SPACING				22
#define STXBOOLINGLIST_MARGIN						4
#define STXBOOLINGLIST_CONTENT_MARGIN				8
#define STXBOOLINGLIST_CONTENT_SIZE					96
#define STXBOOLINGLIST_CONTENT_SPACING				20

#define SAFE_DELETE_PTR(xPtr)\
	if(xPtr)\
	{\
		delete xPtr;\
		xPtr = NULL;\
	}

CSTXSectionList::CSTXSectionList()
{
	_itemsPerLine = 1;
}


CSTXSectionList::~CSTXSectionList()
{
}

int CSTXSectionList::AddSection(LPCTSTR lpszTitle)
{
	RECT rcControl;
	::GetClientRect(_hwndControl, &rcControl);

	CSTXSectionListTitleNode * pSectionNode = new CSTXSectionListTitleNode();
	pSectionNode->_caption = lpszTitle;
	std::map<std::wstring, DOUBLE> mapInit;
	mapInit[_T("x")] = STXBOOLINGLIST_MARGIN + rcControl.right;
	mapInit[_T("y")] = STXBOOLINGLIST_MARGIN + GetTotalHeightBefore(_childNodes.size()) + _childNodes.size() * STXBOOLINGLIST_SECTION_SPACING;
	mapInit[_T("width")] = rcControl.right - STXBOOLINGLIST_MARGIN * 2;
	mapInit[_T("height")] = STXBOOLINGLIST_SECTION_CAPTION_HEIGHT;
	mapInit[_T("captionHeight")] = STXBOOLINGLIST_SECTION_CAPTION_HEIGHT;
	mapInit[_T("opacity")] = 0;
	mapInit[_T("scale")] = 1;
	InitializeChildNode(pSectionNode, &mapInit);
	pSectionNode->BeginSetValue();
	pSectionNode->SetValue(_T("width"), rcControl.right - STXBOOLINGLIST_MARGIN * 2);
	pSectionNode->SetValue(_T("x"), STXBOOLINGLIST_MARGIN);
	pSectionNode->SetValue(_T("opacity"), 1);
	pSectionNode->EndSetValue();

	int nResult = AddChildNode(pSectionNode);
	pSectionNode->Release();
	return nResult;
}

int CSTXSectionList::AddItemToSection(int nSectionIndex, LPCTSTR lpszItemText)
{
	if (nSectionIndex < 0 || nSectionIndex >= (int)_childNodes.size())
		return -1;

	CSTXAnimationControlChildNode *pSectionNode = _childNodes[nSectionIndex];
	RECT rcControl;
	::GetClientRect(_hwndControl, &rcControl);

	DOUBLE fCaptionHeight = pSectionNode->GetFinalValue(_T("captionHeight"));

	CSTXSectionListContentNode *pContentNode = new CSTXSectionListContentNode();
	pContentNode->_caption = lpszItemText;
	pContentNode->_type = rand() % 3;

	int nChildrenCount = pSectionNode->GetChildrenCount();
	std::map<std::wstring, DOUBLE> mapInit;
	mapInit[_T("x")] = STXBOOLINGLIST_CONTENT_MARGIN + (nChildrenCount % _itemsPerLine) * (STXBOOLINGLIST_CONTENT_SIZE + STXBOOLINGLIST_CONTENT_SPACING);
	mapInit[_T("y")] = fCaptionHeight + STXBOOLINGLIST_CONTENT_MARGIN + (nChildrenCount / _itemsPerLine) * (STXBOOLINGLIST_CONTENT_SIZE + STXBOOLINGLIST_CONTENT_SPACING);
	mapInit[_T("width")] = STXBOOLINGLIST_CONTENT_SIZE;
	mapInit[_T("height")] = STXBOOLINGLIST_CONTENT_SIZE;
	mapInit[_T("scale")] = 0.05;
	mapInit[_T("rotate")] = 0;
	mapInit[_T("opacity")] = 1;
	InitializeChildNode(pContentNode, &mapInit);
	pContentNode->SetValue(_T("scale"), 1);

	int nOldLineCount = CalculateLineCount(nChildrenCount);

	int nAddedIndex = pSectionNode->AddChildNode(pContentNode);
	pContentNode->Release();

	int nLineCount = CalculateLineCount(nChildrenCount + 1);

	if (nLineCount != nOldLineCount)
	{
		pSectionNode->BeginSetValue();
		DOUBLE fSectionHeight = pSectionNode->GetFinalValue(_T("height"));
		DOUBLE fSectionDstHeight = STXBOOLINGLIST_SECTION_CAPTION_HEIGHT + STXBOOLINGLIST_CONTENT_MARGIN + nLineCount * (STXBOOLINGLIST_CONTENT_SIZE + STXBOOLINGLIST_CONTENT_SPACING) - STXBOOLINGLIST_CONTENT_SPACING;
		if (nLineCount == 0)
			fSectionDstHeight = STXBOOLINGLIST_SECTION_CAPTION_HEIGHT;

		pSectionNode->SetValue(_T("height"), fSectionDstHeight);

		AdjustSectionLocationsFromIndex(nSectionIndex, static_cast<int>(fSectionDstHeight - fSectionHeight));
		pSectionNode->EndSetValue();
	}

	ResetScrollBars();
	return nAddedIndex;
}

void CSTXSectionList::AdjustSectionLocationsFromIndex(int nSectionIndex, int nOffsetChanged)
{
	std::vector<CSTXAnimationControlChildNode*>::iterator it = _childNodes.begin() + nSectionIndex + 1;
	for (; it != _childNodes.end(); it++)
	{
		DOUBLE yOrg = (*it)->GetFinalValue(_T("y"));
		(*it)->SetValue(_T("y"), yOrg + nOffsetChanged);
	}
}

int CSTXSectionList::GetTotalHeightBefore(int nSectionIndex)
{
	int nCurIndex = 0;
	DOUBLE hTotal = 0;
	std::vector<CSTXAnimationControlChildNode*>::iterator it = _childNodes.begin();
	for (; it != _childNodes.end() && nCurIndex < nSectionIndex; it++, nCurIndex++)
	{
		DOUBLE h = (*it)->GetFinalValue(_T("height"));
		hTotal += h;
	}
	return static_cast<int>(hTotal);
}

void CSTXSectionList::SetItemTipCount(int nSectionIndex, int nItemIndex, int nCount)
{
	TCHAR szText[64];
	_itot_s(nCount, szText, 10);
	CSTXSectionListContentNode *pContentNode = dynamic_cast<CSTXSectionListContentNode*>(GetChildNodeAtIndex(nSectionIndex)->GetChildNodeAtIndex(nItemIndex));

	int nChildrenCount = pContentNode->GetChildrenCount();
	if (nChildrenCount > 0)
	{
		for (int i = 0; i < nChildrenCount; i++)
		{
			CSTXSectionListTipNode *pTipsNode = dynamic_cast<CSTXSectionListTipNode*>(pContentNode->GetChildNodeAtIndex(i));
			if (pTipsNode && pTipsNode->_location == 1)
			{
				pTipsNode->_caption = szText;
				InvalidateRect(_hwndControl, NULL, TRUE);
				return;
			}
		}
	}

	CSTXSectionListTipNode *pTipsNode = new CSTXSectionListTipNode();
	pTipsNode->_caption = szText;
	pTipsNode->_location = 1;	//Right-Top corner

	DOUBLE fWidth = pContentNode->GetFinalValue(_T("width"));

	std::map<std::wstring, DOUBLE> mapInit;
	mapInit[_T("x")] = fWidth - 58;
	mapInit[_T("y")] = -2;
	mapInit[_T("width")] = 1;
	mapInit[_T("height")] = 1;
	InitializeChildNode(pTipsNode, &mapInit);
	pTipsNode->SetValue(_T("width"), 60);
	pTipsNode->SetValue(_T("height"), 18);

	pContentNode->AddChildNode(pTipsNode);
	pTipsNode->Release();
}

void CSTXSectionList::SetItemTipLeftBottom(int nSectionIndex, int nItemIndex, LPCTSTR lpszTipText)
{
	CSTXSectionListContentNode *pContentNode = dynamic_cast<CSTXSectionListContentNode*>(GetChildNodeAtIndex(nSectionIndex)->GetChildNodeAtIndex(nItemIndex));

	int nChildrenCount = pContentNode->GetChildrenCount();
	if (nChildrenCount > 0)
	{
		for (int i = 0; i < nChildrenCount; i++)
		{
			CSTXSectionListTipNode *pTipsNode = dynamic_cast<CSTXSectionListTipNode*>(pContentNode->GetChildNodeAtIndex(i));
			if (pTipsNode && pTipsNode->_location == 2)
			{
				pTipsNode->_caption = lpszTipText;
				InvalidateRect(_hwndControl, NULL, TRUE);
				return;
			}
		}
	}

	CSTXSectionListTipNode *pTipsNode = new CSTXSectionListTipNode();
	pTipsNode->_caption = lpszTipText;
	pTipsNode->_location = 2;	//Right-Top corner

	DOUBLE fHeight = pContentNode->GetFinalValue(_T("height"));

	std::map<std::wstring, DOUBLE> mapInit;
	mapInit[_T("x")] = -2;
	mapInit[_T("y")] = fHeight - 16;
	mapInit[_T("width")] = 1;
	mapInit[_T("height")] = 1;
	InitializeChildNode(pTipsNode, &mapInit);
	pTipsNode->SetValue(_T("width"), 60);
	pTipsNode->SetValue(_T("height"), 18);

	pContentNode->AddChildNode(pTipsNode);
	pTipsNode->Release();
}

void CSTXSectionList::SetItemTipLeftTop(int nSectionIndex, int nItemIndex, LPCTSTR lpszTipText)
{
	CSTXSectionListContentNode *pContentNode = dynamic_cast<CSTXSectionListContentNode*>(GetChildNodeAtIndex(nSectionIndex)->GetChildNodeAtIndex(nItemIndex));

	int nChildrenCount = pContentNode->GetChildrenCount();
	if (nChildrenCount > 0)
	{
		for (int i = 0; i < nChildrenCount; i++)
		{
			CSTXSectionListTipNode *pTipsNode = dynamic_cast<CSTXSectionListTipNode*>(pContentNode->GetChildNodeAtIndex(i));
			if (pTipsNode && pTipsNode->_location == 0)
			{
				pTipsNode->_caption = lpszTipText;
				InvalidateRect(_hwndControl, NULL, TRUE);
				return;
			}
		}
	}

	CSTXSectionListTipNode *pTipsNode = new CSTXSectionListTipNode();
	pTipsNode->_caption = lpszTipText;
	pTipsNode->_location = 0;

	DOUBLE fHeight = pContentNode->GetFinalValue(_T("height"));

	std::map<std::wstring, DOUBLE> mapInit;
	mapInit[_T("x")] = -2;
	mapInit[_T("y")] = -2;
	mapInit[_T("width")] = 1;
	mapInit[_T("height")] = 1;
	InitializeChildNode(pTipsNode, &mapInit);
	pTipsNode->SetValue(_T("width"), 26);
	pTipsNode->SetValue(_T("height"), 26);

	pContentNode->AddChildNode(pTipsNode);
	pTipsNode->Release();
}

int CSTXSectionList::OnQueryNodeFinalHeight()
{
	if (_childNodes.size() == 0)
		return 0;

	CSTXAnimationControlChildNode *pLastNode = _childNodes[_childNodes.size() - 1];
	DOUBLE y = pLastNode->GetFinalValue(_T("y"));
	DOUBLE h = pLastNode->GetFinalValue(_T("height"));

	return static_cast<int>(y + h + STXBOOLINGLIST_MARGIN);
}

LPCTSTR CSTXSectionList::GetControlClassName()
{
	return _T("CSTXSectionList");
}

void CSTXSectionList::RemoveSection(int nSectionIndex)
{
	if (nSectionIndex < 0 || nSectionIndex >= (int)_childNodes.size())
		return;


	CSTXAnimationControlChildNode *pSectionNode = _childNodes[nSectionIndex];
	pSectionNode->SetValue(_T("x"), -500, 0.3);
	//pSectionNode->SetValue(_T("scale"), 2.0);
	pSectionNode->SetValue(_T("opacity"), 0, 0.3);

	AdjustSectionLocationsFromIndex(nSectionIndex, -(pSectionNode->GetFinalValueInt(_T("height")) + STXBOOLINGLIST_SECTION_SPACING));
	__super::RemoveChildNode(nSectionIndex);

}

void CSTXSectionList::RemoveItemFromSection(int nSectionIndex, int nItemIndex)
{
	if (nSectionIndex < 0 || nSectionIndex >= (int)GetChildrenCount())
		return;

	CSTXAnimationControlChildNode *pSectionNode = _childNodes[nSectionIndex];
	int nChildrenCount = pSectionNode->GetChildrenCount();

	if (nItemIndex < 0 || nItemIndex >= nChildrenCount)
		return;

	CSTXAnimationControlChildNode *pItemNode = pSectionNode->GetChildNodeAtIndex(nItemIndex);
	pItemNode->SetValue(_T("opacity"), 0);
	pItemNode->SetValue(_T("scale"), 0.01);
	pItemNode->SetValue(_T("rotate"), -360);

	pSectionNode->RemoveChildNode(nItemIndex);

	int nChildrenCountNew = pSectionNode->GetChildrenCount();

	ResetItemLocationsInSectionFromIndex(nSectionIndex, nItemIndex);

	int nLine = CalculateLineCount(nChildrenCount);
	int nLineNew = CalculateLineCount(nChildrenCountNew);

	if (nLineNew != nLine)
	{
		//if (nLineNew != 0)
		{
			AdjustSectionLocationsFromIndex(nSectionIndex, -(STXBOOLINGLIST_CONTENT_SIZE + STXBOOLINGLIST_CONTENT_SPACING));
		}
		DOUBLE fSectionHeight = pSectionNode->GetFinalValue(_T("height"));
		DOUBLE fSectionDstHeight = STXBOOLINGLIST_SECTION_CAPTION_HEIGHT + STXBOOLINGLIST_CONTENT_MARGIN + nLineNew * (STXBOOLINGLIST_CONTENT_SIZE + STXBOOLINGLIST_CONTENT_SPACING) - STXBOOLINGLIST_CONTENT_SPACING;
		if (nLineNew == 0)
		{
			fSectionDstHeight += STXBOOLINGLIST_CONTENT_SPACING;
		}
		pSectionNode->SetValue(_T("height"), fSectionDstHeight);

		ResetScrollBars();
	}
}

POINT CSTXSectionList::CalculateItemLocation(int nSectionIndex, int nItemIndex)
{
	CSTXAnimationControlChildNode *pSectionNode = _childNodes[nSectionIndex];

	DOUBLE fCaptionHeight = pSectionNode->GetFinalValue(_T("captionHeight"));

	POINT pt;
	pt.x = STXBOOLINGLIST_CONTENT_MARGIN + (nItemIndex % _itemsPerLine) * (STXBOOLINGLIST_CONTENT_SIZE + STXBOOLINGLIST_CONTENT_SPACING);
	pt.y = static_cast<int>(fCaptionHeight + STXBOOLINGLIST_CONTENT_MARGIN + (nItemIndex / _itemsPerLine) * (STXBOOLINGLIST_CONTENT_SIZE + STXBOOLINGLIST_CONTENT_SPACING));

	return pt;
}

void CSTXSectionList::ResetItemLocationsInSectionFromIndex(int nSectionIndex, int nItemIndexStart)
{
	if (nSectionIndex < 0 || nSectionIndex >= (int)_childNodes.size())
		return;

	CSTXAnimationControlChildNode *pSectionNode = _childNodes[nSectionIndex];
	int nChildrenCount = pSectionNode->GetChildrenCount();

	for (int i = nItemIndexStart; i < nChildrenCount; i++)
	{
		CSTXAnimationControlChildNode *pItemNode = pSectionNode->GetChildNodeAtIndex(i);
		POINT pt;
		pt = CalculateItemLocation(nSectionIndex, i);
		pItemNode->SetValue(_T("x"), pt.x);
		pItemNode->SetValue(_T("y"), pt.y);
	}
}

int CSTXSectionList::CalculateLineCount(int nSectionItemCount)
{
	int nLineCount = nSectionItemCount / _itemsPerLine;
	if (nSectionItemCount % _itemsPerLine)
		nLineCount++;

	return nLineCount;
}

int CSTXSectionList::CalculateLineCount(int nSectionItemCount, int nItemsPerLine)
{
	int nLineCount = nSectionItemCount / nItemsPerLine;
	if (nSectionItemCount % nItemsPerLine)
		nLineCount++;

	return nLineCount;
}

void CSTXSectionList::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);

	if (cx <= STXBOOLINGLIST_MARGIN * 2)
		return;

	int nOldItemsPerLine = _itemsPerLine;
	CalculateItemsPerLine();
	int nNewItemsPerLine = _itemsPerLine;

	int i = 0;
	std::vector<CSTXAnimationControlChildNode*>::iterator it = _childNodes.begin();
	for (; it != _childNodes.end(); it++)
	{
		CSTXAnimationControlChildNode *pSectionNode = *it;

		if (dynamic_cast<CSTXSectionListTitleNode*>(pSectionNode) == NULL)
			continue;

		pSectionNode->SetValueInstantly(_T("width"), cx - STXBOOLINGLIST_MARGIN);
		int nOldLineCount = CalculateLineCount(pSectionNode->GetChildrenCount(), nOldItemsPerLine);
		int nNewLineCount = CalculateLineCount(pSectionNode->GetChildrenCount());

		if (nOldItemsPerLine != nNewItemsPerLine)
		{
			ResetItemLocationsInSectionFromIndex(i, min(nOldItemsPerLine, nNewItemsPerLine) - 1);

			if (nOldLineCount != nNewLineCount)
			{
				//if (nLineNew != 0)
				{
					AdjustSectionLocationsFromIndex(i, (STXBOOLINGLIST_CONTENT_SIZE + STXBOOLINGLIST_CONTENT_SPACING) * (nNewLineCount - nOldLineCount));
				}
				DOUBLE fSectionHeight = pSectionNode->GetFinalValue(_T("height"));
				DOUBLE fSectionDstHeight = fSectionHeight + (STXBOOLINGLIST_CONTENT_SIZE + STXBOOLINGLIST_CONTENT_SPACING) * (nNewLineCount - nOldLineCount);
				pSectionNode->SetValue(_T("height"), fSectionDstHeight);
			}
		}
		i++;
	}

	if (nOldItemsPerLine != nNewItemsPerLine)
		ResetScrollBars();
}

void CSTXSectionList::CalculateItemsPerLine()
{
	if (_hwndControl == NULL)
		return;

	RECT rcControl;
	::GetClientRect(_hwndControl, &rcControl);

	int nCount = (rcControl.right - STXBOOLINGLIST_MARGIN * 2 - STXBOOLINGLIST_CONTENT_MARGIN * 2) / (STXBOOLINGLIST_CONTENT_SIZE + STXBOOLINGLIST_CONTENT_SPACING);
	if (nCount < 1)
		nCount = 1;

	_itemsPerLine = nCount;
}

//////////////////////////////////////////////////////////////////////////

CSTXGraphicsBrush* CSTXSectionListTitleNode::_sbkBrush[1] = {0};

void CSTXSectionListTitleNode::DrawNode(CSTXGraphics *pGraphics)
{
	ANI_VAR_VALUE_INT(x, x);
	ANI_VAR_VALUE_INT(y, y);
	ANI_VAR_VALUE_INT(w, width);
	ANI_VAR_VALUE_INT(h, height);

	ANI_VAR_FINAL_VALUE_INT(fw, width);
	ANI_VAR_FINAL_VALUE_INT(fh, height);

	ANI_VAR_VALUE_INT(ch, captionHeight);
	ANI_VAR_VALUE_INT_MUL(opacity, opacity, 255);
	ANI_VAR_VALUE(scale, scale);

	CSTXGraphicsBrush *pBrush = pGraphics->CreateSimpleLinearGradientBrush(0, 0, 0, 114, 198, 255, fw, ch, 255, 255, 255, 255, opacity);

	CSTXGraphicsBrush *pBrushFont = pGraphics->CreateSolidBrush(255, 255, 255, static_cast<byte>(255 * GetAncestorOpacity()));
	CSTXGraphicsFont *pFont = pGraphics->CreateDrawingFont(_T("Arial"), 12, TRUE, FALSE);

	CSTXGraphicsTextFormat tf;

	CSTXGraphicsMatrix *pMatrixOld = pGraphics->GetTransform();

	CSTXGraphicsMatrix *pMatrixNew = pMatrixOld->Clone();

	pMatrixNew->Reset();
	pMatrixNew->Scale(scale, scale, Gdiplus::MatrixOrderAppend);
	pMatrixNew->Translate(x, y, Gdiplus::MatrixOrderAppend);
	pMatrixNew->Multiply(pMatrixOld, Gdiplus::MatrixOrderAppend);

	pGraphics->SetTransform(pMatrixNew);

	pGraphics->FillRectangle(0, 0, static_cast<int>(w), ch, pBrush);

	pGraphics->DrawString(_caption.c_str(), 0, 0, w, ch, pFont, pBrushFont, &tf);

	pGraphics->SetTransform(pMatrixOld);

	pBrush->Release();
	pBrushFont->Release();
	pFont->Release();

	delete pMatrixOld;
	delete pMatrixNew;

	__super::DrawNode(pGraphics);
}

int CSTXSectionListTitleNode::GetTotalHeight()
{
	ANI_VAR_FINAL_VALUE(h, height);
	return static_cast<int>(h);
}

BOOL CSTXSectionListTitleNode::IsDelayRemove()
{
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////

CSTXSectionListContentNode::CSTXSectionListContentNode()
{
	_bkBrush = NULL;
}

CSTXSectionListContentNode::~CSTXSectionListContentNode()
{
}

void CSTXSectionListContentNode::DrawNode(CSTXGraphics *pGraphics)
{
	ANI_VAR_VALUE_INT(x, x);
	ANI_VAR_VALUE_INT(y, y);
	ANI_VAR_VALUE_INT(w, width);
	ANI_VAR_VALUE_INT(h, height);
	ANI_VAR_VALUE(scale, scale);
	ANI_VAR_VALUE(rotate, rotate);
	ANI_VAR_VALUE(opacity, opacity);

	ANI_VAR_FINAL_VALUE_INT(fw, width);
	ANI_VAR_FINAL_VALUE_INT(fh, height);
	CSTXGraphicsMatrix *pMatrixOld = pGraphics->GetTransform();

	CSTXGraphicsMatrix *pMatrixNew = pMatrixOld->Clone();

	int offsetX = static_cast<int>((1 - scale) * w / 2);
	int offsetY = static_cast<int>((1 - scale) * h / 2);

	pMatrixNew->Reset();
	pMatrixNew->Translate(-w/2, -h/2, Gdiplus::MatrixOrderAppend);
	pMatrixNew->Rotate(rotate, Gdiplus::MatrixOrderAppend);
	pMatrixNew->Translate(w / 2, h / 2, Gdiplus::MatrixOrderAppend);
	pMatrixNew->Scale(scale * GetAncestorScale(), scale * GetAncestorScale(), Gdiplus::MatrixOrderAppend);
	pMatrixNew->Translate(x + offsetX, y + offsetY, Gdiplus::MatrixOrderAppend);
	pMatrixNew->Multiply(pMatrixOld, Gdiplus::MatrixOrderAppend);

	CSTXGraphicsBrush *pBrush1 = pGraphics->CreateButtonGradientBrush(0, 0, fw, fh, 102, 168, 168, static_cast<byte>(255 * GetAncestorOpacity() * opacity), 90);
	CSTXGraphicsBrush *pBrush2 = pGraphics->CreateButtonGradientBrush(0, 0, fw, fh, 86, 146, 146, static_cast<byte>(255 * GetAncestorOpacity() * opacity), 90);
	CSTXGraphicsBrush *pBrush3 = pGraphics->CreateButtonGradientBrush(0, 0, fw, fh, 168, 146, 146, static_cast<byte>(255 * GetAncestorOpacity() * opacity), 90);

	CSTXGraphicsBrush *pBrush[3] = { pBrush1, pBrush2, pBrush3 };
	_bkBrush = pBrush[_type];
	_bkBrush->SetOpacity(static_cast<byte>(255 * GetAncestorOpacity() * opacity));

	CSTXGraphicsBrush *pBrushFont = pGraphics->CreateSolidBrush(255, 255, 255, static_cast<byte>(255 * GetAncestorOpacity() * opacity));
	CSTXGraphicsFont *pFont = pGraphics->CreateDrawingFont(_T("Arial"), 12, TRUE, FALSE);

	CSTXGraphicsTextFormat tf;
	tf._alignment = 1;
	tf._valignment = 1;

	pGraphics->SetTransform(pMatrixNew);
	pGraphics->FillRectangle(0, 0, w, h, _bkBrush);
	pGraphics->DrawString(_caption.c_str(), 0, 0, w, h, pFont, pBrushFont, &tf);
	pGraphics->SetTransform(pMatrixOld);

	pBrushFont->Release();
	pFont->Release();
	pBrush1->Release();
	pBrush2->Release();
	pBrush3->Release();

	delete pMatrixOld;
	delete pMatrixNew;

	__super::DrawNode(pGraphics);
}

void CSTXSectionListContentNode::OnLButtonDown(int x, int y, UINT nFlags, BOOL bForRButton /*= FALSE*/)
{
	SetValue(_T("scale"), 0.9, 0.1);
	__super::OnLButtonDown(x, y, nFlags, bForRButton);
}

void CSTXSectionListContentNode::OnMouseMove(int x, int y, UINT nFlags)
{
	__super::OnMouseMove(x, y, nFlags);
}

void CSTXSectionListContentNode::OnLButtonUp(int x, int y, UINT nFlags, BOOL bForRButton /*= FALSE*/)
{
	SetValue(_T("scale"), 1.0, 0.1);
	__super::OnLButtonUp(x, y, nFlags, bForRButton);
}

void CSTXSectionListContentNode::OnMouseLeave()
{
	__super::OnMouseLeave();

	std::wstring s = _T("CSTXSectionListContentNode::OnMouseLeave  ");
	s += _caption;
	s += _T("\n");
	OutputDebugString(s.c_str());
}

void CSTXSectionListContentNode::OnLButtonClick()
{
	std::wstring s = _T("CSTXSectionListContentNode::OnLButtonClick  ");
	s += _caption;
	s += _T("\n");
	OutputDebugString(s.c_str());

	int nIndex = GetIndexInParentNode();
	int nSectionIndex = GetParentNode()->GetIndexInParentNode();
	((CSTXSectionList*)GetParentControl())->RemoveItemFromSection(nSectionIndex, nIndex);
}

void CSTXSectionListContentNode::OnMouseEnter()
{
	__super::OnMouseEnter();

	std::wstring s = _T("CSTXSectionListContentNode::OnMouseEnter  ");
	s += _caption;
	s += _T("\n");
	OutputDebugString(s.c_str());
}

void CSTXSectionListContentNode::OnRButtonClick()
{
	std::wstring s = _T("CSTXSectionListContentNode::OnRButtonClick  ");
	s += _caption;
	s += _T("\n");
	OutputDebugString(s.c_str());

}

void CSTXSectionListContentNode::OnLButtonDblClk(int x, int y, UINT nFlags)
{
	std::wstring s = _T("CSTXSectionListContentNode::OnLButtonDblClk  ");
	s += _caption;
	s += _T("\n");
	OutputDebugString(s.c_str());
}

void CSTXSectionListContentNode::OnQueryToolTipsText(LPTSTR pszBuffer, int cchBufferSize, POINT ptLocation, LPCTSTR *ppszToolTips)
{
	*ppszToolTips = _caption.c_str();
	__super::OnQueryToolTipsText(pszBuffer, cchBufferSize, ptLocation, ppszToolTips);
}

BOOL CSTXSectionListContentNode::IsDelayRemove()
{
	return TRUE;
}

void CSTXSectionListContentNode::OnBeginMouseDragging(POINT ptGlobal)
{
	__super::OnBeginMouseDragging(ptGlobal);
	std::wstring s = _T("CSTXSectionListContentNode::OnBeginMouseDragging  ");
	s += _caption;
	s += _T("\n");
	OutputDebugString(s.c_str());
}

void CSTXSectionListContentNode::OnEndMouseDragging(POINT ptGlobal, POINT ptOffset)
{
	__super::OnEndMouseDragging(ptGlobal, ptOffset);
	std::wstring s = _T("CSTXSectionListContentNode::OnEndMouseDragging  ");
	s += _caption;
	s += _T("\n");
	OutputDebugString(s.c_str());
}


//////////////////////////////////////////////////////////////////////////

void CSTXSectionListTipNode::DrawNode(CSTXGraphics *pGraphics)
{
	ANI_VAR_VALUE_INT(x, x);
	ANI_VAR_VALUE_INT(y, y);
	ANI_VAR_VALUE_INT(w, width);
	ANI_VAR_VALUE_INT(h, height);

	ANI_VAR_FINAL_VALUE_INT(fw, width);
	ANI_VAR_FINAL_VALUE_INT(fh, height);


	CSTXGraphicsMatrix *pMatrixOld = pGraphics->GetTransform();
	CSTXGraphicsMatrix *pMatrixNew = pMatrixOld->Clone();

	pMatrixNew->Reset();
	pMatrixNew->Scale(GetAncestorScale(), GetAncestorScale(), Gdiplus::MatrixOrderAppend);
	pMatrixNew->Translate(x, y, Gdiplus::MatrixOrderAppend);
	pMatrixNew->Multiply(pMatrixOld, Gdiplus::MatrixOrderAppend);

	byte ancestorOpacity = GetAncestorOpacityByte();

	CSTXGraphicsBrush *pBrush1 = pGraphics->CreateButtonGradientBrush(0, 0, fw, fh, 192, 192, 192, ancestorOpacity, 90);
	CSTXGraphicsBrush *pBrush2 = pGraphics->CreateButtonGradientBrush(0, 0, fw, fh, 255, 37, 37, ancestorOpacity, 90);
	CSTXGraphicsBrush *pBrush3 = pGraphics->CreateSolidBrush(255, 255, 255, ancestorOpacity / 4);
	CSTXGraphicsBrush *pBrush4 = pGraphics->CreateButtonGradientBrush(0, 0, fw, fh, 63, 169, 63, ancestorOpacity, 90);

	CSTXGraphicsBrush *pBrushs[4] = { pBrush1, pBrush2, pBrush3, pBrush4 };

	CSTXGraphicsFont *pFont1 = pGraphics->CreateDrawingFont(_T("Arial"), 12, FALSE, FALSE);
	CSTXGraphicsFont *pFont2 = pGraphics->CreateDrawingFont(_T("Arial"), 12, TRUE, FALSE);
	CSTXGraphicsFont *pFont3 = pGraphics->CreateDrawingFont(_T("Arial"), 24, TRUE, FALSE);

	CSTXGraphicsFont *pFonts[3] = { pFont1, pFont2, pFont3 };

	CSTXGraphicsBrush *pBrush = NULL;
	CSTXGraphicsFont *pFont = NULL;

	if (_location == 1)		//Top Right
	{
		if (_caption == _T("0"))
			pBrush = pBrushs[0];
		else
			pBrush = pBrushs[1];

		pFont = pFonts[0];
	}
	else if (_location == 0)	//Top Left
	{
		pBrush = pBrushs[2];
		pFont = pFonts[2];
	}
	else
	{
		pBrush = pBrushs[3];
		pFont = pFonts[0];
	}

	CSTXGraphicsTextFormat tf;
	tf._alignment = 1;			//Center
	tf._valignment = 1;			//V-Center
	tf._endEllipsis = TRUE;
	tf._wordWrap = FALSE;

	pGraphics->SetTransform(pMatrixNew);

	if (_location == 0)
	{
		pGraphics->FillRectangle(0, 0, w, h, pBrush);
	}
	else
		pGraphics->FillRectangle(0, 0, w, h, pBrush);


	CSTXGraphicsBrush *pBrushFont = pGraphics->CreateSolidBrush(255, 255, 255, ancestorOpacity);
	if (_location == 1)
	{
		LANGID lang = GetUserDefaultUILanguage();
		if (lang != 0x804)		//not chinese
		{
			pGraphics->DrawString((_caption + _T(" left")).c_str(), 0, 0, w, h, pFont, pBrushFont, &tf);
		}
		else
		{
			pGraphics->DrawString((_T("สฃำเ: ") + _caption).c_str(), 0, 0, w, h, pFont, pBrushFont, &tf);
		}
	}
	else
	{
		pGraphics->DrawString(_caption.c_str(), 0, 0, w, h, pFont, pBrushFont, &tf);
	}

	pGraphics->SetTransform(pMatrixOld);

	pBrushFont->Release();
	pFont1->Release();
	pFont2->Release();
	pFont3->Release();
	pBrush1->Release();
	pBrush2->Release();
	pBrush3->Release();
	pBrush4->Release();

	delete pMatrixOld;
	delete pMatrixNew;

	__super::DrawNode(pGraphics);
}

void CSTXSectionListTipNode::OnQueryToolTipsText(LPTSTR pszBuffer, int cchBufferSize, POINT ptLocation, LPCTSTR *ppszToolTips)
{
	*ppszToolTips = _caption.c_str();
	__super::OnQueryToolTipsText(pszBuffer, cchBufferSize, ptLocation, ppszToolTips);

}

