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

#include "boost/graph/adjacency_list.hpp"

using namespace std;
/* Complete the function below to print 1 integer which will be your next move 
 */
int N, M, K, time_left, player;

struct Wall {
	bool horizontal;
	int row;
	int column;
};

struct Move {
	// 0 - pawn, 1 - h. wall, 2 - v. wall
	int type;
	int row;
	int column;
};

//Standard matrix - top left cell is 1,1
struct Board {
	int rows, columns, k, me;

	float time;

	int myRow, myColumn, myWalls, oRow, oColumn, oWalls;

	vector<Wall> walls;
};

int getVNum(int row, int column, Board board) {
	return board.rows * (row - 1) + column;
}

boost::adjacency_list boardToGraph(Board board) {

	//Create the graph
	boost::adjacency_list<boost::listS, boost::vecS, boost::undirectedS> bGraph(
			board.rows * board.columns);

	//Add horizontal edges
	for (int i = 1; i <= board.rows; i++) {
		for (int j = 1; j < board.columns; j++) {
			boost::add_edge(board.columns * (i - 1) + j,
					board.columns * (i - 1) + j + 1, bGraph);
		}
	}

	//Add vertical edges
	for (int j = 1; j <= board.columns; j++) {
		for (int i = 1; i < board.rows; i++) {
			boost::add_edge(board.columns * (i - 1) + j,
					board.columns * (i) + j, bGraph);
		}
	}

	//Delete edges where there are walls.
	for (int i = 0; i < board.walls.size(); i++) {
		Wall wall = board.walls[i];

		//   [i-1,j-1]   [i-1,j]
		//             .             i,j
		//	 [i,j-1]     [i,j]

		int i = wall.row;
		int j = wall.column;
		if (wall.horizontal) {
			boost::remove_edge(getVNum(i - 1, j - 1, board),
					getVNum(i, j - 1, board), bGraph);
			boost::remove_edge(getVNum(i - 1, j, board), getVNum(i, j, board),
					bGraph);
		} else {
			boost::remove_edge(getVNum(i - 1, j - 1, board),
					getVNum(i - 1, j, board), bGraph);
			boost::remove_edge(getVNum(i, j - 1, board), getVNum(i, j, board),
					bGraph);
		}
	}

	//Adjust for the location of the opponent.
	// [a] [b] [c]
	// [h] [x] [d]
	// [g] [f] [e]
	// If there is no wall/board edge, remove x-b, x-f edges (x => opponent), and create b-f edge, and similarly h-d edge.
	// Else, if b is wall(or equivalently end of board), remove x-f, create h-f, d-f.

	//TODO The case where there are TWO walls surrounding x. In that case, what happens to the paths? For example,
	// [a] [b] [c]
	// ++++++++
	// [h] [x]+[d]
	// [g] [f]+[e]
	// In this case, the code will go ahead and create an edge from h-b, but that doesn't seem right.
	int i = board.oRow;
	int j = board.oColumn;
	bool e1 = (boost::edge(getVNum(i, j - 1, board), getVNum(i, j, board),
			bGraph)).second; //h-x
	bool e2 = (boost::edge(getVNum(i, j, board), getVNum(i, j + 1, board),
			bGraph)).second; //x-d
	bool e3 = (boost::edge(getVNum(i - 1, j, board), getVNum(i, j, board),
			bGraph)).second; //b-x
	bool e4 = (boost::edge(getVNum(i, j, board), getVNum(i + 1, j, board),
			bGraph)).second; //f-x

	if (e1 && e2) {
		boost::remove_edge(getVNum(i, j - 1, board), getVNum(i, j, board),
				bGraph);
		boost::remove_edge(getVNum(i, j, board), getVNum(i, j + 1, board),
				bGraph);
		boost::add_edge(getVNum(i, j - 1, board), getVNum(i, j + 1, board),
				bGraph);
	} else if (e1 && !e2) {
		boost::remove_edge(getVNum(i, j - 1, board), getVNum(i, j, board),
				bGraph);
		boost::add_edge(getVNum(i, j - 1, board), getVNum(i - 1, j, board),
				bGraph);
		boost::add_edge(getVNum(i, j - 1, board), getVNum(i + 1, j, board),
				bGraph);
	} else if (!e1 && e2) {
		boost::remove_edge(getVNum(i, j, board), getVNum(i, j + 1, board),
				bGraph);
		boost::add_edge(getVNum(i - 1, j, board), getVNum(i, j + 1, board),
				bGraph);
		boost::add_edge(getVNum(i + 1, j, board), getVNum(i, j + 1, board),
				bGraph);
	}

	if (e3 && e4) {
		boost::remove_edge(getVNum(i - 1, j, board), getVNum(i, j, board),
				bGraph);
		boost::remove_edge(getVNum(i, j, board), getVNum(i + 1, j, board),
				bGraph);
		boost::add_edge(getVNum(i - 1, j, board), getVNum(i + 1, j, board),
				bGraph);
	} else if (e3 && !e4) {
		boost::remove_edge(getVNum(i - 1, j, board), getVNum(i, j, board),
				bGraph);
		boost::add_edge(getVNum(i - 1, j, board), getVNum(i, j - 1, board),
				bGraph);
		boost::add_edge(getVNum(i - 1, j, board), getVNum(i, j + 1, board),
				bGraph);
	} else if (!e3 && e4) {
		boost::remove_edge(getVNum(i, j, board), getVNum(i + 1, j, board),
				bGraph);
		boost::add_edge(getVNum(i + 1, j, board), getVNum(i, j - 1, board),
				bGraph);
		boost::add_edge(getVNum(i + 1, j, board), getVNum(i, j + 1, board),
				bGraph);
	}

	return bGraph;
}

