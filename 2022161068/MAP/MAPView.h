
// MAPView.h: CMAPView 클래스의 인터페이스
//

#pragma once
#include <atlimage.h> 
#include <vector> // <--- [추가] 벡터(동적 배열) 사용을 위해 필요

#define MAX_NODES 100

struct Node {
	CPoint position; // 화면상 좌표 (x, y)
	int id;          // 노드 번호 (0부터 시작 또는 1부터 시작)
};

// [추가] 엣지(선) 구조체 정의
struct Edge {
	int nodeId1;   // 시작점 노드 번호
	int nodeId2;   // 도착점 노드 번호
	double weight; // 두 점 사이의 거리
};

class CMAPView : public CView
{
protected: // serialization에서만 만들어집니다.
	CMAPView() noexcept;
	DECLARE_DYNCREATE(CMAPView)

// 특성입니다.
public:
	CMAPDoc* GetDocument() const;
	// [추가 2] 멤버 변수 선언
	CImage m_bgImage;

	std::vector<Node> m_nodes;      // 찍힌 점들을 모아두는 리스트
	std::vector<Edge> m_edges;

	double m_adjMatrix[MAX_NODES][MAX_NODES];

	int m_iSelectedNode1;      // 엣지 연결할 때 첫 번째 선택한 노드 인덱스 (-1이면 선택 안 함)
	int m_iPathStartNode;      // 최단 경로 출발 노드 인덱스
	int m_iPathEndNode;        // 최단 경로 도착 노드 인덱스

	// [추가] 최단 경로 결과 저장용 (지나가는 노드들의 번호를 순서대로 저장)
	std::vector<int> m_shortestPath;

	// [추가] 헬퍼 함수: 클릭한 위치에 노드가 있는지 확인하는 함수
	int GetNodeIndexAt(CPoint point);

	void RunDijkstra();

// 작업입니다.
public:


// 재정의입니다.
public:
	virtual void OnDraw(CDC* pDC);  // 이 뷰를 그리기 위해 재정의되었습니다.
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

