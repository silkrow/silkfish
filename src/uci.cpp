#include "uci.hpp"

#include <thread>
#include <chrono>
#include "search.hpp"
#include "evaluation.hpp"
#include "constants.hpp"


using namespace std;
using namespace chess;
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

			if (time_left > 10*60*1000) mm_depth = 7;
			else if (time_left > 3*60*1000) mm_depth = 6;
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