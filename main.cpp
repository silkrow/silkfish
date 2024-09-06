#include "chess-library/include/chess.hpp"
#include "constants.hpp"

#include <chrono>

using namespace chess;
using namespace std;

long minimax_searched = 0;

Board board;

int evaluation(Board& board) {
	int evaluation = 0;

	if (board.isGameOver().second == GameResult::DRAW) {
		return 0;
	}

	if (board.isGameOver().first == GameResultReason::CHECKMATE) {
		return board.sideToMove() == Color::BLACK ? MAX_SCORE:-MAX_SCORE;
	}

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

bool compare_moves (Move a, Move b) {
	int a_val = 0, b_val = 0;

	if (a == Move::CASTLING) a_val = CASTLE; // Castle is encoded as king capturing rook in the library!
	else if (board.isCapture(a)) {
		int af = (int)((board.at<Piece>(a.from())).type());
		int at = (int)((board.at<Piece>(a.to())).type());
		
		std::pair<int, int> key = std::make_pair(af, at % 6);
		a_val = capture_score[key];
	}

	if (b == Move::CASTLING) b_val = CASTLE;
	else if (board.isCapture(b)) {
		int bf = (int)((board.at<Piece>(b.from())).type());
		int bt = (int)((board.at<Piece>(b.to())).type());
		
		std::pair<int, int> key = std::make_pair(bf, bt % 6);
		b_val = capture_score[key];
	}

    return a_val > b_val;
}

int minimax (int depth, int alpha, int beta, Color color) {
	minimax_searched++;
	Movelist moves;

	if (depth == 0) {
		return evaluation(board);
	}

	if (board.isGameOver().second == GameResult::DRAW) {
		return 0;
	}

	if (board.isGameOver().first == GameResultReason::CHECKMATE) {
		return board.sideToMove() == Color::BLACK ? MAX_SCORE:-MAX_SCORE;
	}

	movegen::legalmoves(moves, board);

	sort(moves.begin(), moves.end(), compare_moves);

	if (color == Color::WHITE) {
		int max_eval = -MAX_SCORE;
		for (int i = 0; i < moves.size(); i++) {
			const auto move = moves[i];
			board.makeMove(move);
			int eval = minimax(depth - 1, alpha, beta, 1 - color);
			board.unmakeMove(move);

			max_eval = eval > max_eval ? eval:max_eval;
			alpha = alpha > max_eval ? alpha:max_eval;

			if (beta <= alpha) break;
		}

		if (max_eval < B_WIN_THRE) return max_eval + 1;
		else if (max_eval > W_WIN_THRE) return max_eval - 1;
		return max_eval;
	} else {
		int min_eval = MAX_SCORE;
		for (int i = 0; i < moves.size(); i++) {
			const auto move = moves[i];
			board.makeMove(move);
			int eval = minimax(depth - 1, alpha, beta, 1 - color);
			board.unmakeMove(move);

			min_eval = eval < min_eval ? eval:min_eval;
			beta = beta < min_eval ? beta:min_eval;

			if (beta <= alpha) break;
		}

		if (min_eval < B_WIN_THRE) return min_eval + 1;
		else if (min_eval > W_WIN_THRE) return min_eval - 1;
		return min_eval;
	}
}

void usage_error() {
	cout << "Usage: ./silkrow <-m> <depth> \"{fen_string}\" or ./silkrow <-m> demo <depth>" << endl;
	return;
}

Color fen_player_color(string& fen) {
	size_t spacePos = fen.find(' ');
    if (spacePos != string::npos) {
        char toMove = fen[spacePos + 1];
        if (toMove == 'w') {
            return Color::WHITE;
        } else if (toMove == 'b') {
            return Color::BLACK;
        }
    }

    throw std::invalid_argument("Invalid FEN string format");
}

int main (int argc, char *argv[]) {
	int depth = DEFAULT_DEPTH;
	string fen_string = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"; 
	fen_string = "4k3/8/6K1/8/3Q4/8/8/8 w - - 0 1"; // for testing purpose
	bool demo_mode = false;
	bool mute = false;

	if (argc == 1){
		usage_error();
		return 1;
	}

    int arg_start = 1;
    if (string(argv[1]) == "-m") {
        mute = true;
        arg_start = 2;
    }

	if (arg_start > argc - 1) {
		usage_error();
		return 1;
	}

	string second_arg = argv[arg_start];
	if (second_arg == "demo") { // demo mode
		demo_mode = true;
		if (argc - 1 == arg_start + 1) {
			string third_arg = argv[arg_start + 1];
			try {
				size_t pos;
				depth = stoi(third_arg, &pos);
				if (pos != third_arg.length()) {
					usage_error();
					return 1;
				}
			} catch (const std::invalid_argument&) {
				usage_error();
				return 1;
    		} catch (const std::out_of_range&) {
        		usage_error();
				return 1;
			}
		}
	} else { // customized fen 
		try {
			size_t pos;
			depth = stoi(second_arg, &pos);
			if (pos == second_arg.length()) {
				arg_start++;
			} else {
				depth = DEFAULT_DEPTH;
			}
		} catch (const std::invalid_argument&) {
		} catch (const std::out_of_range&) {
		}

		if (arg_start > argc - 1) { // check if there is fen string input
			usage_error();
			return 1;
		}

		fen_string = "";
		for (int j = arg_start; j < argc; ++j) {
        	fen_string += argv[j];
        	if (j < argc - 1) {
            	fen_string += " "; // Add a space between arguments
        	}
    	}
	}

	// Main logic
	if (demo_mode) {
        if (!mute) {
            cout << "Running in demo mode with depth " << depth << ", engine vs engine." << endl;
        }
		board = Board(fen_string);
		Movelist moves;
		Color turn = fen_player_color(fen_string);

		string game_pgn = "";
		int round = 1;
		while (board.isGameOver().first == GameResultReason::NONE) {
			movegen::legalmoves(moves, board);
			Move picked_move;
			bool move_found = false;

			auto start = std::chrono::high_resolution_clock::now();
			int eval = turn == Color::WHITE ? -MAX_SCORE:MAX_SCORE;
			for (int i = 0; i < moves.size(); i++) {
				const auto move = moves[i];
				board.makeMove(move);
				int move_eval = minimax(depth, -MAX_SCORE, MAX_SCORE, 1 - turn);	
				board.unmakeMove(move);

				if ((turn == Color::WHITE && move_eval > eval) || (turn == Color::BLACK && move_eval < eval)) {
					eval = move_eval;
					picked_move = move;
					move_found = true;
				} 
			}
			auto end = std::chrono::high_resolution_clock::now();
			if (!move_found) picked_move = moves[0];
			chrono::duration<double> duration = end - start;
			if (!mute) {
				cout << "Execution time: " << duration.count() << " seconds\n";
				printf("nodes: %ld\n", minimax_searched);
				printf("eval: %d\n", eval);
			}
			string move_s = uci::moveToSan(board, picked_move);
			if (turn == Color::WHITE) {
				game_pgn = game_pgn + " " + to_string(round) + ". " + move_s;
			} else {
				game_pgn = game_pgn + " " + move_s;
				round++;
			}
			board.makeMove(picked_move);
			if (!mute) {
				cout << move_s << endl << endl;
			}
			turn = 1 - turn;
		} 

		cout << game_pgn << endl;
	} else { // engine mode
		Color turn = fen_player_color(fen_string);
		board = Board(fen_string);
		Movelist moves;
		movegen::legalmoves(moves, board);
		Move picked_move;
		bool move_found = false;

		auto start = std::chrono::high_resolution_clock::now();
		int eval = turn == Color::WHITE ? -MAX_SCORE:MAX_SCORE;
		for (int i = 0; i < moves.size(); i++) {
			const auto move = moves[i];
			board.makeMove(move);
			int move_eval = minimax(depth, -MAX_SCORE, MAX_SCORE, 1 - turn);	
			board.unmakeMove(move);

			if ((turn == Color::WHITE && move_eval > eval) || (turn == Color::BLACK && move_eval < eval)) {
				eval = move_eval;
				picked_move = move;
				move_found = true;
			} 
		}
		if (!move_found) picked_move = moves[0];
		auto end = std::chrono::high_resolution_clock::now();
		chrono::duration<double> duration = end - start;
		if (!mute) {
			cout << "Execution time: " << duration.count() << " seconds\n";
			printf("nodes: %ld\n", minimax_searched);
			printf("eval: %d\n", eval);
			cout << "Engine depth: " << depth << endl;
		}
		string move_s = uci::moveToSan(board, picked_move);
		// board.makeMove(picked_move);
		cout << move_s << endl;
	}
    return 0;
}
