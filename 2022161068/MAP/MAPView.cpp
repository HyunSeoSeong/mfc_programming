
// MAPView.cpp: CMAPView 클래스의 구현
//

#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS는 미리 보기, 축소판 그림 및 검색 필터 처리기를 구현하는 ATL 프로젝트에서 정의할 수 있으며
// 해당 프로젝트와 문서 코드를 공유하도록 해 줍니다.
#ifndef SHARED_HANDLERS
#include "MAP.h"
#endif

#include "MAPDoc.h"
#include "MAPView.h"

#include <float.h> // DBL_MAX (무한대 값) 사용을 위해 필요

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
				m_adjMatrix[i][j] = 0;  // 자기 자신까지 거리는 0
			else
				m_adjMatrix[i][j] = -1; // 나머지는 연결 없음(-1)
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

	// 1. 배경 지도 그리기
	if (!m_bgImage.IsNull()) {
		m_bgImage.Draw(pDC->m_hDC, 0, 0);
	}

	// 펜 설정 (파란선, 빨간선)
	CPen bluePen(PS_SOLID, 2, RGB(0, 0, 255));
	CPen redPen(PS_SOLID, 4, RGB(255, 0, 0)); // 경로는 더 굵게
	CPen* pOldPen;

	// 2. 모든 엣지(선) 그리기 (파란색)
	pOldPen = pDC->SelectObject(&bluePen);
	for (int i = 0; i < m_edges.size(); i++) {
		CPoint p1 = m_nodes[m_edges[i].nodeId1].position;
		CPoint p2 = m_nodes[m_edges[i].nodeId2].position;

		pDC->MoveTo(p1);
		pDC->LineTo(p2);

		// 거리 숫자 표시
		CString strDist;
		strDist.Format(_T("%.1f"), m_edges[i].weight);
		pDC->TextOutW((p1.x + p2.x) / 2, (p1.y + p2.y) / 2, strDist);
	}
	pDC->SelectObject(pOldPen);

	// 3. 최단 경로 엣지 덮어 그리기 (빨간색) - 추후 구현될 m_shortestPath 활용
	if (m_shortestPath.size() > 1) {
		CPen redPathPen(PS_SOLID, 5, RGB(255, 0, 0));
		pOldPen = pDC->SelectObject(&redPathPen);

		// 반복문 시작
		for (int i = 0; i < (int)m_shortestPath.size() - 1; i++) {
			int node1 = m_shortestPath[i];
			int node2 = m_shortestPath[i + 1];

			// 좌표 가져오기
			CPoint p1 = m_nodes[node1].position;
			CPoint p2 = m_nodes[node2].position;

			pDC->MoveTo(p1);
			pDC->LineTo(p2);
		}
		pDC->SelectObject(pOldPen);
	}
	// 4. 노드(점) 그리기
	CBrush blueBrush(RGB(0, 0, 255));
	CBrush redBrush(RGB(255, 0, 0));
	CBrush* pOldBrush;

	for (int i = 0; i < m_nodes.size(); i++) {
		CPoint pt = m_nodes[i].position;
		CRect rect(pt.x - 10, pt.y - 10, pt.x + 10, pt.y + 10); // 점 크기

		// 색상 결정: 출발점/도착점이거나 경로에 포함되면 빨간색
		if (i == m_iPathStartNode || i == m_iPathEndNode) {
			pOldBrush = pDC->SelectObject(&redBrush);
		}
		else {
			pOldBrush = pDC->SelectObject(&blueBrush);
		}

		pDC->Ellipse(rect); // 원 그리기
		pDC->SelectObject(pOldBrush);

		// 노드 번호 표시 (선택사항)
		CString strNum;
		strNum.Format(_T("%d"), i + 1);
		pDC->SetBkMode(TRANSPARENT); // 글자 배경 투명하게
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

CMAPDoc* CMAPView::GetDocument() const // 디버그되지 않은 버전은 인라인으로 지정됩니다.
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMAPDoc)));
	return (CMAPDoc*)m_pDocument;
}
#endif //_DEBUG


// CMAPView 메시지 처리기
void CMAPView::OnInitialUpdate()
{
	CView::OnInitialUpdate();

	// 1. 이미지 로드 (프로젝트 폴더에 map.jpg가 있어야 함)
	// 경로에 한글이 포함되면 에러가 날 수 있으니 주의하세요.
	HRESULT hResult = m_bgImage.Load(_T("MAP.png"));

	// 2. 실패 시 메시지 박스 출력
	if (FAILED(hResult)) {
		AfxMessageBox(_T("이미지 로드 실패! 프로젝트 폴더에 map.png가 있는지 확인하세요."));
	}

}

