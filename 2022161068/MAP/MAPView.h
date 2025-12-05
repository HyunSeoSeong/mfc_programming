
// MAPView.h: CMAPView 클래스의 인터페이스
//

#pragma once
#include <atlimage.h> 
#include <vector> 

#define MAX_NODES 100

struct Node {
	CPoint position; 
	int id;          
};

struct Edge {
	int nodeId1; 
	int nodeId2;
	double weight; 
};

class CMAPView : public CView
{
protected:
	CMAPView() noexcept;
	DECLARE_DYNCREATE(CMAPView)

// 특성입니다.
public:
	CMAPDoc* GetDocument() const;
	CImage m_bgImage;

	std::vector<Node> m_nodes;
	std::vector<Edge> m_edges;

	double m_adjMatrix[MAX_NODES][MAX_NODES];

	int m_iSelectedNode1;
	int m_iPathStartNode; 
	int m_iPathEndNode;

	std::vector<int> m_shortestPath;
	int GetNodeIndexAt(CPoint point);
	void RunDijkstra();

// 작업입니다.
public:


// 재정의입니다.
public:
	virtual void OnDraw(CDC* pDC); 
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

	virtual void OnInitialUpdate();

protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// 구현입니다.
public:
	virtual ~CMAPView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 생성된 메시지 맵 함수
protected:
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
};

#ifndef _DEBUG  // MAPView.cpp의 디버그 버전
inline CMAPDoc* CMAPView::GetDocument() const
   { return reinterpret_cast<CMAPDoc*>(m_pDocument); }
#endif

