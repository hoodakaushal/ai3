#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h> 
#include <bits/stdc++.h>
#include <vector>

using namespace std;

int progress = 0;

//=======================================Graph stuff=========================================
struct Wall {
	bool horizontal;
	int row;
	int column;
	Wall(bool h, int r, int c) {
		horizontal = h;
		row = r;
		column = c;
	}
};

struct Move {
	// 0 - pawn, 1 - h. wall, 2 - v. wall
	int type;
	int row;
	int column;
	int player;

	void movePrint(){
		if(type == 0){
			cout<<"Move to : ";
		}
		else if(type == 1){
			cout<<"Place Horizontal wall at : ";
		}
		else{
			cout<<"Place Vertical wall at : ";
		}

		cout<<row<<","<<column<<endl;
	}

	Move(int t, int r, int c, int p) {
		type = t;
		row = r;
		column = c;
		player = p;
	}
	Move() {
		type, row, column, player = 0;
	}
};

/**
 * We will store the state in the graph, which will allow us to avoid conversions.
 * Because anyways most operations need that the state be a graph (checking paths etc.)
 */
class BGraph {
public:
	int rows;
	int columns;
	int walls;
	int me;

	int row1;
	int column1;
	int walls1;

	int row2;
	int column2;
	int walls2;

	float time;

	// Stores whether a wall has been placed at a vertex.
	vector<int> wallsList;

	vector<set<int> > edges;

	void addEdge(int v1, int v2) {
//		cout<<"Trying to add edge\n";
		edges[v1].insert(v2);
		edges[v2].insert(v1);
	}

	void removeEdge(int v1, int v2) {
		edges[v1].erase(v2);
		edges[v2].erase(v1);
	}

	bool checkEdge(int v1, int v2) {
		return edges[v1].find(v2) != edges[v1].end();
	}

	//Take row and column, return the vertex number.
	//For example, 1,1 -> 1, 2,1 -> row_length + 1
	int rc2v(int r, int c) {
		return max(0, columns * (r - 1) + c);
	}

	//Take vertex number, return (row,column)
	pair<int, int> v2rc(int v) {
		pair<int, int> ans;
		ans.first = v / columns + 1;
		ans.second = v % columns;
		if (ans.second == 0) {
			ans.second = columns;
		}
		return ans;
	}

	void addWall(Wall wall) {
		if (wall.horizontal) {
			removeEdge(rc2v(wall.row-1, wall.column), rc2v(wall.row, wall.column));
			removeEdge(rc2v(wall.row - 1, wall.column - 1), rc2v(wall.row, wall.column - 1));
			wallsList[rc2v(wall.row, wall.column)] = 1;
		} else {
			removeEdge(rc2v(wall.row - 1, wall.column), rc2v(wall.row - 1, wall.column - 1));
			removeEdge(rc2v(wall.row, wall.column - 1), rc2v(wall.row, wall.column));
			wallsList[rc2v(wall.row, wall.column)] = 2;
		}
	}

	//There are two conditions - the edges that it is trying to break should EXIST.
	bool wallIsLegal(Wall wall) {

		bool debug = wall.row == 2;

		if (wall.row <= rows && wall.row > 1 && wall.column <= columns && wall.column > 1) {
			if (wall.horizontal) {
				bool e1 = checkEdge(rc2v(wall.row-1, wall.column), rc2v(wall.row, wall.column));
				bool e2 = checkEdge(rc2v(wall.row - 1, wall.column - 1), rc2v(wall.row, wall.column - 1));
				bool e3 = wallsList[rc2v(wall.row, wall.column)] == 0;
				if(debug)
//					cout<<"horizont "<<wall.row<<" "<<wall.column<<" "<<e1<<" "<<e2<<" "<<e3<<endl;

				return e1 && e2 && e3;
			} else {
				bool e1 = checkEdge(rc2v(wall.row - 1, wall.column), rc2v(wall.row - 1, wall.column - 1));
				bool e2 = checkEdge(rc2v(wall.row, wall.column - 1), rc2v(wall.row, wall.column));
				bool e3 = wallsList[rc2v(wall.row, wall.column)] == 0;
				if(debug)
//					cout<<"vertical "<<wall.row<<" "<<wall.column<<" "<<e1<<" "<<e2<<" "<<e3<<endl;
				return e1 && e2 && e3;
			}
		} else {
			return false;
		}
	}

