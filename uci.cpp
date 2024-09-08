#include "chess-library/include/chess.hpp"
#include "constants.hpp"

#include <chrono>
#include <random>

using namespace chess;
using namespace std;

long minimax_searched = 0;
long quiescence_searched = 0;
int quiescence_depth = DEFAULT_DEPTH_Q;
int mm_depth = DEFAULT_DEPTH_MM;
float time_limit = 15.0;
auto start = std::chrono::high_resolution_clock::now();

Board board;

// InputParser source: https://stackoverflow.com/questions/865668/parsing-command-line-arguments-in-c
class InputParser{
    public:
        InputParser (int &argc, char **argv){
            for (int i=1; i < argc; ++i)
                this->tokens.push_back(std::string(argv[i]));
        }
        // @author iain
        const std::string& getCmdOption(const std::string &option) const{
            std::vector<std::string>::const_iterator itr;
            itr =  std::find(this->tokens.begin(), this->tokens.end(), option);
            if (itr != this->tokens.end() && ++itr != this->tokens.end()){
                return *itr;
            }
            static const std::string empty_string("");
            return empty_string;
        }
        // @author iain
        bool cmdOptionExists(const std::string &option) const{
            return std::find(this->tokens.begin(), this->tokens.end(), option)
                   != this->tokens.end();
        }
    private:
        std::vector <std::string> tokens;
};

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

bool appear_quiet() {
	if (board.inCheck())
		return false;

	Movelist moves;
	movegen::legalmoves(moves, board);
	for (int i = 0; i < moves.size(); i++) {
		const auto move = moves[i];
		board.makeMove(move);
		if (board.isCapture(move)) {
			int attacker_val = PIECE_VAL[(int)((board.at<Piece>(move.from())).type())];
			int victim_val = PIECE_VAL[(int)((board.at<Piece>(move.from())).type())];
			if (attacker_val < victim_val) {
				board.unmakeMove(move);
				return false;
			}
		}
		board.unmakeMove(move);
	}

	return true;
}

int quiescence_search (int q_depth, int alpha, int beta, Color color) {
	quiescence_searched++;
	
	// Force quit if time is up
	if (time_limit > 0) {
		auto end = std::chrono::high_resolution_clock::now();
		chrono::duration<double> duration = end - start;
		if (duration.count() > time_limit) {
			return color == Color::BLACK ? MAX_SCORE:-MAX_SCORE;
		}
	}

	if (q_depth == 0 || appear_quiet()) return evaluation(board);

	Movelist moves;
	movegen::legalmoves(moves, board);

	if (board.isGameOver().second == GameResult::DRAW) {
		return 0;
	}

	if (board.isGameOver().first == GameResultReason::CHECKMATE) {
		return board.sideToMove() == Color::BLACK ? MAX_SCORE:-MAX_SCORE;
	}

	if (color == Color::WHITE) {
		int max_eval = -MAX_SCORE;
		for (int i = 0; i < moves.size(); i++) {
			const auto move = moves[i];
			board.makeMove(move);
			int eval = quiescence_search(q_depth, alpha, beta, 1 - color);
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
			int eval = quiescence_search(q_depth, alpha, beta, 1 - color);
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

int minimax (int mm_depth, int alpha, int beta, Color color) {
	minimax_searched++;

	// Force quit if time is up
	if (time_limit > 0) {
		auto end = std::chrono::high_resolution_clock::now();
		chrono::duration<double> duration = end - start;
		if (duration.count() > time_limit) {
			return color == Color::BLACK ? MAX_SCORE:-MAX_SCORE;
		}
	}

	Movelist moves;
	movegen::legalmoves(moves, board);
	sort(moves.begin(), moves.end(), compare_moves);

	if (mm_depth == 0) {
		if (appear_quiet()) {
			return evaluation(board);
		} else {
			if (color == Color::WHITE) {
				int max_eval = -MAX_SCORE;
				for (int i = 0; i < moves.size(); i++) {
					const auto move = moves[i];
					board.makeMove(move);
					int eval = quiescence_search(DEFAULT_DEPTH_Q, alpha, beta, 1 - color);
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
					int eval = quiescence_search(quiescence_depth, alpha, beta, 1 - color);
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
	}

	if (board.isGameOver().second == GameResult::DRAW) {
		return 0;
	}

	if (board.isGameOver().first == GameResultReason::CHECKMATE) {
		return board.sideToMove() == Color::BLACK ? MAX_SCORE:-MAX_SCORE;
	}

	if (color == Color::WHITE) {
		int max_eval = -MAX_SCORE;
		for (int i = 0; i < moves.size(); i++) {
			const auto move = moves[i];
			board.makeMove(move);
			int eval = minimax(mm_depth - 1, alpha, beta, 1 - color);
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
			int eval = minimax(mm_depth - 1, alpha, beta, 1 - color);
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

void send_uci_info() {
    cout << "id name silkrow" << endl;
    cout << "id author Erkai Yu" << endl;
    cout << "uciok" << endl;
}

void send_ready_ok() {
    cout << "readyok" << endl;
}

void send_best_move(const Move& best_move) {
    cout << "bestmove " << uci::moveToUci(best_move) << endl;
}

void handle_uci_command() {
    string command;
    while (getline(cin, command)) {
        if (command == "uci") {
            send_uci_info();
        } else if (command == "isready") {
            send_ready_ok();
        } else if (command.rfind("position", 0) == 0) {
            // Extract FEN and moves from "position" command
            size_t fen_start = command.find("fen");
            if (fen_start != string::npos) {
                string fen_string = command.substr(fen_start + 4);
                board = Board(fen_string);
            }

            size_t moves_start = command.find("moves");
            if (moves_start != string::npos) {
                string moves_string = command.substr(moves_start + 6);
                istringstream move_stream(moves_string);
                string move_str;
                while (move_stream >> move_str) {
                    Move move = uci::uciToMove(board, move_str);
                    board.makeMove(move);
                }
            }
        } else if (command.rfind("go", 0) == 0) {
            // Run search algorithm
            Movelist moves;
            movegen::legalmoves(moves, board);

            Move best_move;
            int best_eval = -MAX_SCORE;

            for (int i = 0; i < moves.size(); i++) {
                const auto move = moves[i];
                board.makeMove(move);
                int eval = minimax(mm_depth, -MAX_SCORE, MAX_SCORE, board.sideToMove());
                board.unmakeMove(move);

                if (eval > best_eval) {
                    best_eval = eval;
                    best_move = move;
                }
            }

            send_best_move(best_move);
        } else if (command == "stop") {
            // Stop the search and return the best move found so far
        } else if (command == "quit") {
            break;
        }
    }
}

int main(int argc, char *argv[]) {
    handle_uci_command();
    return 0;
}