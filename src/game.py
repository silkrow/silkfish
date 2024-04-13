import chess

from player import Player, PlayerType
import display_unicode

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
                and self.ply1.color != None \
                and self.ply2.color != None \
                and self.ply1.color != self.ply2.color 

    def start(self):
        """
        Starts the game!
        """
        ############## Game Play Condition Check ##############
        if not self.ready():
            print("Failed to start the game due to incomplete game configuration.")
            return
        
        # Force an initialization of the board?
        board = chess.Board()

        if self.ply1.color == chess.WHITE:
            white_player = self.ply1
            black_player = self.ply2
        else:
            white_player = self.ply2
            black_player = self.ply1

        while not board.is_game_over():
            if board.turn == chess.WHITE:
                move = white_player.get_move(board)
            else:
                move = black_player.get_move(board)

            board.push(move)

            # Display the board at each move
            display_unicode.print_board(board.turn, board)

        # Print the result
        if board.is_checkmate():
            winner = "White" if board.turn == chess.BLACK else "Black"
            print(f"Checkmate! {winner} wins!")
        elif board.is_stalemate():
            print("Stalemate! The game is drawn.")
        elif board.is_insufficient_material():
            print("Insufficient material! The game is drawn.")
        elif board.is_seventyfive_moves():
            print("Seventy-five moves rule! The game is drawn.")
        elif board.is_fivefold_repetition():
            print("Fivefold repetition! The game is drawn.")
        else:
            print("Unknown result.")


if __name__ == "__main__":
    game = Game()
    game.add_player(PlayerType.ENGINE, "../config/config.json")
    game.add_player(PlayerType.HUMAN, "../config/config.json")

    game.assign_color(1, chess.BLACK)
    game.assign_color(2, chess.WHITE)

    print("game ready? ", game.ready())

    game.start()
