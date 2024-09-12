#pragma once
#include "chess.hpp"
#include <iostream>
using namespace chess;

void send_uci_info();
void send_ready_ok();
void send_best_move(const Move& best_move);
void set_option(const std::string& name, const std::string& value);
void handle_uci_command();