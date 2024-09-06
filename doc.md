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

TODO

만약 리더가 동시 발생