	//Removes/adds edges to account for the presence of a player.
	void adjustForPlayer(int player) {


		int i, j;
		if (player == 1) {
			i = row1;
			j = column1;
		} else {
			i = row2;
			j = column2;
		}

		// [a] [b] [c]
		// [h] [x] [d]
		// [g] [f] [e]

		bool e1 = (checkEdge(rc2v(i, j - 1), rc2v(i, j))); //h-x

		bool e2 = (checkEdge(rc2v(i, j), rc2v(i, j + 1))); //x-d

		bool e3 = (checkEdge(rc2v(i - 1, j), rc2v(i, j))); //b-x

		bool e4 = (checkEdge(rc2v(i, j), rc2v(i + 1, j))); //f-x




		if (e1 && e2) {
			removeEdge(rc2v(i, j - 1), rc2v(i, j));
			removeEdge(rc2v(i, j), rc2v(i, j + 1));
			addEdge(rc2v(i, j - 1), rc2v(i, j + 1));
		} else if (e1 && !e2) {
			removeEdge(rc2v(i, j - 1), rc2v(i, j));
			if (e3)
				addEdge(rc2v(i, j - 1), rc2v(i - 1, j));
			if (e4)
				addEdge(rc2v(i, j - 1), rc2v(i + 1, j));
		} else if (!e1 && e2) {
			removeEdge(rc2v(i, j), rc2v(i, j + 1));
			if (e3)
				addEdge(rc2v(i - 1, j), rc2v(i, j + 1));
			if (e4)
				addEdge(rc2v(i + 1, j), rc2v(i, j + 1));
		}

		if (e3 && e4) {
			removeEdge(rc2v(i - 1, j), rc2v(i, j));
			removeEdge(rc2v(i, j), rc2v(i + 1, j));
			addEdge(rc2v(i - 1, j), rc2v(i + 1, j));
		} else if (e3 && !e4) {
			removeEdge(rc2v(i - 1, j), rc2v(i, j));
			if (e1)
				addEdge(rc2v(i - 1, j), rc2v(i, j - 1));
			if (e2)
				addEdge(rc2v(i - 1, j), rc2v(i, j + 1));
		} else if (!e3 && e4) {
			removeEdge(rc2v(i, j), rc2v(i + 1, j));
			if (e1)
				addEdge(rc2v(i + 1, j), rc2v(i, j - 1));
			if (e2)
				addEdge(rc2v(i + 1, j), rc2v(i, j + 1));
		}
	}

	//reverse whatever was done by adjustForPlayer. Assumes that NOTHING ELSE HAS CHANGED!!
	void deadjustForPlayer(int player) {

		int i, j;
		if (player == 1) {
			i = row1;
			j = column1;
		} else {
			i = row2;
			j = column2;
		}

		// [a] [b] [c]
		// [h] [x] [d]
		// [g] [f] [e]
		bool e1 = (wallsList[rc2v(i, j)] == 0) || (wallsList[rc2v(i - 1, j)] == 0); //h-x
		bool e2 = (wallsList[rc2v(i + 1, j + 1)] == 0) || (wallsList[rc2v(i, j + 1)] == 0); //x-d
		bool e3 = (wallsList[rc2v(i, j)] == 0) || (wallsList[rc2v(i, j + 1)] == 0); //b-x
		bool e4 = (wallsList[rc2v(i + 1, j)] == 0) || (wallsList[rc2v(i + 1, j + 1)] == 0); //f-x

		if (e1 && e2) {
			addEdge(rc2v(i, j - 1), rc2v(i, j));
			addEdge(rc2v(i, j), rc2v(i, j + 1));
			removeEdge(rc2v(i, j - 1), rc2v(i, j + 1));
		} else if (e1 && !e2) {
			addEdge(rc2v(i, j - 1), rc2v(i, j));
			if (e3)
				removeEdge(rc2v(i, j - 1), rc2v(i - 1, j));
			if (e4)
				removeEdge(rc2v(i, j - 1), rc2v(i + 1, j));
		} else if (!e1 && e2) {
			addEdge(rc2v(i, j), rc2v(i, j + 1));
			if (e3)
				removeEdge(rc2v(i - 1, j), rc2v(i, j + 1));
			if (e4)
				removeEdge(rc2v(i + 1, j), rc2v(i, j + 1));
		}

		if (e3 && e4) {
			addEdge(rc2v(i - 1, j), rc2v(i, j));
			addEdge(rc2v(i, j), rc2v(i + 1, j));
			removeEdge(rc2v(i - 1, j), rc2v(i + 1, j));
		} else if (e3 && !e4) {
			addEdge(rc2v(i - 1, j), rc2v(i, j));
			if (e1)
				removeEdge(rc2v(i - 1, j), rc2v(i, j - 1));
			if (e2)
				removeEdge(rc2v(i - 1, j), rc2v(i, j + 1));
		} else if (!e3 && e4) {
			addEdge(rc2v(i, j), rc2v(i + 1, j));
			if (e1)
				removeEdge(rc2v(i + 1, j), rc2v(i, j - 1));
			if (e2)
				removeEdge(rc2v(i + 1, j), rc2v(i, j + 1));
		}
	}

