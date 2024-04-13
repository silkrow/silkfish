import chess

class Evaluation:
    """
    A class to give an evaluation of a given position. Each 
    engine Player object has its own Evaluation object, 
    initialized with the configuration loaded by Configuration.
    """

    def __init__(self, conf) -> None:
        """
        Load the configuration of the engine, define some
        default constants
        """
        self.conf = conf

        # Piece values
        self.P = 1
        self.B = 3
        self.N = 3
        self.R = 5
        self.Q = 9

    def evaluate(self, board):
        """
        Returns:
            float: the evaluation of a position
        """
        
        return 0
    
