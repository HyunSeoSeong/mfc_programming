
// MAPView.cpp: CMAPView 클래스의 구현
//

#include "pch.h"
#include "framework.h"
#ifndef SHARED_HANDLERS
#include "MAP.h"
#endif

#include "MAPDoc.h"
#include "MAPView.h"

#include <float.h> 

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMAPView

IMPLEMENT_DYNCREATE(CMAPView, CView)

BEGIN_MESSAGE_MAP(CMAPView, CView)
	// 표준 인쇄 명령입니다.
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CView::OnFilePrintPreview)
	ON_WM_RBUTTONDOWN()
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()

// CMAPView 생성/소멸

CMAPView::CMAPView() noexcept
{
	for (int i = 0; i < MAX_NODES; i++) {
		for (int j = 0; j < MAX_NODES; j++) {
			if (i == j)
				m_adjMatrix[i][j] = 0; 
			else
				m_adjMatrix[i][j] = -1;
		}
	}

	m_iSelectedNode1 = -1;
	m_iPathStartNode = -1;
	m_iPathEndNode = -1;
}

CMAPView::~CMAPView()
{
}

BOOL CMAPView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: CREATESTRUCT cs를 수정하여 여기에서
	//  Window 클래스 또는 스타일을 수정합니다.

	return CView::PreCreateWindow(cs);
}

// CMAPView 그리기

void CMAPView::OnDraw(CDC* pDC)
{
	CMAPDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc) return;

	if (!m_bgImage.IsNull()) {
		m_bgImage.Draw(pDC->m_hDC, 0, 0);
	}

	CPen bluePen(PS_SOLID, 2, RGB(0, 0, 255));
	CPen redPen(PS_SOLID, 4, RGB(255, 0, 0));
	CPen* pOldPen;

	pOldPen = pDC->SelectObject(&bluePen);
	for (int i = 0; i < m_edges.size(); i++) {
		CPoint p1 = m_nodes[m_edges[i].nodeId1].position;
		CPoint p2 = m_nodes[m_edges[i].nodeId2].position;

		pDC->MoveTo(p1);
		pDC->LineTo(p2);

		CString strDist;
		strDist.Format(_T("%.1f"), m_edges[i].weight);
		pDC->TextOutW((p1.x + p2.x) / 2, (p1.y + p2.y) / 2, strDist);
	}
	pDC->SelectObject(pOldPen);

	if (m_shortestPath.size() > 1) {
		CPen redPathPen(PS_SOLID, 5, RGB(255, 0, 0));
		pOldPen = pDC->SelectObject(&redPathPen);

		for (int i = 0; i < (int)m_shortestPath.size() - 1; i++) {
			int node1 = m_shortestPath[i];
			int node2 = m_shortestPath[i + 1];

			CPoint p1 = m_nodes[node1].position;
			CPoint p2 = m_nodes[node2].position;

			pDC->MoveTo(p1);
			pDC->LineTo(p2);
		}
		pDC->SelectObject(pOldPen);
	}
	CBrush blueBrush(RGB(0, 0, 255));
	CBrush redBrush(RGB(255, 0, 0));
	CBrush* pOldBrush;

	for (int i = 0; i < m_nodes.size(); i++) {
		CPoint pt = m_nodes[i].position;
		CRect rect(pt.x - 10, pt.y - 10, pt.x + 10, pt.y + 10);

		if (i == m_iPathStartNode || i == m_iPathEndNode) {
			pOldBrush = pDC->SelectObject(&redBrush);
		}
		else {
			pOldBrush = pDC->SelectObject(&blueBrush);
		}

		pDC->Ellipse(rect); 
		pDC->SelectObject(pOldBrush);

		CString strNum;
		strNum.Format(_T("%d"), i + 1);
		pDC->SetBkMode(TRANSPARENT);
		pDC->TextOutW(pt.x - 5, pt.y - 5, strNum);
	}
}


// CMAPView 인쇄

BOOL CMAPView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// 기본적인 준비
	return DoPreparePrinting(pInfo);
}

void CMAPView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 인쇄하기 전에 추가 초기화 작업을 추가합니다.
}

void CMAPView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 인쇄 후 정리 작업을 추가합니다.
}


// CMAPView 진단

#ifdef _DEBUG
void CMAPView::AssertValid() const
{
	CView::AssertValid();
}

void CMAPView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CMAPDoc* CMAPView::GetDocument() const /
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMAPDoc)));
	return (CMAPDoc*)m_pDocument;
}
#endif //_DEBUG


// CMAPView 메시지 처리기
void CMAPView::OnInitialUpdate()
{
	CView::OnInitialUpdate();

	HRESULT hResult = m_bgImage.Load(_T("MAP.png"));

	if (FAILED(hResult)) {
		AfxMessageBox(_T("이미지 로드 실패! 프로젝트 폴더에 MAP.png가 있는지 확인하세요."));
	}

	CString strHelpMsg =
		_T("=============== [ 프로그램 사용법 ] ===============\n\n")
		_T("1. 노드 생성\n")
		_T("   - 조작: 마우스 우 클릭\n")
		_T("   - 동작: 파란색 노드 생성\n\n")
		_T("2. 노드 연결(선)\n")
		_T("   - 조작: [Ctrl] + 점 2개 순서대로 좌 클릭\n")
		_T("   - 동작: 두 점 사이의 파란 선과 거리가 표시\n\n")
		_T("3. 최단 경로 탐색\n")
		_T("   - 조작: [Alt] + 출발점, 도착점 순서대로 좌 클릭\n")
		_T("   - 동작: 최단 경로가 빨간색 굵은 선으로 표시");

	AfxMessageBox(strHelpMsg, MB_ICONINFORMATION);

}

