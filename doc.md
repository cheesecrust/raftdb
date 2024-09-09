node->node_id = id;
node->current_term = 0;
node->voted_for = -1;
node->state = FOLLOWER;
node->leader_id = -1;
node->last_heartbeat = time(NULL);
node->election_timeout = ((double)rand() / RAND_MAX) * 2.0 + 1.0;

voted_for == -1 일 경우 투표 가능
leader_id == -1 아직 리더 없음

nodes 에는 명령줄에 입력된 순서대로 노드에 들어가 있습니다.
따라서 맨 앞이 자기 자신이고 뒤에 차례로 다른 노드들이 주어집니다.

이떄 노드들의 순서는 노드 num 과는 관련이 없습니다.
따라서 리더 노드를 저장할 때에는 해당 ip와 port로 저장합니다.

TODO

만약 리더가 동시 발생

현재 시간 설정

현재 선거 타임아웃 : 2 - 4
하트 비트 전송 간격 : 1초
후보자의 투표 요청은 2초 간격 실행

## issue

리더 사망시 재 선출 로직 수행불가

모두가 투표를 하기전에 자기 자신이 후보자가 되어 투표 불가
-> 각각의 프로세스에서 하는데 모두의 선거 타임아웃이 같음
-> seed 초기화


- 하지만 rand 이어도 동점자 처리 로직이 들어가야 함

- 추가하지도 않은 노드, 자신 보다 임기가 낮은 노드들의 투표 요청 자르기