	//Number of edges in path, -1 means no path.
	int bfs(int start, int dest) {
		set<int> visited;
		queue<int> todo;
		todo.push(start);
		int depth = 0;

		bool debug = start == 77 && dest == 1;

		while (true) {
			depth++;
			if (todo.empty()) {
				return -1;
			}
			queue<int> next;
			while (!todo.empty()) {
				int current = todo.front();
				todo.pop();
				set<int> neighbours = edges[current];
				std::set<int>::iterator iter;
				for (iter = neighbours.begin(); iter != neighbours.end(); ++iter) {
					int v = *iter;
					if (visited.find(v) == visited.end()) {
						next.push(v);
					}
					if (v == dest) {
						return depth;
					}
				}
				visited.insert(current);
			}
			todo = next;
		}
	}

	BGraph(int r, int c, int k, int t, int m) {
		rows = r;
		columns = c;
		walls = k;
		time = t;
		me = m;

		row1 = r;
		column1 = c / 2 + 1;
		walls1 = k;
		row2 = 1;
		column2 = c / 2 + 1;
		walls2 = k;


		//r*c+1 so that we don't have to worry about the fact that these things are actually indexed from zero.
		wallsList = vector<int>(r * c + 1, 0);

		for (int i = 1; i <= r * c + 1; i++) {
			set<int> s;
			edges.push_back(s);
		}

		//Initialise horizontal edges.
		for (int i = 1; i <= r; i++) {
			for (int j = 1; j < c; j++) {
				addEdge(rc2v(i, j), rc2v(i, j + 1));
			}
		}

		//Initialise vertical edges.
		for (int j = 1; j <= c; j++) {
			for (int i = 1; i < r; i++) {
				addEdge(rc2v(i, j), rc2v(i + 1, j));
			}
		}
	}

	BGraph(){
		int rows;
			columns, walls, me, row1, column1, walls1, row2, column2, walls2, time = 0;
	}

	//Copy Constructor
	BGraph(const BGraph & obj) {
		rows = obj.rows;
		columns = obj.columns;
		walls = obj.walls;
		me = obj.me;

		row1 = obj.row1;
		column1 = obj.column1;
		walls1 = obj.walls1;

		row2 = obj.row2;
		column2 = obj.column2;
		walls2 = obj.walls2;

		time = obj.time;

		wallsList = obj.wallsList;
		edges = obj.edges;
	}


	bool operator==(const BGraph& other) {
	  return row1 == other.row1 && column1 == other.column1 && walls1 == other.walls1
			  && row2 == other.row2 && column2 == other.column2 && walls2 == other.walls2
			  && wallsList == other.wallsList;
	}

};

//===========================================================================================

/* Complete the function below to print 1 integer which will be your next move 
 */
int N, M, K, time_left, player;

//Standard matrix - top left cell is 1,1
struct Board {
	int rows, columns, k, me;

	float time;

	int row1, column1, walls1, row2, column2, walls2;

	vector<Wall> walls;
};

//int getVNum(int row, int column, Board board) {
//	return board.columns * (row - 1) + column;
//}

//pair<int, int> getRC(int v, Board board) {
//	pair<int, int> loc;
//	loc.first = v / board.columns + 1;
//	loc.second = v % board.columns;
//	return loc;
//}

