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

void usage_error() {
	cout << "Usage: ./silkrow <-flag1> <option1> <-flag2> <option2> ... <-fen> {fen_string}" << endl;
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
	string fen_string = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"; 
	// fen_string = "4k3/8/6K1/8/3Q4/8/8/8 w - - 0 1"; // for testing purpose
	bool demo_mode = false;
	bool mute = false;

	InputParser input(argc, argv);

	if (input.cmdOptionExists("-m")) {
		mute = true;
	} 

	if (input.cmdOptionExists("-demo")) {
		demo_mode = true;
	} 

	const string &mm_depth_s = input.getCmdOption("-md");
	if (!mm_depth_s.empty()) {
		try {
			size_t pos;
			mm_depth = stoi(mm_depth_s, &pos);
			if (pos != mm_depth_s.length()) {
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

	const string &q_depth_s = input.getCmdOption("-qd");
	if (!q_depth_s.empty()) {
		try {
			size_t pos;
			quiescence_depth = stoi(q_depth_s, &pos);
			if (pos != q_depth_s.length()) {
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

	const string &time_limit_s = input.getCmdOption("-t");
	if (!time_limit_s.empty()) {
		try {
			size_t pos;
			time_limit = (double) stoi(time_limit_s, &pos);
			if (pos != time_limit_s.length()) {
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

	if (!demo_mode) {
		int arg_start = 0;
		for (int i = 0; i < argc - 1; i++) {
			if (string(argv[i]) == "-fen") {
				arg_start = i + 1;
				break;
			}
		}

		fen_string = "";
		for (int j = arg_start; j < argc; ++j) {
        	fen_string += argv[j];
        	if (j < argc - 1) {
            	fen_string += " "; // Add a space between arguments
        	}
    	}
	}

	random_device rd;
	mt19937 gen(rd());
	uniform_int_distribution<> distr(0, 9);

	// Main logic
	if (demo_mode) {
        if (!mute) {
            cout << "Running in demo mode with mm_depth " << mm_depth << ", q_depth " << quiescence_depth << ", time_limit " << time_limit << ", engine vs engine." << endl;
        }
		board = Board(fen_string);
		Movelist moves;
		Color turn = fen_player_color(fen_string);

		string game_pgn = "";
		int round = 1;
		while (board.isGameOver().first == GameResultReason::NONE) {
			movegen::legalmoves(moves, board);
			Move picked_move;
			minimax_searched = 0;
			quiescence_searched = 0;
			start = std::chrono::high_resolution_clock::now();
			int eval = turn == Color::WHITE ? -MAX_SCORE:MAX_SCORE; // Supposed to be assigned later
			int eval1 = turn == Color::WHITE ? -MAX_SCORE:MAX_SCORE;
			int eval2 = turn == Color::WHITE ? -MAX_SCORE:MAX_SCORE;
			int eval3 = turn == Color::WHITE ? -MAX_SCORE:MAX_SCORE;
			int can_1 = 0, can_2 = 0, can_3 = 0;
			for (int i = 0; i < moves.size(); i++) {
				const auto move = moves[i];
				board.makeMove(move);
				int move_eval = minimax(mm_depth, -MAX_SCORE, MAX_SCORE, 1 - turn);	
				board.unmakeMove(move);

				if ((turn == Color::WHITE && move_eval > eval1) || (turn == Color::BLACK && move_eval < eval1)) {
					eval1 = move_eval;
					can_1 = i;
				} else if ((turn == Color::WHITE && move_eval > eval2) || (turn == Color::BLACK && move_eval < eval2)) {
					eval2 = move_eval;
					can_2 = i;
				} else if ((turn == Color::WHITE && move_eval > eval3) || (turn == Color::BLACK && move_eval < eval3)) {
					eval3 = move_eval;
					can_3 = i;	
				}
			}
			if (abs(eval1 - eval2) > RAND_MOVE_THRE) {
				picked_move = moves[can_1];
				eval = eval1;
			} else if (abs(eval1 - eval3) > RAND_MOVE_THRE) { // 6:4
				int rand_num = distr(gen);
				if (rand_num <= 5) {
					picked_move = moves[can_1];
					eval = eval1;
				} else {
					picked_move = moves[can_2];
					eval = eval2;	
				}
			} else { // 4:4:2
				int rand_num = distr(gen);
				if (rand_num <= 3) {
					picked_move = moves[can_1];
					eval = eval1;
				} else if (rand_num <= 7){
					picked_move = moves[can_2];
					eval = eval2;	
				} else {
					picked_move = moves[can_3];
					eval = eval3;
				}
			}

			auto end = std::chrono::high_resolution_clock::now();
			chrono::duration<double> duration = end - start;
			if (!mute) {
				cout << "Execution time: " << duration.count() << " seconds\n";
				printf("minimax nodes: %ld\n", minimax_searched);
				printf("quiescence nodes: %ld\n", quiescence_searched);
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
		minimax_searched = 0;
		quiescence_searched = 0;
		start = std::chrono::high_resolution_clock::now();
		int eval = turn == Color::WHITE ? -MAX_SCORE:MAX_SCORE; // Supposed to be assigned later
		int eval1 = turn == Color::WHITE ? -MAX_SCORE:MAX_SCORE;
		int eval2 = turn == Color::WHITE ? -MAX_SCORE:MAX_SCORE;
		int eval3 = turn == Color::WHITE ? -MAX_SCORE:MAX_SCORE;
		int can_1 = 0, can_2 = 0, can_3 = 0;
		for (int i = 0; i < moves.size(); i++) {
			const auto move = moves[i];
			board.makeMove(move);
			int move_eval = minimax(mm_depth, -MAX_SCORE, MAX_SCORE, 1 - turn);	
			board.unmakeMove(move);

			if ((turn == Color::WHITE && move_eval > eval1) || (turn == Color::BLACK && move_eval < eval1)) {
				eval1 = move_eval;
				can_1 = i;
			} else if ((turn == Color::WHITE && move_eval > eval2) || (turn == Color::BLACK && move_eval < eval2)) {
				eval2 = move_eval;
				can_2 = i;
			} else if ((turn == Color::WHITE && move_eval > eval3) || (turn == Color::BLACK && move_eval < eval3)) {
				eval3 = move_eval;
				can_3 = i;	
			}
		}
		if (abs(eval1 - eval2) > RAND_MOVE_THRE) {
			picked_move = moves[can_1];
			eval = eval1;
		} else if (abs(eval1 - eval3) > RAND_MOVE_THRE) { // 6:4
			int rand_num = distr(gen);
			if (rand_num <= 5) {
				picked_move = moves[can_1];
				eval = eval1;
			} else {
				picked_move = moves[can_2];
				eval = eval2;	
			}
		} else { // 4:4:2
			int rand_num = distr(gen);
			if (rand_num <= 3) {
				picked_move = moves[can_1];
				eval = eval1;
			} else if (rand_num <= 7){
				picked_move = moves[can_2];
				eval = eval2;	
			} else {
				picked_move = moves[can_3];
				eval = eval3;
			}
		}

		auto end = std::chrono::high_resolution_clock::now();

		chrono::duration<double> duration = end - start;
		if (!mute) {
			cout << "Execution time: " << duration.count() << " seconds\n";
			printf("minimax nodes: %ld\n", minimax_searched);
			printf("quiescence nodes: %ld\n", quiescence_searched);
			printf("eval: %d\n", eval);
			cout << "Engine mm_depth: " << mm_depth  << ", q_depth: " << quiescence_depth << ", time_limit: " << time_limit << "s" << endl;
		}
		string move_s = uci::moveToSan(board, picked_move);
		// board.makeMove(picked_move);
		cout << move_s << endl;
	}
    return 0;
}