vector<Move> movegen(Board board) {
	vector<Move> moves;
	moves.reserve(board.rows * board.columns + 4);

}

float utility(Board board) {
	int score = 0;

	score += board.rows - board.myRow;
	score -= board.oRow - 1;

	return score;
}

Board applyMove(Board board, Move move, int player){
	//TODO
	return board;
}

//================================================================================================================================================

Move minimax(Board board) {
	//Do your thing
}

//=================================================================================================================================================

int main(int argc, char *argv[]) {
	srand(time(NULL));
	int sockfd = 0, n = 0;
	char recvBuff[1024];
	char sendBuff[1025];
	struct sockaddr_in serv_addr;

	if (argc != 3) {
		printf("\n Usage: %s <ip of server> <port no> \n", argv[0]);
		return 1;
	}

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("\n Error : Could not create socket \n");
		return 1;
	}

	memset(&serv_addr, '0', sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(argv[2]));

	if (inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) <= 0) {
		printf("\n inet_pton error occured\n");
		return 1;
	}

	if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))
			< 0) {
		printf("\n Error : Connect Failed \n");
		return 1;
	}

//==================Game starts from here=====================================

	cout << "Quoridor will start..." << endl;

	memset(recvBuff, '0', sizeof(recvBuff));
	n = read(sockfd, recvBuff, sizeof(recvBuff) - 1);
	recvBuff[n] = 0;
	sscanf(recvBuff, "%d %d %d %d %d", &player, &N, &M, &K, &time_left);

	Board board = new Board();
	board.rows = N;
	board.columns = M;
	board.walls = K;
	board.time = time_left;
	board.me = player;

	cout << "Player " << player << endl;
	cout << "Time " << time_left << endl;
	cout << "Board size " << N << "x" << M << " :" << K << endl;
	float TL;
	int om, oro, oc;
	int m, r, c;
	int d = 3;
	char s[100];
	int x = 1;
	if (player == 1) {

		memset(sendBuff, '0', sizeof(sendBuff));
		string temp;

		//Get a move from player.
//		cin>>m>>r>>c;
		Move move = minimax(board);

		board = applyMove(board, move, 1);

		m = move.type;
		r = move.row;
		c = move.column;

		snprintf(sendBuff, sizeof(sendBuff), "%d %d %d", m, r, c);
		write(sockfd, sendBuff, strlen(sendBuff));

		memset(recvBuff, '0', sizeof(recvBuff));
		n = read(sockfd, recvBuff, sizeof(recvBuff) - 1);
		recvBuff[n] = 0;
		sscanf(recvBuff, "%f %d", &TL, &d);
		cout << TL << " " << d << endl;
		if (d == 1) {
			cout << "You win!! Yayee!! :D ";
			x = 0;
		} else if (d == 2) {
			cout << "Loser :P ";
			x = 0;
		}
	}

	while (x) {
		memset(recvBuff, '0', sizeof(recvBuff));

		//Get move of opponent, and game status.
		n = read(sockfd, recvBuff, sizeof(recvBuff) - 1);
		recvBuff[n] = 0;
		sscanf(recvBuff, "%d %d %d %d", &om, &oro, &oc, &d);

		Move omove = Move();
		omove.type = om;
		omove.row = oro;
		omove.column = oc;

		board = applyMove(board, move, 2)

		cout << om << " " << oro << " " << oc << " " << d << endl;
		if (d == 1) {
			cout << "You win!! Yayee!! :D ";
			break;
		} else if (d == 2) {
			cout << "Loser :P ";
			break;
		}
		memset(sendBuff, '0', sizeof(sendBuff));
		string temp;

		//Get a move from player.
//		cin>>m>>r>>c;

		Move move = minimax(board);

		board = applyMove(board, move, 1);

		m = move.type;
		r = move.row;
		c = move.column;

		snprintf(sendBuff, sizeof(sendBuff), "%d %d %d", m, r, c);
		write(sockfd, sendBuff, strlen(sendBuff));

		memset(recvBuff, '0', sizeof(recvBuff));
		n = read(sockfd, recvBuff, sizeof(recvBuff) - 1);
		recvBuff[n] = 0;
		sscanf(recvBuff, "%f %d", &TL, &d); //d=3 indicates game continues.. d=2 indicates lost game, d=1 means game won.
		cout << TL << " " << d << endl;
		if (d == 1) {
			cout << "You win!! Yayee!! :D ";
			break;
		} else if (d == 2) {
			cout << "Loser :P ";
			break;
		}
	}
	cout << endl << "The End" << endl;
	return 0;
}