//player specifies whose turn it is. Because the graph depends on the *opponent* of the *current* player.
BGraph boardToGraph(Board board, int player) {

	//Create the graph
	BGraph bGraph = BGraph(board.rows, board.columns, board.k, board.time, board.me);

	//Delete edges where there are walls.
	for (int iter = 0; iter < board.walls.size(); iter++) {
		Wall wall = board.walls[iter];

		//   [i-1,j-1]   [i-1,j]
		//             .             i,j
		//	 [i,j-1]     [i,j]

		bGraph.addWall(wall);
	}

	//Adjust for the location of the opponent.
	// [a] [b] [c]
	// [h] [x] [d]
	// [g] [f] [e]
	// If there is no wall/board edge, remove x-b, x-f edges (x => opponent), and create b-f edge, and similarly h-d edge.
	// Else, if b is wall(or equivalently end of board), remove x-f, create h-f, d-f.
	int i, j = 0;
	if (player == 1) {
		i = board.row2;
		j = board.column2;
	} else {
		i = board.row1;
		j = board.column1;
	}

	bool e1 = (bGraph.checkEdge(bGraph.rc2v(i, j - 1), bGraph.rc2v(i, j))); //h-x
	bool e2 = (bGraph.checkEdge(bGraph.rc2v(i, j), bGraph.rc2v(i, j + 1))); //x-d
	bool e3 = (bGraph.checkEdge(bGraph.rc2v(i - 1, j), bGraph.rc2v(i, j))); //b-x
	bool e4 = (bGraph.checkEdge(bGraph.rc2v(i, j), bGraph.rc2v(i + 1, j))); //f-x

	if (e1 && e2) {
		bGraph.removeEdge(bGraph.rc2v(i, j - 1), bGraph.rc2v(i, j));
		bGraph.removeEdge(bGraph.rc2v(i, j), bGraph.rc2v(i, j + 1));
		bGraph.addEdge(bGraph.rc2v(i, j - 1), bGraph.rc2v(i, j + 1));
	} else if (e1 && !e2) {
		bGraph.removeEdge(bGraph.rc2v(i, j - 1), bGraph.rc2v(i, j));
		if (e3)
			bGraph.addEdge(bGraph.rc2v(i, j - 1), bGraph.rc2v(i - 1, j));
		if (e4)
			bGraph.addEdge(bGraph.rc2v(i, j - 1), bGraph.rc2v(i + 1, j));
	} else if (!e1 && e2) {
		bGraph.removeEdge(bGraph.rc2v(i, j), bGraph.rc2v(i, j + 1));
		if (e3)
			bGraph.addEdge(bGraph.rc2v(i - 1, j), bGraph.rc2v(i, j + 1));
		if (e4)
			bGraph.addEdge(bGraph.rc2v(i + 1, j), bGraph.rc2v(i, j + 1));
	}

	if (e3 && e4) {
		bGraph.removeEdge(bGraph.rc2v(i - 1, j), bGraph.rc2v(i, j));
		bGraph.removeEdge(bGraph.rc2v(i, j), bGraph.rc2v(i + 1, j));
		bGraph.addEdge(bGraph.rc2v(i - 1, j), bGraph.rc2v(i + 1, j));
	} else if (e3 && !e4) {
		bGraph.removeEdge(bGraph.rc2v(i - 1, j), bGraph.rc2v(i, j));
		if (e1)
			bGraph.addEdge(bGraph.rc2v(i - 1, j), bGraph.rc2v(i, j - 1));
		if (e2)
			bGraph.addEdge(bGraph.rc2v(i - 1, j), bGraph.rc2v(i, j + 1));
	} else if (!e3 && e4) {
		bGraph.removeEdge(bGraph.rc2v(i, j), bGraph.rc2v(i + 1, j));
		if (e1)
			bGraph.addEdge(bGraph.rc2v(i + 1, j), bGraph.rc2v(i, j - 1));
		if (e2)
			bGraph.addEdge(bGraph.rc2v(i + 1, j), bGraph.rc2v(i, j + 1));
	}

	return bGraph;
}

