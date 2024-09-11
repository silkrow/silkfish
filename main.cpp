#include "chess-library/include/chess.hpp"
#include "constants.hpp"

#include <chrono>

#include <thread>
#include <vector>
#include <semaphore>

using namespace chess;
using namespace std;

int quiescence_depth = DEFAULT_DEPTH_Q;
int mm_depth = DEFAULT_DEPTH_MM;
float time_limit = 15.0;
auto start = std::chrono::high_resolution_clock::now();
bool debug_mode = false; // Not being used.
int evals[1000];
std::counting_semaphore<MAX_THREAD> thread_limit(MAX_THREAD);

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

bool appear_quiet(Board board) {
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

int quiescence_search (int q_depth, int alpha, int beta, Color color, Board board) {
	// Force quit if time is up
	if (time_limit > 0) {
		auto end = std::chrono::high_resolution_clock::now();
		chrono::duration<double> duration = end - start;
		if (duration.count() > time_limit) {
			return color == Color::BLACK ? MAX_SCORE:-MAX_SCORE;
		}
	}

	if (q_depth == 0 || appear_quiet(board)) return evaluation(board);

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
			int eval = quiescence_search(q_depth, alpha, beta, 1 - color, board);
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
			int eval = quiescence_search(q_depth, alpha, beta, 1 - color, board);
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

int minimax (int mm_depth, int alpha, int beta, Color color, Board board) {
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
	sort(moves.begin(), moves.end(), [board] (Move a, Move b) {
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
	});

	if (mm_depth == 0) {
		if (appear_quiet(board)) {
			return evaluation(board);
		} else {
			if (color == Color::WHITE) {
				int max_eval = -MAX_SCORE;
				for (int i = 0; i < moves.size(); i++) {
					const auto move = moves[i];
					board.makeMove(move);
					int eval = quiescence_search(DEFAULT_DEPTH_Q, alpha, beta, 1 - color, board);
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
					int eval = quiescence_search(quiescence_depth, alpha, beta, 1 - color, board);
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
			int eval = minimax(mm_depth - 1, alpha, beta, 1 - color, board);
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
			int eval = minimax(mm_depth - 1, alpha, beta, 1 - color, board);
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
	std::cout << "Usage: ./silkrow <-flag1> <option1> <-flag2> <option2> ... <-fen> {fen_string}" << endl;
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

void handle_uci_command();

int main (int argc, char *argv[]) {
	if (argc == 1) {      // UCI mode if no argument passed in.
		handle_uci_command();
		return 0;
	}

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

	// Main logic
	if (demo_mode) {
        if (!mute) {
            std::cout << "Running in demo mode with mm_depth " << mm_depth << ", q_depth " << quiescence_depth << ", time_limit " << time_limit << ", engine vs engine." << endl;
        }

		Board board = Board(fen_string);
		Movelist moves;

		Color turn = fen_player_color(fen_string);

		string game_pgn = "";
		int round = 1;
		mutex mtx;
		while (board.isGameOver().first == GameResultReason::NONE) {
			movegen::legalmoves(moves, board);
			Move picked_move;
			start = std::chrono::high_resolution_clock::now();

			sort(moves.begin(), moves.end(), [board] (Move a, Move b) {
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
			});

			if (turn == Color::WHITE) fill(evals, evals + moves.size(), -MAX_SCORE);
			else fill(evals, evals + moves.size(), MAX_SCORE);

			std::vector<std::thread> children;
			mutex mtx;
			int alpha = -MAX_SCORE;
			int beta = MAX_SCORE;

			for (int i = 0; i < moves.size(); i++) {
				const auto move = moves[i];
				Board board_cp = board;
				thread_limit.acquire(); 
				children.emplace_back([board_cp = std::move(board_cp), move = std::move(move), turn, i, &alpha, &beta, &mtx]() mutable {
					board_cp.makeMove(move);
					evals[i] = minimax(mm_depth, alpha, beta, 1 - turn, board_cp);

					if (turn == Color::WHITE && evals[i] > alpha) {
						mtx.lock();
						alpha = evals[i];
						mtx.unlock();
					} else if (turn == Color::BLACK && evals[i] < beta){
						mtx.lock();
						beta = evals[i];
						mtx.unlock();
					}
					thread_limit.release();
				});
			}

			for (auto& child : children) {
        		if (child.joinable()) {
            		child.join();
				}
    		}

			int eval = evals[0];
			picked_move = moves[0];
			for (int i = 0; i < moves.size(); i++) {
				if ((turn == Color::WHITE && eval < evals[i]) ||
				(turn == Color::BLACK && eval > evals[i])) {
					eval = evals[i];
					picked_move = moves[i];
				}
			}
			auto end = std::chrono::high_resolution_clock::now();
			chrono::duration<double> duration = end - start;

			if (!mute) {
				std::cout << "Execution time: " << duration.count() << " seconds\n";
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
				std::cout << move_s << endl << endl;
			}
			turn = 1 - turn;
		} 

		std::cout << game_pgn << endl;
	} else { // engine mode
		Color turn = fen_player_color(fen_string);
		Board board = Board(fen_string);
		Movelist moves;
		movegen::legalmoves(moves, board);
		Move picked_move;
		start = std::chrono::high_resolution_clock::now();
		int evals[moves.size()];

		if (turn == Color::WHITE) fill(evals, evals + moves.size(), -MAX_SCORE);
		else fill(evals, evals + moves.size(), MAX_SCORE);

		for (int i = 0; i < moves.size(); i++) {
			const auto move = moves[i];
			board.makeMove(move);
			evals[i] = minimax(mm_depth, -MAX_SCORE, MAX_SCORE, 1 - turn, board);	
			board.unmakeMove(move);
		}

		int eval = evals[0];
		picked_move = moves[0];
		for (int i = 0; i < moves.size(); i++) {
			if ((turn == Color::WHITE && eval < evals[i]) ||
			(turn == Color::BLACK && eval > evals[i])) {
				eval = evals[i];
				picked_move = moves[i];
			}
		}
		auto end = std::chrono::high_resolution_clock::now();

		chrono::duration<double> duration = end - start;
		if (!mute) {
			std::cout << "Execution time: " << duration.count() << " seconds\n";
			printf("eval: %d\n", eval);
			std::cout << "Engine mm_depth: " << mm_depth  << ", q_depth: " << quiescence_depth << ", time_limit: " << time_limit << "s" << endl;
		}
		string move_s = uci::moveToSan(board, picked_move);
		// board.makeMove(picked_move);
		std::cout << move_s << endl;
	}
    return 0;
}

///////////////// UCI implementation //////////////////

int move_overhead = 0; // Default value

void send_uci_info() {
    std::cout << "id name silkrow" << endl;
    std::cout << "id author Erkai Yu" << endl;
    std::cout << "uciok" << endl;
}

void send_ready_ok() {
    std::cout << "readyok" << endl;
}

void send_best_move(const Move& best_move) {
    std::cout << "bestmove " << uci::moveToUci(best_move) << endl;
}

// Function to handle UCI options like "Move Overhead"
void set_option(const std::string& name, const std::string& value) {
    if (name == "Move Overhead") {
        move_overhead = stoi(value);  // Convert value to integer and set move_overhead
        std::cout << "info string Set Move Overhead to " << move_overhead << " ms" << endl;
    } else {
        // For unsupported options, ignore or log a message
        std::cout << "info string Unsupported option: " << name << endl;
    }
}

void handle_uci_command() {
	time_limit = 0;
    string command;
	Board board;
    while (getline(cin, command)) {
        if (command == "uci") {
            send_uci_info();
        } else if (command == "isready") {
            send_ready_ok();
        } else if (command.rfind("position", 0) == 0) {
            // Handle "position" command
            size_t startpos_start = command.find("startpos");
            size_t fen_start = command.find("fen");
            if (startpos_start != string::npos) {
                // If "startpos" is given, set up the board with the initial position
                board = Board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
            } else if (fen_start != string::npos) {
                // Handle FEN string if present
                string fen_string = command.substr(fen_start + 4);
                board = Board(fen_string);
            }

            // Handle moves after "position" command
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
			size_t btime_pos = command.find("btime");
            size_t wtime_pos = command.find("wtime");

			int time_left;

			if (board.sideToMove() == Color::WHITE && wtime_pos != string::npos) {
                time_left = stoi(command.substr(wtime_pos + 6));
            } else if (board.sideToMove() == Color::BLACK && btime_pos != string::npos) {
                time_left = stoi(command.substr(btime_pos + 6));
            }

			if (time_left > 10*1000) mm_depth = 7;
			else if (time_left > 6*1000) mm_depth = 6;
			else mm_depth = 5;

            // Run search algorithm
            Movelist moves;
            movegen::legalmoves(moves, board);

			Move picked_move;

			sort(moves.begin(), moves.end(), [board] (Move a, Move b) {
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
			});

			if (board.sideToMove() == Color::WHITE) fill(evals, evals + moves.size(), -MAX_SCORE);
			else fill(evals, evals + moves.size(), MAX_SCORE);

			std::vector<std::thread> children;
			mutex mtx;
			int alpha = -MAX_SCORE;
			int beta = MAX_SCORE;

			for (int i = 0; i < moves.size(); i++) {
				const auto move = moves[i];
				Board board_cp = board;
				thread_limit.acquire(); 
				children.emplace_back([board_cp = std::move(board_cp), move = std::move(move), i, &alpha, &beta, &mtx]() mutable {
					board_cp.makeMove(move);
					evals[i] = minimax(mm_depth, alpha, beta, board_cp.sideToMove(), board_cp);

					if (1 - board_cp.sideToMove() == 0 && evals[i] > alpha) {
						mtx.lock();
						alpha = evals[i];
						mtx.unlock();
					} else if (1 - board_cp.sideToMove() == 1 && evals[i] < beta){
						mtx.lock();
						beta = evals[i];
						mtx.unlock();
					}
					thread_limit.release();
				});
			}

			for (auto& child : children) {
        		if (child.joinable()) {
            		child.join();
				}
    		}

			int eval = evals[0];
			picked_move = moves[0];
			for (int i = 0; i < moves.size(); i++) {
				if ((board.sideToMove() == Color::WHITE && eval < evals[i]) ||
				(board.sideToMove() == Color::BLACK && eval > evals[i])) {
					eval = evals[i];
					picked_move = moves[i];
				}
			}
			auto end = std::chrono::high_resolution_clock::now();
			chrono::duration<double> duration = end - start;

            send_best_move(picked_move);
        } else if (command == "stop") {
            // Stop the search and return the best move found so far
        } else if (command == "quit") {
            break;
        } else if (command.rfind("setoption", 0) == 0) {
            // Handle "setoption" command for setting engine options
            size_t name_start = command.find("name");
            size_t value_start = command.find("value");
            if (name_start != string::npos && value_start != string::npos) {
                string option_name = command.substr(name_start + 5, value_start - name_start - 6);
                string option_value = command.substr(value_start + 6);
                set_option(option_name, option_value);
            }
        } else if (command == "debug on") {
            debug_mode = true;
        } else if (command == "debug off") {
            debug_mode = false;
        }
    }
}

////////////// End of UCI imlementation /////////////