import chess
import chess.pgn

from player import Player, PlayerType
import display_unicode

class Game:

    def __init__(self):
        self.ply1 = None
        self.ply2 = None

        self.board = chess.Board()


    def add_player(self, player_type):
        """
        Adds a player to the game.

        Returns: 
            bool: Whether the players is successfully added.
        """
        if self.ply1 == None:
            self.ply1 = Player(player_type)
            return True
        elif self.ply2 == None:
            self.ply2 = Player(player_type)
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
        return self.ply1 != None and self.ply2 != None \
                and self.ply1.color != None \
                and self.ply2.color != None \
                and self.ply1.color != self.ply2.color 

    def start(self):
        ############## Game Play Condition Check ##############
        if not self.ready():
            print("Failed to start the game due to incomplete game configuration.")
            return
        
        # Force an initialization of the board?
        board = chess.Board()

        print("Game started, initial position: ")
        display_unicode.print_board(board.turn, board)

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
        
        # Create a new game to store the moves
        game = chess.pgn.Game()

        # Set up the board from the game
        node = game.add_variation(board.move_stack[0])

        # Add moves to the PGN
        for move in board.move_stack[1:]:
            node = node.add_variation(move)

        # Print the PGN
        pgn_string = str(game)
        print()
        print(pgn_string)


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

    print("Select game mode:")
    print("A. Human vs Engine")
    print("B. Engine vs Human")
    print("C. Human vs Human")
    print("D. Engine vs Engine")

    choice = input("Enter your choice (A/B/C/D): ").strip().upper()

    game = Game()

    if choice == "A":
        game.add_player(PlayerType.HUMAN)
        game.add_player(PlayerType.ENGINE)
        game.assign_color(1, chess.WHITE)
        game.assign_color(2, chess.BLACK)
    elif choice == "B":
        game.add_player(PlayerType.ENGINE)
        game.add_player(PlayerType.HUMAN)
        game.assign_color(1, chess.WHITE)
        game.assign_color(2, chess.BLACK)
    elif choice == "C":
        game.add_player(PlayerType.HUMAN)
        game.add_player(PlayerType.HUMAN)
        game.assign_color(1, chess.WHITE)
        game.assign_color(2, chess.BLACK)
    elif choice == "D":
        game.add_player(PlayerType.ENGINE)
        game.add_player(PlayerType.ENGINE)
        game.assign_color(1, chess.WHITE)
        game.assign_color(2, chess.BLACK)
    else:
        print("Invalid choice! Please select A, B, C, or D.")

    game.start()