//Finds a path from the given vertex to the edge to the board.
// mode -> 1 = find if path EXISTS
// mode -> 2 = find SHORTEST path
int pathfinder(BGraph bGraph, int player, int mode) {

	bool debug = bGraph.wallsList[18] == 1 && player == 1;

//	if (debug) {
//		for (int i = 1; i < 19; i++) {
//			cout << endl << i << " -> ";
//			set<int>::iterator iter;
//			set<int> neighbours = bGraph.edges[i];
//			for (iter = neighbours.begin(); iter != neighbours.end(); ++iter) {
//				int n = *iter;
//				cout << n << ",";
//			}
//		}
//		cout<<endl;
//	}

	int vertex, dest;
	if (player == 1) {
		vertex = bGraph.rc2v(bGraph.row1, bGraph.column1);
		dest = 1;
	} else {
		vertex = bGraph.rc2v(bGraph.row2, bGraph.column1);
		dest = (bGraph.rows - 1) * bGraph.columns + 1;;
	}

	int shortestPath = INT_MAX;
	bool found = false;
	for (int i = dest; i < dest + bGraph.columns; i++) {
//		if(debug)
//			cout<<"Trying to go from "<<vertex<<" to "<<i<<endl;
		int distance = bGraph.bfs(vertex, i);
//		if(debug) cout<<distance<<endl;
		if (distance != -1) {
			found = true;
			if (mode == 1) {
				return 1;
			}
			if (found < shortestPath)
				shortestPath = distance;
		}
	}
	if (mode == 1) {
//		if(debug)
//			cout<<"\n-------------\n"<<found<<endl;
		return (int) (found == true);
	}
	return shortestPath;
}

//The possible moves depend on which player it is.
vector<Move> movegen(BGraph bGraph, int player) {

	int row,column,v;
	if(player==1){
		row = bGraph.row1;
		column = bGraph.column1;
	} else{
		row = bGraph.row2;
		column = bGraph.column2;
	}

	int opponent = (player==1) ? 2 : 1;

	v = bGraph.rc2v(row, column);

	//These two will be used for path checking.
	BGraph opponentGraph = bGraph;
	BGraph playerGraph = bGraph;

	playerGraph.adjustForPlayer(opponent);

	opponentGraph.adjustForPlayer(player);

	vector<Move> moves;
	moves.reserve(2 * bGraph.rows * bGraph.columns + 4);


	//Moves for moving the pawn.
	set<int> neighbours = playerGraph.edges[v];
	set<int>::iterator iter;
	for(iter = neighbours.begin(); iter!=neighbours.end(); ++iter){
		int n = *iter;
		moves.push_back(Move(0, bGraph.v2rc(n).first, bGraph.v2rc(n).second, player));
	}



	//Moves for the walls.
	for(int i=1; i<bGraph.wallsList.size(); i++){

		if(bGraph.wallsList[i] == 0){

			BGraph t_pGraph = playerGraph;
			BGraph t_oGraph = opponentGraph;

			//Try to place H-wall there.
			Wall hWall = Wall(true, bGraph.v2rc(i).first, bGraph.v2rc(i).second);
			if(bGraph.wallIsLegal(hWall)){
				t_pGraph.addWall(hWall);
				t_oGraph.addWall(hWall);
//				cout<<"Testing path existances\n";
//				cout<<pathfinder(t_pGraph, player, 1)<<endl;
//				cout<<pathfinder(t_oGraph, opponent, 1)<<endl;
//				if(hWall.row==2){
//					for (int i = 1; i < 19; i++) {
//						cout << endl << i << " -> ";
//						set<int>::iterator iter;
//						set<int> neighbours = t_oGraph.edges[i];
//						for (iter = neighbours.begin(); iter != neighbours.end(); ++iter) {
//							int n = *iter;
//							cout << n << ",";
//						}
//					}
//				}
				if(pathfinder(t_pGraph, player, 1)==1 && pathfinder(t_oGraph, opponent, 1) == 1){
					Move hMove = Move(1, hWall.row, hWall.column, player);
					moves.push_back(hMove);
				}
			}

			BGraph t2_pGraph = playerGraph;
			BGraph t2_oGraph = opponentGraph;

			//Try to place W-wall there.
			Wall vWall = Wall(false, bGraph.v2rc(i).first, bGraph.v2rc(i).second);
			if(bGraph.wallIsLegal(vWall)){

				t2_pGraph.addWall(vWall);
				t2_oGraph.addWall(vWall);
				if(pathfinder(t2_pGraph, player, 1)==1 && pathfinder(t2_oGraph, opponent, 1) == 1){
					Move vMove = Move(2, vWall.row, vWall.column, player);
					moves.push_back(vMove);
				}
			}
		}
	}

	progress = progress + moves.size();
	cout<<progress<<endl;

	return moves;
}

