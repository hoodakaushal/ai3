int inf, neginf, d;
// int player,opp;
bool finish(BGraph board) {
	return ((board.row1 == board.rows) && (board.row2 == 1));
}
struct pv {
	BGraph state;
	Move mov;
	int value;
};

void reorder(BGraph state, vector<Move> moves, vector<pv> path, int a)        // simple reordering
		{
	if (a == 0) {
		moves.insert(moves.begin(), path[0].mov);
	} else if (state == path[a - 1].state) {
		moves.insert(moves.begin(), path[a].mov);
	}

}

/*void reorder(bGraph state,vector<Move> moves,vector<pv> path,int d)
 {
 for(int k=0;k++;k<moves.size())
 {bGraph temp=applyMove(state,moves[k]);
 if (temp==(path[d].state))
 {Move t=moves[k];
 moves.erase(moves.begin()+k);
 moves.insert(moves.begin(),t);}


 }} */
vector<pv> NEGAMAX(BGraph node, int depth, int alpha, int beta, int color, vector<pv> path) {

	int bestValue = neginf;
	Move bestMove;
	BGraph bestState;
	vector<Move> movelist;
	if (color == 1)
		movelist = movegen(node, node.me);
	else
		movelist = movegen(node, 3 - (node.me));
	vector<pv> ans;
	if (path.size() > (d - depth))
		reorder(node, movelist, path, (d - depth));
	for (int i = 0; i++; i < movelist.size()) {
		BGraph child = applyMove(node, movelist[i]);
		if (finish(child) || depth == 1) {
			int val = color * utility(child);
			if (bestValue < val) {
				bestMove = movelist[i];
				bestState = child;
			}
			bestValue = max(bestValue, val);

			alpha = max(alpha, val);
			if (alpha >= beta)
				break;
		}

		else {
			vector<pv> p = NEGAMAX(child, depth - 1, -beta, -alpha, (-1) * color, path);
			p[0].value = (-1) * (p[0].value);

			if (bestValue < p[0].value) {
				bestMove = movelist[i];
				ans = p;
				bestState = child;
			}
			bestValue = max(bestValue, p[0].value);
			alpha = max(alpha, p[0].value);
			if (alpha >= beta)
				break;
		}

	}

	pv edge;
	edge.mov = bestMove;
	edge.value = bestValue;
	edge.state = bestState;
	ans.insert(ans.begin(), edge);
	return ans;
}

Move ITERATIVE_NEGAMAX(BGraph board) {
	d = 1;vector<pv> path;
	Move m;
	while(d<4)           //while time's left
	{
		path = NEGAMAX(board,d,neginf,inf,1,path);
		m = (path[0]).mov;
		d = d+1;
	}
	return m;
}