int CMAPView::GetNodeIndexAt(CPoint point)
{
	// 저장된 모든 노드를 검사
	for (int i = 0; i < m_nodes.size(); i++) {
		// 두 점 사이의 거리 계산 (피타고라스 정리)
		double dx = m_nodes[i].position.x - point.x;
		double dy = m_nodes[i].position.y - point.y;
		double dist = sqrt(dx * dx + dy * dy);

		// 반지름 15 픽셀 이내에 클릭했으면 해당 노드로 인정
		if (dist < 15) {
			return i; // 찾은 노드의 인덱스 반환
		}
	}
	return -1; // 못 찾음
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

	// ---------------------------------------------------------
	// 상황 B: Ctrl 키 누른 상태 -> 엣지(선) 연결
	// ---------------------------------------------------------
	if (nFlags & MK_CONTROL) {
		if (clickedNodeIndex != -1) { // 노드를 정확히 클릭했다면

			if (m_iSelectedNode1 == -1) {
				// 첫 번째 점 선택
				m_iSelectedNode1 = clickedNodeIndex;
			}
			else {
				// 두 번째 점 선택 -> 선 연결!
				int node1 = m_iSelectedNode1;
				int node2 = clickedNodeIndex;

				// 같은 점을 두 번 클릭한 게 아니라면
				if (node1 != node2) {
					// 1. 거리 계산 (유클리드 거리)
					CPoint p1 = m_nodes[node1].position;
					CPoint p2 = m_nodes[node2].position;
					double dist = sqrt(pow(p1.x - p2.x, 2) + pow(p1.y - p2.y, 2));

					// 2. 엣지 정보 저장
					Edge newEdge;
					newEdge.nodeId1 = node1;
					newEdge.nodeId2 = node2;
					newEdge.weight = dist;
					m_edges.push_back(newEdge);

					// 3. 인접 행렬 업데이트 (양방향)
					m_adjMatrix[node1][node2] = dist;
					m_adjMatrix[node2][node1] = dist;

					// 선택 초기화
					m_iSelectedNode1 = -1;
					Invalidate(); // 화면 갱신
				}
			}
		}
	}
	// ---------------------------------------------------------
	// 상황 C: Alt 키 누른 상태 -> 최단 경로 검색
	// (Alt키는 nFlags로 확인이 잘 안 될 때가 있어 GetKeyState 사용)
	// ---------------------------------------------------------
	else if (GetKeyState(VK_MENU) < 0) {
		if (clickedNodeIndex != -1) {

			// 만약 이미 경로가 그려져 있다면 초기화 (새로운 찾기 시작)
			if (m_iPathStartNode != -1 && m_iPathEndNode != -1) {
				m_iPathStartNode = -1;
				m_iPathEndNode = -1;
				m_shortestPath.clear(); // 저장된 경로 삭제
				Invalidate();
			}

			if (m_iPathStartNode == -1) {
				// 출발점 선택
				m_iPathStartNode = clickedNodeIndex;
				Invalidate(); // 빨간색으로 칠하기 위해 갱신
			}
			else {
				// 도착점 선택
				m_iPathEndNode = clickedNodeIndex;

				// ★ 여기서 다익스트라 알고리즘 실행! ★
				RunDijkstra(); // (이 함수는 다음 단계에서 구현할 예정)

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

	// 1. 필요한 배열 선언 및 초기화
	double* dist = new double[nNodeCount];  // 거리 저장 (dist[])
	int* parent = new int[nNodeCount];      // 경로 복원용 (predecessor[])
	bool* visited = new bool[nNodeCount];   // 방문 여부 확인

	for (int i = 0; i < nNodeCount; i++) {
		dist[i] = DBL_MAX; // 처음에 모든 거리는 무한대로 설정
		parent[i] = -1;    // 연결된 부모 노드 없음
		visited[i] = false;
	}

	// 2. 출발 노드 설정
	dist[m_iPathStartNode] = 0; // 시작점의 거리는 0

	// 3. 루프 시작 (모든 노드를 방문할 때까지)
	for (int i = 0; i < nNodeCount; i++) {

		// 3-1. 방문하지 않은 노드 중 거리가 가장 짧은 노드(u) 찾기
		int u = -1;
		double minDesc = DBL_MAX;

		for (int j = 0; j < nNodeCount; j++) {
			if (!visited[j] && dist[j] < minDesc) {
				minDesc = dist[j];
				u = j;
			}
		}

		// 더 이상 갈 곳이 없거나, 도착점에 도달했으면 중단 (최적화)
		if (u == -1 || dist[u] == DBL_MAX) break;

		// 3-2. 노드 방문 처리
		visited[u] = true;

		// 3-3. 인접한 노드들의 거리 갱신 (Relaxation)
		for (int v = 0; v < nNodeCount; v++) {
			// 연결되어 있고(거리 > 0), 아직 방문 안 했으면
			if (m_adjMatrix[u][v] != -1 && !visited[v]) {

				// (시작->u 거리) + (u->v 거리) 가 (기존 시작->v 거리)보다 짧으면 갱신
				if (dist[u] + m_adjMatrix[u][v] < dist[v]) {
					dist[v] = dist[u] + m_adjMatrix[u][v];
					parent[v] = u; // v에 오기 직전 노드는 u라고 기록 (족보 작성)
				}
			}
		}
	}

	// 4. 경로 역추적 (Backtracking) 및 결과 저장
	// 도착점(End)부터 시작해서 parent를 타고 출발점(Start)까지 거슬러 올라갑니다.
	m_shortestPath.clear();

	// 도착점에 도달할 수 있는 경우에만 경로 복원
	if (dist[m_iPathEndNode] != DBL_MAX) {
		int curr = m_iPathEndNode;
		while (curr != -1) {
			m_shortestPath.push_back(curr); // 경로에 추가
			curr = parent[curr]; // 부모 노드로 이동
		}
		// 현재 m_shortestPath에는 [도착점, ..., 출발점] 순서로 들어있음
	}
	CString debugMsg;
	debugMsg.Format(_T("경로 개수: %d"), m_shortestPath.size());
	AfxMessageBox(debugMsg);
	// 5. 총 거리 출력 (메시지 박스)
	if (dist[m_iPathEndNode] == DBL_MAX) {
		AfxMessageBox(_T("길이 연결되어 있지 않습니다!"));
	}
	else {
		CString strMsg;
		strMsg.Format(_T("최단 거리: %.1f"), dist[m_iPathEndNode]);
		// 화면 좌상단에 표시하거나 메시지 박스로 띄움 (여기선 간단히 메시지박스)
		AfxMessageBox(strMsg);
	}

	// 메모리 해제 (중요!)
	delete[] dist;
	delete[] parent;
	delete[] visited;
}