//Utility is ALWAYS calculated assuming that we are player1. If that's not the case, just reverse the sign.
float utility(BGraph bGraph) {
	int score = 0;

	score += bGraph.rows - bGraph.row1;
	score -= bGraph.row2 - 1;
	score += bGraph.walls1;
	score -= bGraph.walls2;

	if (bGraph.me == 2) {
		score = -1 * score;
	}
	return score;
}

BGraph applyMove(BGraph bGraph, Move move) {


	if (move.type == 0) {
		if (move.player == 1) {
			bGraph.row1 = move.row;
			bGraph.column1 = move.column;
		}

		else {
			bGraph.row2 = move.row;
			bGraph.column2 = move.column;
		}
	} else {
		Wall wall = Wall(true, move.row, move.column);
		if (move.type != 1)
			wall.horizontal = false;
		bGraph.addWall(wall);
	}

	return bGraph;
}

//================================================================================================================================================



int inf = 99999;
int neginf = -99999;
int d;
// int player,opp;
bool finish(BGraph board) {
	return ((board.row2 == board.rows) && (board.row1 == 1));
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
	{
		movelist = movegen(node, node.me);
	}
	else{
		movelist = movegen(node, 3 - (node.me));
	}

	vector<pv> ans;
	if (path.size() > (d - depth))
	reorder(node, movelist, path, (d - depth));


	for (int i = 0; i < movelist.size(); i++) {



		BGraph child = applyMove(node, movelist[i]);
		if (finish(child) || depth == 1) {
			int val = color * utility(child);
			if (bestValue < val) {
				bestMove = movelist[i];
				bestState = child;
			}
			bestValue = max(bestValue, val);

			alpha = max(alpha, val);
			if (alpha >= beta) {

				break;
			}
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
			if (alpha >= beta) {

				break;
			}
			 }

	}

	pv edge;
	edge.mov = bestMove;
	edge.value = bestValue;
	edge.state = bestState;
	ans.insert(ans.begin(), edge);
	//if(depth>1 && i>56) cout << "depth:"<<depth << endl;
	return ans;
}

Move ITERATIVE_NEGAMAX(BGraph board) {
	d = 1;
	vector<pv> path;
	Move m;
	while (d < 2)           //while time's left
	{
		path = NEGAMAX(board, d, neginf, inf, 1, path);
		m = (path[0]).mov;
		m.movePrint();
		d = d + 1;
	}
	cout << "iter-nega returned : ";
	m.movePrint();
	return m;
}

//=================================================================================================================================================
int main(){
	BGraph bGraph = BGraph(9,9,10,60,1);
	cout<<"Bgraph constructed"<<endl;

//	bool test = bGraph.edges[0].find(14) != bGraph.edges[0].end();
//	cout<<test<<endl;
//	return test;

	Move move = ITERATIVE_NEGAMAX(bGraph);
	move.movePrint();

//	bGraph.addWall(Wall(false, 2, 5));
//	bGraph.addWall(Wall(false, 2, 6));

//	cout<<bGraph.edges.size()<<endl;

//	for(int i=0; i<19; i++){
//		cout<<endl<<i<<" -> ";
//		set<int>::iterator iter;
//		set<int> neighbours = bGraph.edges[i];
//		for(iter = neighbours.begin(); iter!=neighbours.end(); ++iter){
//				int n = *iter;
//				cout<<n<<",";
//		}
//	}


//	vector<Move> moves = movegen(bGraph,1);
//	for(int i=0; i<moves.size(); i++){
//		moves[i].movePrint();
//	}
}


