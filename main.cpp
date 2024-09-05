#include "chess-library/include/chess.hpp"
#include <chrono>

using namespace chess;
using namespace std;

long minimax_searched = 0;

int evaluation(Board& board) {
	return 0;
}

int minimax (Board& board, int depth) {
	minimax_searched++;
	Movelist moves;
	movegen::legalmoves(moves, board);

	if (depth == 0) {
		return evaluation(board);
	}

	int eval = 0;

	for (int i = 0; i < moves.size(); i++) {
		const auto move = moves[i];
 		board.makeMove(move);
		eval = minimax(board, depth - 1);
        board.unmakeMove(move);
    }
	
	return eval;
}

int main () {
    Board board = Board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    Movelist moves;
    movegen::legalmoves(moves, board);



	auto start = std::chrono::high_resolution_clock::now();
	minimax(board, 6);	
	auto end = std::chrono::high_resolution_clock::now();
    chrono::duration<double> duration = end - start;
    cout << "Execution time: " << duration.count() << " seconds\n";

	printf("nodes: %ld\n", minimax_searched);
    return 0;
}
