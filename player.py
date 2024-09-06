import random
import chess
import time
import subprocess

from enum import Enum, auto

class PlayerType(Enum):
    HUMAN = auto()
    ENGINE = auto()

class Player:
    """
    A class for the player in a game, it stores all the configuration for the 
    player, including settings for an engine, the color it is using, etc.
    """

    def __init__ (self, type):
        self.type = type
        self.color = None
        self.nodes_searched = 0
    
    def assign_color(self, color):
        """
        Assuming the given color input is valid.
        """
        self.color = color
    
    def get_move(self, board):
        """
        Returns:
            chess.Move: A move to be made given the board.
        """
        if self.type == PlayerType.HUMAN:
            while True:
                user_input = input("Enter your move in SAN format (e.g. Nf3): ")
                
                # Try to parse the input move
                try:
                    move = board.parse_san(user_input)  # Parse Standard Algebraic Notation (SAN)
                except ValueError:
                    print("Invalid move format. Please try again.")
                    continue
                
                # Check if the move is legal
                if move in board.legal_moves:
                    return move
                else:
                    print("Illegal move. Please try again.")

        elif self.type == PlayerType.ENGINE:
            depth = 7
            command = ['./silkfish', str(depth), board.fen()]

            result = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)

            output = result.stdout

            best_move = output.strip().split('\n')[-1]

            print(f"silkfish with depth {depth} makes move: {best_move}")
            move = board.parse_san(best_move)
            return move

        else:
            raise ValueError("Error: Player type invalid.")