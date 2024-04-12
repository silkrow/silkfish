import chess
import random
from enum import Enum, auto
from configure import Configure

class PlayerType(Enum):
    HUMAN = auto()
    ENGINE = auto()

class PlayerColor(Enum):
    UNDECIDED = auto()
    WHITE = auto()
    BLACK = auto()

class Player:
    """
    A class for the player in a game, it stores all the configuration for the 
    player, including settings for an engine, the color it is using, etc.
    """

    def __init__ (self, type, conf_file=None):
        self.type = type
        self.color = PlayerColor.UNDECIDED
    
        # Load configuration    
        if self.type == PlayerType.ENGINE:
            if conf_file == None:
                raise ValueError("Error: Engine player missing configuration file.")
            self.conf = Configure(conf_file)
        elif self.type == PlayerType.HUMAN:
            self.conf = None
        else:
            raise ValueError("Error: Player type invalid.")
    
    def assign_color(self, color):
        """
        Assuming the given color input is valid.
        """
        self.color = color
    
    def get_move(self, board):
        legal_moves = list(board.legal_moves)
        return random.choice(legal_moves)
