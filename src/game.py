import chess

from configure import Configure
from player import Player, PlayerColor, PlayerType

class Game:
    """
    A class to hold all the needed information of a game, including 
    engine configuration, board state, etc.
    """

    def __init__(self):
        """
        Initialize a chess game. 
        """

        self.ply1 = None
        self.ply2 = None

        self.board = chess.Board()


    def add_player(self, player_type, conf_file=None):
        """
        Adds a player to the game.

        Returns: 
            bool: Whether the players is successfully added.
        """
        if self.ply1 == None:
            self.ply1 = Player(player_type, conf_file)
            return True
        elif self.ply2 == None:
            self.ply2 = Player(player_type, conf_file)
            return True
        else:
            return False
        
    def assign_color(self, playernum, color):
        """
        Assigns the player with the color, 
            1 for ply1
            2 for ply2

        Returns:
            bool: Whether the assignment succeeded.
        """

        if playernum == 1 and self.ply1 != None:
            self.ply1.assign_color(color)
            return True
        elif playernum == 2 and self.ply2 != None:
            self.ply2.assign_color(color)
            return True
        else:
            return False

    def ready(self):
        """
        Returns:
            bool: Whether a game is ready to start.
        """

        return self.ply1 != None and self.ply2 != None \
                and self.ply1.color != PlayerColor.UNDECIDED \
                and self.ply2.color != PlayerColor.UNDECIDED \
                and self.ply1.color != self.ply2.color 



if __name__ == "__main__":
    game = Game()
    if game.add_player(PlayerType.ENGINE, "../config/config.json"):
        print("Add engine success")
    if game.add_player(PlayerType.ENGINE, "../config/config.json"):
        print("Add engine success")

    game.assign_color(1, PlayerColor.BLACK)
    game.assign_color(2, PlayerColor.WHITE)

    print("game ready? ", game.ready())


