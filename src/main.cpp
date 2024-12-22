#include "chess.hpp"
#include "constants.hpp"
#include "input_parser.hpp"
#include "evaluation.hpp"
#include "search.hpp"
#include "uci.hpp"

#include <chrono>

#include <thread>
#include <vector>
#include <semaphore>

using namespace chess;
using namespace std;

int quiescence_depth = DEFAULT_DEPTH_Q;
int mm_depth = DEFAULT_DEPTH_MM;

void usage_error() {
	std::cout << "Usage: ./silkrow <-flag1> <option1> <-flag2> <option2> ... <-fen> {fen_string}" << endl;
	return;
}

int main (int argc, char *argv[]) {

	if (argc == 1) {      // UCI mode if no argument passed in.
		handle_uci_command();
		return 0;
	}

	string fen_string = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"; 
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
            std::cout << "Running in demo mode with mm_depth " << mm_depth << ", q_depth " << quiescence_depth << ", engine vs engine." << endl;
        }

		Board board = Board(fen_string);
		Movelist moves;

		Color turn = board.sideToMove();

		string game_pgn = "";
		int round = 1;
		

		while (board.isGameOver().first == GameResultReason::NONE) {
			movegen::legalmoves(moves, board);
			Move picked_move;
			auto start = std::chrono::high_resolution_clock::now();
			picked_move = findBestMove(board, mm_depth);
			auto end = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double> duration = end - start;
			if (!mute) {
				std::cout << "Execution time: " << duration.count() << " seconds\n";
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
		// a good testing fen for mate in 5, should use md = 9, correct move is h4: 2N5/5p2/6pp/7k/4N3/p3P1KP/1p6/3b4 w - - 0 1
		// a good testing fen for mate in 4, should use md = 8, correct move is Bxg2: Q7/p1p1q1pk/3p2rp/4n3/3bP3/7b/PP3PPK/R1B2R2 b - - 0 1
		Board board = Board(fen_string);
		Color turn = board.sideToMove();
		Move picked_move;

		if (!mute) {
			std::cout << "engine thinking ..." << endl;
		}

		auto start = std::chrono::high_resolution_clock::now();
		picked_move = findBestMove(board, mm_depth);
		auto end = std::chrono::high_resolution_clock::now();

		chrono::duration<double> duration = end - start;
		if (!mute) {
			std::cout << "Execution time: " << duration.count() << " seconds\n";
			std::cout << "Engine mm_depth: " << mm_depth  << ", q_depth: " << quiescence_depth << endl;
		}
		string move_s = uci::moveToSan(board, picked_move);
		std::cout << move_s << endl;
	}
    return 0;
}
