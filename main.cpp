#include "chess-library/include/chess.hpp"
#include "constants.hpp"

#include <chrono>

using namespace chess;
using namespace std;

long minimax_searched = 0;

int evaluation(Board& board) {
	int evaluation = 0;

	for (int i = 0; i < BOARD_SIZE; i++) {
		auto piece = board.at(Square((uint8_t)i));
		if (piece != Piece::NONE) {
			if (piece.color() == Color::WHITE) {
				evaluation += PST[piece.type()][BOARD_SIZE - 1 - i];
			} else {
				evaluation -= PST[piece.type()][i];
			}
		} 
	}

	return evaluation;
}

int minimax (Board& board, int depth, int alpha, int beta, Color color) {
	minimax_searched++;
	Movelist moves;

	if (depth == 0) {
		return evaluation(board);
	}

	if (color == Color::WHITE) {
		int max_eval = -MAX_SCORE;
		movegen::legalmoves(moves, board);
		for (int i = 0; i < moves.size(); i++) {
			const auto move = moves[i];
			board.makeMove(move);
			int eval = minimax(board, depth - 1, alpha, beta, 1 - color);
			board.unmakeMove(move);

			max_eval = eval > max_eval ? eval:max_eval;
			alpha = alpha > max_eval ? alpha:max_eval;

			if (beta <= alpha) break;
		}

		return max_eval;
	} else {
		int min_eval = MAX_SCORE;
		movegen::legalmoves(moves, board);
		for (int i = 0; i < moves.size(); i++) {
			const auto move = moves[i];
			board.makeMove(move);
			int eval = minimax(board, depth - 1, alpha, beta, 1 - color);
			board.unmakeMove(move);

			min_eval = eval < min_eval ? eval:min_eval;
			beta = beta < min_eval ? beta:min_eval;

			if (beta <= alpha) break;
		}

		return min_eval;
	}
}

int main (int argc, char *argv[]) {
    Board board = Board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    Movelist moves;
    movegen::legalmoves(moves, board);
	int depth = atoi(argv[1]);

	auto start = std::chrono::high_resolution_clock::now();
	int eval = minimax(board, depth, -MAX_SCORE, MAX_SCORE, Color::WHITE);	
	auto end = std::chrono::high_resolution_clock::now();
    chrono::duration<double> duration = end - start;
    cout << "Execution time: " << duration.count() << " seconds\n";

	printf("nodes: %ld\n", minimax_searched);
	printf("eval: %d\n", eval);
    return 0;
}
