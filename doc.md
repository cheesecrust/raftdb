node->node_id = id;
node->current_term = 0;
node->voted_for = -1;
node->state = FOLLOWER;
node->leader_id = -1;
node->last_heartbeat = time(NULL);
node->election_timeout = ((double)rand() / RAND_MAX) * 2.0 + 1.0;

voted_for == -1 일 경우 투표 가능
leader_id == -1 아직 리더 없음