//int main2(int argc, char *argv[]) {
//	srand(time(NULL));
//	int sockfd = 0, n = 0;
//	char recvBuff[1024];
//	char sendBuff[1025];
//	struct sockaddr_in serv_addr;
//
//	if (argc != 3) {
//		printf("\n Usage: %s <ip of server> <port no> \n", argv[0]);
//		return 1;
//	}
//
//	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
//		printf("\n Error : Could not create socket \n");
//		return 1;
//	}
//
//	memset(&serv_addr, '0', sizeof(serv_addr));
//
//	serv_addr.sin_family = AF_INET;
//	serv_addr.sin_port = htons(atoi(argv[2]));
//
//	if (inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) <= 0) {
//		printf("\n inet_pton error occured\n");
//		return 1;
//	}
//
//	if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
//		printf("\n Error : Connect Failed \n");
//		return 1;
//	}
//
////==================Game starts from here=====================================
//
//	cout << "Quoridor will start..." << endl;
//
//	memset(recvBuff, '0', sizeof(recvBuff));
//	n = read(sockfd, recvBuff, sizeof(recvBuff) - 1);
//	recvBuff[n] = 0;
//	sscanf(recvBuff, "%d %d %d %d %d", &player, &N, &M, &K, &time_left);
//
//	BGraph bGraph = BGraph(N, M, K, time_left, player);
//
//
//	cout << "Player " << player << endl;
//	cout << "Time " << time_left << endl;
//	cout << "Board size " << N << "x" << M << " :" << K << endl;
//	float TL;
//	int om, oro, oc;
//	int m, r, c;
//	int d = 3;
//	char s[100];
//	int x = 1;
//	if (player == 1) {
//
//		memset(sendBuff, '0', sizeof(sendBuff));
//		string temp;
//
//		//Get a move from player.
////		cin>>m>>r>>c;
//
//		//This move had better have it's player set to the correct value.
//		Move move = minimax(bGraph);
//
//		bGraph = applyMove(bGraph, move);
//
//		m = move.type;
//		r = move.row;
//		c = move.column;
//
//		snprintf(sendBuff, sizeof(sendBuff), "%d %d %d", m, r, c);
//		write(sockfd, sendBuff, strlen(sendBuff));
//
//		memset(recvBuff, '0', sizeof(recvBuff));
//		n = read(sockfd, recvBuff, sizeof(recvBuff) - 1);
//		recvBuff[n] = 0;
//		sscanf(recvBuff, "%f %d", &TL, &d);
//		cout << TL << " " << d << endl;
//		if (d == 1) {
//			cout << "You win!! Yayee!! :D ";
//			x = 0;
//		} else if (d == 2) {
//			cout << "Loser :P ";
//			x = 0;
//		}
//	}
//
//	while (x) {
//		memset(recvBuff, '0', sizeof(recvBuff));
//
//		//Get move of opponent, and game status.
//		n = read(sockfd, recvBuff, sizeof(recvBuff) - 1);
//		recvBuff[n] = 0;
//		sscanf(recvBuff, "%d %d %d %d", &om, &oro, &oc, &d);
//
//		Move omove = Move(om, oro, oc, 1);
//		omove.player = (bGraph.me == 1) ? 2 : 1;
//
//		bGraph = applyMove(bGraph, omove);
//
//		cout << om << " " << oro << " " << oc << " " << d << endl;
//		if (d == 1) {
//			cout << "You win!! Yayee!! :D ";
//			break;
//		} else if (d == 2) {
//			cout << "Loser :P ";
//			break;
//		}
//		memset(sendBuff, '0', sizeof(sendBuff));
//		string temp;
//
//		//Get a move from player.
////		cin>>m>>r>>c;
//
//		Move move = minimax(bGraph);
//
//		bGraph = applyMove(bGraph, move);
//
//		m = move.type;
//		r = move.row;
//		c = move.column;
//
//		snprintf(sendBuff, sizeof(sendBuff), "%d %d %d", m, r, c);
//		write(sockfd, sendBuff, strlen(sendBuff));
//
//		memset(recvBuff, '0', sizeof(recvBuff));
//		n = read(sockfd, recvBuff, sizeof(recvBuff) - 1);
//		recvBuff[n] = 0;
//		sscanf(recvBuff, "%f %d", &TL, &d); //d=3 indicates game continues.. d=2 indicates lost game, d=1 means game won.
//		cout << TL << " " << d << endl;
//		if (d == 1) {
//			cout << "You win!! Yayee!! :D ";
//			break;
//		} else if (d == 2) {
//			cout << "Loser :P ";
//			break;
//		}
//	}
//	cout << endl << "The End" << endl;
//	return 0;
//}
