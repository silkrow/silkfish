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

            if (time_left > 10*60*1000) mm_depth = 8; 
			else if (time_left > 4*60*1000) mm_depth = 7;
			else if (time_left > 1*60*1000) mm_depth = 6;
			else mm_depth = 5;

            // Run search algorithm
            Movelist moves;
            movegen::legalmoves(moves, board);
			LennyPOOL lenny_pool(MAX_THREAD);
			Move picked_move = findBestMove(board, mm_depth, MAX_THREAD);
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