int CMAPView::GetNodeIndexAt(CPoint point)
{
	for (int i = 0; i < m_nodes.size(); i++) {
		double dx = m_nodes[i].position.x - point.x;
		double dy = m_nodes[i].position.y - point.y;
		double dist = sqrt(dx * dx + dy * dy);

		if (dist < 15) {
			return i;
		}
	}
	return -1;
}
void CMAPView::OnRButtonDown(UINT nFlags, CPoint point)
{
	if (m_nodes.size() >= MAX_NODES) {
		AfxMessageBox(_T("더 이상 노드를 추가할 수 없습니다."));
		return;
	}

	Node newNode;
	newNode.position = point;
	newNode.id = (int)m_nodes.size();

	m_nodes.push_back(newNode);

	Invalidate();
	CView::OnRButtonDown(nFlags, point);
}

void CMAPView::OnLButtonDown(UINT nFlags, CPoint point)
{
	int clickedNodeIndex = GetNodeIndexAt(point);


	if (nFlags & MK_CONTROL) {
		if (clickedNodeIndex != -1) { 

			if (m_iSelectedNode1 == -1) {
				m_iSelectedNode1 = clickedNodeIndex;
			}
			else {
				int node1 = m_iSelectedNode1;
				int node2 = clickedNodeIndex;

				if (node1 != node2) {
					CPoint p1 = m_nodes[node1].position;
					CPoint p2 = m_nodes[node2].position;
					double dist = sqrt(pow(p1.x - p2.x, 2) + pow(p1.y - p2.y, 2));

					Edge newEdge;
					newEdge.nodeId1 = node1;
					newEdge.nodeId2 = node2;
					newEdge.weight = dist;
					m_edges.push_back(newEdge);

					m_adjMatrix[node1][node2] = dist;
					m_adjMatrix[node2][node1] = dist;

					m_iSelectedNode1 = -1;
					Invalidate();
				}
			}
		}
	}
	
	else if (GetKeyState(VK_MENU) < 0) {
		if (clickedNodeIndex != -1) {

			if (m_iPathStartNode != -1 && m_iPathEndNode != -1) {
				m_iPathStartNode = -1;
				m_iPathEndNode = -1;
				m_shortestPath.clear(); 
				Invalidate();
			}

			if (m_iPathStartNode == -1) {
				m_iPathStartNode = clickedNodeIndex;
				Invalidate(); 
			}
			else {
				m_iPathEndNode = clickedNodeIndex;
				RunDijkstra();
				Invalidate();
			}
		}
	}
	CView::OnLButtonDown(nFlags, point);
}

void CMAPView::RunDijkstra()
{
	int nNodeCount = (int)m_nodes.size();
	if (nNodeCount == 0) return;

	double* dist = new double[nNodeCount];  
	int* parent = new int[nNodeCount];     
	bool* visited = new bool[nNodeCount];   

	for (int i = 0; i < nNodeCount; i++) {
		dist[i] = DBL_MAX; 
		parent[i] = -1;    
		visited[i] = false;
	}

	dist[m_iPathStartNode] = 0; 

	for (int i = 0; i < nNodeCount; i++) {
		int u = -1;
		double minDesc = DBL_MAX;

		for (int j = 0; j < nNodeCount; j++) {
			if (!visited[j] && dist[j] < minDesc) {
				minDesc = dist[j];
				u = j;
			}
		}

		if (u == -1 || dist[u] == DBL_MAX) break;

		visited[u] = true;

		for (int v = 0; v < nNodeCount; v++) {
			if (m_adjMatrix[u][v] != -1 && !visited[v]) {
				if (dist[u] + m_adjMatrix[u][v] < dist[v]) {
					dist[v] = dist[u] + m_adjMatrix[u][v];
					parent[v] = u; 
				}
			}
		}
	}


	m_shortestPath.clear();

	if (dist[m_iPathEndNode] != DBL_MAX) {
		int curr = m_iPathEndNode;
		while (curr != -1) {
			m_shortestPath.push_back(curr);
			curr = parent[curr];
		}
	}
	CString debugMsg;
	debugMsg.Format(_T("경로 개수: %d"), m_shortestPath.size());
	AfxMessageBox(debugMsg);
	if (dist[m_iPathEndNode] == DBL_MAX) {
		AfxMessageBox(_T("길이 연결되어 있지 않습니다!"));
	}
	else {
		CString strMsg;
		strMsg.Format(_T("최단 거리: %.1f"), dist[m_iPathEndNode]);
		AfxMessageBox(strMsg);
	}

	delete[] dist;
	delete[] parent;
	delete[] visited;
}