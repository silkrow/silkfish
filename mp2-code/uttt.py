from time import sleep
from math import inf
from random import randint
import time


class ultimateTicTacToe:
    def __init__(self):
        """
        Initialization of the game.
        """
        self.board = [['_', '_', '_', '_', '_', '_', '_', '_', '_'],
                      ['_', '_', '_', '_', '_', '_', '_', '_', '_'],
                      ['_', '_', '_', '_', '_', '_', '_', '_', '_'],
                      ['_', '_', '_', '_', '_', '_', '_', '_', '_'],
                      ['_', '_', '_', '_', '_', '_', '_', '_', '_'],
                      ['_', '_', '_', '_', '_', '_', '_', '_', '_'],
                      ['_', '_', '_', '_', '_', '_', '_', '_', '_'],
                      ['_', '_', '_', '_', '_', '_', '_', '_', '_'],
                      ['_', '_', '_', '_', '_', '_', '_', '_', '_']]
        self.maxPlayer = 'X'
        self.minPlayer = 'O'
        self.maxDepth = 3
        # The start indexes of each local board
        self.globalIdx = [(0, 0), (0, 3), (0, 6), (3, 0),
                          (3, 3), (3, 6), (6, 0), (6, 3), (6, 6)]

        # Start local board index for reflex agent playing
        self.startBoardIdx = 4
        #self.startBoardIdx = randint(0, 8)

        # utility value for reflex offensive and reflex defensive agents
        self.winnerMaxUtility = 10000
        self.twoInARowMaxUtility = 500
        self.preventThreeInARowMaxUtility = 100
        self.cornerMaxUtility = 30

        self.winnerMinUtility = -10000
        self.twoInARowMinUtility = -100
        self.preventThreeInARowMinUtility = -500
        self.cornerMinUtility = -30

        self.expandedNodes = 0
        self.currPlayer = True

    def getNextBoardIdx(self, i, j):
        return (i % 3) * 3 + j % 3

    def printGameBoard(self):
        """
        This function prints the current game board.
        """
        print('\n'.join([' '.join([str(cell) for cell in row])
                         for row in self.board[:3]])+'\n')
        print('\n'.join([' '.join([str(cell) for cell in row])
                         for row in self.board[3:6]])+'\n')
        print('\n'.join([' '.join([str(cell) for cell in row])
                         for row in self.board[6:9]])+'\n')

    def evaluatePredifined(self, isMax):
        winner = self.checkWinner()
        if winner == 1 and isMax:
            return 10000
        elif winner == -1 and not isMax:
            return -10000

        score = 0
        counter_500 = 0
        counter_100 = 0

        # Helper function to check patterns
        def check_pattern(player, opponent, *positions):
            nonlocal counter_500, counter_100
            cur_board = [self.board[y][x] for y, x in positions]
            if cur_board.count(player) == 2 and cur_board.count('_') == 1:
                counter_500 += 1
            if cur_board.count(opponent) == 2 and cur_board.count(player) == 1:
                counter_100 += 1

        # Check all patterns
        for row, col in self.globalIdx:
            cur_player = self.maxPlayer if isMax else self.minPlayer
            opponent_player = self.minPlayer if isMax else self.maxPlayer

            # Check rows and columns
            for i in range(3):
                check_pattern(cur_player, opponent_player, (row, col+i), (row+1, col+i), (row+2, col+i))
                check_pattern(cur_player, opponent_player, (row+i, col), (row+i, col+1), (row+i, col+2))

            # Check diagonals
            check_pattern(cur_player, opponent_player, (row, col), (row+1, col+1), (row+2, col+2))
            check_pattern(cur_player, opponent_player, (row, col+2), (row+1, col+1), (row+2, col))

        # Calculate score
        if isMax:
            score += 500 * counter_500 + 100 * counter_100
        else:
            score -= 100 * counter_500 + 500 * counter_100

        # Additional scoring logic
        if score == 0:
            for row, col in self.globalIdx:
                for y, x in [(row, col), (row+2, col), (row, col+2), (row+2, col+2)]:
                    if self.board[y][x] == self.maxPlayer and isMax:
                        score += 30
                    elif self.board[y][x] == self.minPlayer and not isMax:
                        score -= 30

        return score

    def evaluateDesigned(self, isMax):
        """
        This function implements the evaluation function for ultimate tic tac toe for your own agent.
        input args:
        isMax(bool): boolean variable indicates whether it's maxPlayer or minPlayer.
                     True for maxPlayer, False for minPlayer
        output:
        score(float): estimated utility score for maxPlayer or minPlayer
        """
        # YOUR CODE HERE
        winner = self.checkWinner()
        if winner == 1:
            return 10000
        elif winner == -1:
            return -10000

        score = 0
        counter_500 = 0
        counter_100 = 0

        # Helper function to check patterns
        def check_pattern(player, opponent, *positions):
            nonlocal counter_500, counter_100
            cur_board = [self.board[y][x] for y, x in positions]
            if cur_board.count(player) == 2 and cur_board.count('_') == 1:
                counter_500 += 1
            if cur_board.count(opponent) == 2 and cur_board.count(player) == 1:
                counter_100 += 1

        # Check all patterns
        for row, col in self.globalIdx:
            cur_player = self.maxPlayer if isMax else self.minPlayer
            opponent_player = self.minPlayer if isMax else self.maxPlayer

            # Check rows and columns
            for i in range(3):
                check_pattern(cur_player, opponent_player, (row, col+i), (row+1, col+i), (row+2, col+i))
                check_pattern(cur_player, opponent_player, (row+i, col), (row+i, col+1), (row+i, col+2))

            # Check diagonals
            check_pattern(cur_player, opponent_player, (row, col), (row+1, col+1), (row+2, col+2))
            check_pattern(cur_player, opponent_player, (row, col+2), (row+1, col+1), (row+2, col))

        # Calculate score
        if isMax:
            score += 500 * counter_500 + 100 * counter_100
        else:
            score -= 100 * counter_500 + 500 * counter_100

        # Additional scoring logic
        for row, col in self.globalIdx:
            for y, x in [(row, col), (row+2, col), (row, col+2), (row+2, col+2)]:
                if self.board[y][x] == self.maxPlayer:
                    score += 30
                elif self.board[y][x] == self.minPlayer:
                    score -= 30

        return score

    def checkMovesLeft(self):
        """
        This function checks whether any legal move remains on the board.
        output:
        movesLeft(bool): boolean variable indicates whether any legal move remains
                        on the board.
        """
        # YOUR CODE HERE
        return any('_' in row for row in self.board)

    def checkWinner(self):
        """
        This function checks whether there is a winner on the board.
        output:
        winner(int): Return 0 if there is no winner.
                    Return 1 if maxPlayer is the winner.
                    Return -1 if minPlayer is the winner.
        """
        # Define a helper function to check for a win
        def check_line(start, step):
            symbol = self.board[start[0]][start[1]]
            if symbol != '_' and all(self.board[start[0] + i * step[0]][start[1] + i * step[1]] == symbol for i in range(1, 3)):
                return symbol
            return None

        # Check all rows, columns, and diagonals
        for i in range(9):
            row, col = self.globalIdx[i]
            # Check rows and columns
            for j in range(3):
                result = check_line((row, col + j), (1, 0)) or check_line((row + j, col), (0, 1))
                if result:
                    return 1 if result == 'X' else -1
            # Check diagonals
            result = check_line((row, col), (1, 1)) or check_line((row, col + 2), (1, -1))
            if result:
                return 1 if result == 'X' else -1

        return 0

    def alphabeta(self, depth, currBoardIdx, alpha, beta, isMax):
        """
        This function implements alpha-beta algorithm for ultimate tic-tac-toe game.
        input args:
        depth(int): current depth level
        currBoardIdx(int): current local board index
        alpha(float): alpha value
        beta(float): beta value
        isMax(bool): boolean variable indicates whether it's maxPlayer or minPlayer.
                    True for maxPlayer, False for minPlayer
        output:
        best_value(float): the best_value that current player may have
        """
        # Terminal condition check
        if (depth == self.maxDepth) or (not self.checkMovesLeft()) or (self.checkWinner() != 0):
            self.expandedNodes += 1
            return self.evaluatePredifined(self.currPlayer)

        # Initialize best_value
        best_value = -inf if isMax else inf
        player = self.maxPlayer if isMax else self.minPlayer

        # Iterate through all cells
        y, x = self.globalIdx[currBoardIdx]
        for j in range(3):
            for i in range(3):
                if self.board[y+j][x+i] == '_':
                    # Make a move
                    self.board[y+j][x+i] = player
                    # Recurse to next depth
                    cur_value = self.alphabeta(depth+1, self.getNextBoardIdx(y+j, x+i), alpha, beta, not isMax)
                    # Undo the move
                    self.board[y+j][x+i] = '_'
                    # Update best_value, alpha, and beta
                    if isMax:
                        best_value = max(best_value, cur_value)
                        alpha = max(alpha, best_value)
                    else:
                        best_value = min(best_value, cur_value)
                        beta = min(beta, best_value)
                    # Alpha-beta pruning
                    if beta <= alpha:
                        break
            if beta <= alpha:
                break

        return best_value

    def minimax(self, depth, currBoardIdx, isMax):
        """
        This function implements minimax algorithm for ultimate tic-tac-toe game.
        input args:
        depth(int): current depth level
        currBoardIdx(int): current local board index
        isMax(bool): boolean variable indicates whether it's maxPlayer or minPlayer.
                    True for maxPlayer, False for minPlayer
        output:
        bestValue(float): the bestValue that current player may have
        """
        # Terminal condition check
        if (depth == self.maxDepth) or (not self.checkMovesLeft()) or (self.checkWinner() != 0):
            self.expandedNodes += 1
            return self.evaluatePredifined(self.currPlayer)

        # Initialize best_value
        best_value = -inf if isMax else inf
        player = self.maxPlayer if isMax else self.minPlayer

        # Iterate through all cells
        y, x = self.globalIdx[currBoardIdx]
        for j in range(3):
            for i in range(3):
                if self.board[y+j][x+i] == '_':
                    # Make a move
                    self.board[y+j][x+i] = player
                    # Recurse to next depth
                    cur_value = self.minimax(depth+1, self.getNextBoardIdx(y+j, x+i), not isMax)
                    # Undo the move
                    self.board[y+j][x+i] = '_'
                    # Update best_value
                    best_value = max(best_value, cur_value) if isMax else min(best_value, cur_value)

        return best_value

    def playGamePredifinedAgent(self, maxFirst, isMinimaxOffensive, isMinimaxDefensive):
        """
        This function implements the processes of the game of predifined offensive agent vs defensive agent.
        input args:
        maxFirst(bool): boolean variable indicates whether maxPlayer or minPlayer plays first.
                        True for maxPlayer plays first, and False for minPlayer plays first.
        isMinimaxOffensive(bool):boolean variable indicates whether it's using minimax or alpha-beta pruning algorithm for offensive agent.
                        True is minimax and False is alpha-beta.
        isMinimaxOffensive(bool):boolean variable indicates whether it's using minimax or alpha-beta pruning algorithm for defensive agent.
                        True is minimax and False is alpha-beta.
        output:
        bestMove(list of tuple): list of bestMove coordinates at each step
        bestValue(list of float): list of bestValue at each move
        expandedNodes(list of int): list of expanded nodes at each move
        gameBoards(list of 2d lists): list of game board positions at each move
        winner(int): 1 for maxPlayer is the winner, -1 for minPlayer is the winner, and 0 for tie.
        """
        # YOUR CODE HERE
        cur_player = maxFirst
        cur_board = self.startBoardIdx
        self.expandedNodes = 0
        bestMove = []
        bestValue = []
        gameBoards = []
        expandedNodes = []

        alpha = float('-inf')
        beta = float('inf')
        while self.checkMovesLeft() and self.checkWinner() == 0:
            self.currPlayer = cur_player
            y, x = self.globalIdx[cur_board]
            best_coord = (-1, -1)
            best_value = float('-inf') if cur_player else float('inf')
            for j in range(3):
                for i in range(3):
                    if self.board[y+j][x+i] == '_':
                        self.board[y+j][x+i] = self.maxPlayer if cur_player else self.minPlayer
                        cur_board = self.getNextBoardIdx(y+j, x+i)
                        cur_value = self.minimax(1, cur_board, not cur_player) if (cur_player and isMinimaxOffensive) or (not cur_player and isMinimaxDefensive) else self.alphabeta(1, cur_board, alpha, beta, not cur_player)
                        self.board[y+j][x+i] = '_'
                        if (cur_player and cur_value > best_value) or (not cur_player and cur_value < best_value):
                            best_coord = (y+j, x+i)
                            best_value = cur_value
            self.board[best_coord[0]][best_coord[1]] = self.maxPlayer if cur_player else self.minPlayer
            cur_board = self.getNextBoardIdx(best_coord[0], best_coord[1])
            bestMove.append(best_coord)
            bestValue.append(best_value)
            gameBoards.append(self.board)
            expandedNodes.append(self.expandedNodes)
            self.printGameBoard()
            cur_player = not cur_player
        print("# Expanded nodes: ", sum(expandedNodes))
        winner = self.checkWinner()
        return gameBoards, bestMove, expandedNodes, bestValue, winner

    def my_minimax(self, depth, currBoardIdx, isMax):
        """
        This function implements minimax algorithm for ultimate tic-tac-toe game.
        input args:
        depth(int): current depth level
        currBoardIdx(int): current local board index
        isMax(bool): boolean variable indicates whether it's maxPlayer or minPlayer.
                    True for maxPlayer, False for minPlayer
        output:
        bestValue(float): the bestValue that current player may have
        """
        # if (depth == self.maxDepth) or (not self.checkMovesLeft()) or (self.checkWinner() != 0):
        #     self.expandedNodes += 1
        #     return self.evaluateDesigned(self.currPlayer)

        if self.checkWinner() != 0:
            if self.checkWinner() == 1:
                return 10000
            else: 
                return -10000

        if (depth == self.maxDepth) or (not self.checkMovesLeft()):
            self.expandedNodes += 1
            return self.evaluateDesigned(self.currPlayer)

        best_value = float('-inf') if isMax else float('inf')
        y, x = self.globalIdx[currBoardIdx]
        for j in range(3):
            for i in range(3):
                if self.board[y+j][x+i] == '_':
                    self.board[y+j][x+i] = self.maxPlayer if isMax else self.minPlayer
                    cur_value = self.my_minimax(depth+1, self.getNextBoardIdx(y+j, x+i), not isMax)
                    self.board[y+j][x+i] = '_'
                    best_value = max(best_value, cur_value) if isMax else min(best_value, cur_value)
        return best_value

    def playGameYourAgent(self, maxFirst):
        """
        This function implements the processes of the game of your own agent vs predifined offensive agent.
        input args:
        output:
        best_coord(list of tuple): list of best_coord coordinates at each step
        gameBoards(list of 2d lists): list of game board positions at each move
        winner(int): 1 for maxPlayer is the winner, -1 for minPlayer is the winner, and 0 for tie.
        """
        cur_player = maxFirst  # true max first, false min first
        cur_board = self.startBoardIdx
        self.expandedNodes = 0
        bestMove = []
        bestValue = []
        gameBoards = []
        expandedNodes = []
        alpha = float('-inf')
        beta = float('inf')

        while self.checkMovesLeft() and self.checkWinner() == 0:
            self.currPlayer = cur_player
            y, x = self.globalIdx[cur_board]
            best_coord = (-1, -1)
            best_value = float('-inf') if cur_player else float('inf')
            for j in range(3):
                for i in range(3):
                    if self.board[y+j][x+i] == '_':
                        self.board[y+j][x+i] = self.maxPlayer if cur_player else self.minPlayer
                        cur_board = self.getNextBoardIdx(y+j, x+i)
                        cur_value = self.alphabeta(1, cur_board, alpha, beta, not cur_player) if cur_player else self.my_minimax(1, cur_board, not cur_player)
                        self.board[y+j][x+i] = '_'
                        if (cur_player and cur_value > best_value) or (not cur_player and cur_value < best_value):
                            best_coord = (y+j, x+i)
                            best_value = cur_value
            self.board[best_coord[0]][best_coord[1]] = self.maxPlayer if cur_player else self.minPlayer
            cur_board = self.getNextBoardIdx(best_coord[0], best_coord[1])
            bestMove.append(best_coord)
            bestValue.append(best_value)
            gameBoards.append(self.board)
            expandedNodes.append(self.expandedNodes)
            self.printGameBoard()
            cur_player = not cur_player
        print("# Expanded nodes: ", sum(expandedNodes))
        winner = self.checkWinner()
        return gameBoards, bestMove, expandedNodes, bestValue, winner

    def playGameHuman(self):
        """
        This function implements the processes of the game of your own agent vs a human.
        output:
        best_coord(list of tuple): list of best_coord coordinates at each step
        gameBoards(list of 2d lists): list of game board positions at each move
        winner(int): 1 for maxPlayer is the winner, -1 for minPlayer is the winner, and 0 for tie.
        """
        cur_player = True  # true max first, false min first
        cur_board = self.startBoardIdx
        self.expandedNodes = 0
        bestMove = []
        bestValue = []
        gameBoards = []
        expandedNodes = []

        while self.checkMovesLeft() and self.checkWinner() == 0:
            self.currPlayer = cur_player
            y, x = self.globalIdx[cur_board]
            if cur_player:
                best_coord = (-1, -1)
                best_value = -float('inf')
                for j in range(3):
                    for i in range(3):
                        if self.board[y+j][x+i] == '_':
                            self.board[y+j][x+i] = self.maxPlayer
                            cur_board = self.getNextBoardIdx(y+j, x+i)
                            cur_value = self.my_minimax(1, cur_board, not cur_player)
                            self.board[y+j][x+i] = '_'
                            if cur_value > best_value:
                                best_coord = (y+j, x+i)
                                best_value = cur_value
                self.board[best_coord[0]][best_coord[1]] = self.maxPlayer
                cur_board = self.getNextBoardIdx(best_coord[0], best_coord[1])
                bestMove.append(best_coord)
                bestValue.append(best_value)
            else:
                print("put in board:", cur_board)
                x = input('x:')
                y = input('y:')
                put_y = self.globalIdx[cur_board][0] + int(y)  # Convert y to an integer
                put_x = self.globalIdx[cur_board][1] + int(x)  # Convert x to an integer
                self.board[put_y][put_x] = self.minPlayer if not self.currPlayer else self.maxPlayer
                cur_board = self.getNextBoardIdx(put_y, put_x)

            gameBoards.append(self.board.copy())
            expandedNodes.append(self.expandedNodes)
            self.printGameBoard()
            cur_player = not cur_player

        winner = self.checkWinner()
        return gameBoards, bestMove, expandedNodes, bestValue, winner

    def ec_checkWinner(self):
        counter_x = counter_y = 0

        # Helper function to check for win condition
        def check_line(start_row, start_col, delta_row, delta_col):
            nonlocal counter_x, counter_y
            symbol = self.board[start_row][start_col]
            if symbol != '_':
                if all(self.board[start_row + i * delta_row][start_col + i * delta_col] == symbol for i in range(3)):
                    if symbol == 'X':
                        counter_x += 1
                    elif symbol == 'O':
                        counter_y += 1
                    return (start_row, start_col)
            return None

        # Check rows, columns and diagonals
        for row, col in self.globalIdx:
            check_line(row, col, 1, 0)  # Check vertical
            check_line(row, col, 0, 1)  # Check horizontal
            check_line(row, col, 1, 1)  # Check diagonal
            check_line(row, col + 2, 1, -1)  # Check anti-diagonal

        # Function to mark 'A' in the empty cells of the winning grid
        def mark_empty_cells(coor):
            if coor:
                idx = (coor[0] // 3) * 3 + (coor[1] // 3)
                cd = self.globalIdx[idx]
                for j in range(3):
                    for i in range(3):
                        if self.board[cd[0] + j][cd[1] + i] == '_':
                            self.board[cd[0] + j][cd[1] + i] = 'A'

        # Mark 'A' for X and O wins
        for row, col in self.globalIdx:
            coor = check_line(row, col, 1, 0) or check_line(row, col, 0, 1) or check_line(row, col, 1, 1) or check_line(row, col + 2, 1, -1)
            mark_empty_cells(coor)

        # Return the result based on counters
        if counter_x == 3:
            return 1
        elif counter_y == 3:
            return -1
        return 0


    def ec_evaluateDesigned(self, isMax):
        
        score = 0
        counter_500 = 0
        counter_100 = 0

        # Helper function to check patterns
        def check_pattern(player, opponent, *positions):
            nonlocal counter_500, counter_100
            cur_board = [self.board[y][x] for y, x in positions]
            if cur_board.count(player) == 2 and cur_board.count('_') == 1:
                counter_500 += 1
            if cur_board.count(opponent) == 2 and cur_board.count(player) == 1:
                counter_100 += 1

        # Check all patterns
        for row, col in self.globalIdx:
            cur_player = self.maxPlayer if isMax else self.minPlayer
            opponent_player = self.minPlayer if isMax else self.maxPlayer

            # Check rows and columns
            for i in range(3):
                check_pattern(cur_player, opponent_player, (row, col+i), (row+1, col+i), (row+2, col+i))
                check_pattern(cur_player, opponent_player, (row+i, col), (row+i, col+1), (row+i, col+2))

            # Check diagonals
            check_pattern(cur_player, opponent_player, (row, col), (row+1, col+1), (row+2, col+2))
            check_pattern(cur_player, opponent_player, (row, col+2), (row+1, col+1), (row+2, col))

        # Calculate score
        if isMax:
            score += 500 * counter_500 + 100 * counter_100
        else:
            score -= 100 * counter_500 + 500 * counter_100

        # Additional scoring logic
        for row, col in self.globalIdx:
            for y, x in [(row, col), (row+2, col), (row, col+2), (row+2, col+2)]:
                if self.board[y][x] == self.maxPlayer:
                    score += 30
                elif self.board[y][x] == self.minPlayer:
                    score -= 30

        return score

    def my_minimax1(self, depth, currBoardIdx, isMax):
        if (depth == self.maxDepth) or (not self.checkMovesLeft()):
            self.expandedNodes += 1
            return self.ec_evaluateDesigned(self.currPlayer)

        best_value = float('-inf') if isMax else float('inf')
        y, x = self.globalIdx[currBoardIdx]
        for j in range(3):
            for i in range(3):
                if self.board[y+j][x+i] == '_':
                    self.board[y+j][x+i] = self.maxPlayer if isMax else self.minPlayer
                    cur_value = self.my_minimax(depth+1, self.getNextBoardIdx(y+j, x+i), not isMax)
                    self.board[y+j][x+i] = '_'
                    best_value = max(best_value, cur_value) if isMax else min(best_value, cur_value)
        return best_value

    def ec_playGame(self):
        cur_player = True  # true max first, false min first
        cur_board = self.startBoardIdx
        self.expandedNodes = 0
        bestMove = []
        bestValue = []
        gameBoards = []
        expandedNodes = []

        while self.checkMovesLeft() and self.ec_checkWinner() == 0:
            self.currPlayer = cur_player
            y, x = self.globalIdx[cur_board]
            best_coord = (-1, -1)
            best_value = -float('inf') if cur_player else float('inf')
            minimax_func = self.my_minimax if cur_player else self.my_minimax1
            player_symbol = self.maxPlayer if cur_player else self.minPlayer

            # Find the best move for the current player
            for j in range(3):
                for i in range(3):
                    if self.board[y+j][x+i] == '_':
                        self.board[y+j][x+i] = player_symbol
                        cur_board = self.getNextBoardIdx(y+j, x+i)
                        cur_value = minimax_func(1, cur_board, not cur_player)
                        self.board[y+j][x+i] = '_'
                        if (cur_player and cur_value > best_value) or (not cur_player and cur_value < best_value):
                            best_coord = (y+j, x+i)
                            best_value = cur_value

            # Apply the best move
            self.board[best_coord[0]][best_coord[1]] = player_symbol
            cur_board = self.getNextBoardIdx(best_coord[0], best_coord[1])
            bestMove.append(best_coord)
            bestValue.append(best_value)
            gameBoards.append(self.board.copy())  # Store a copy of the board
            expandedNodes.append(self.expandedNodes)
            self.printGameBoard()
            cur_player = not cur_player

            # Check for a winner or if no moves are left
            if self.ec_checkWinner() != 0 or not self.checkMovesLeft():
                break

        winner = self.ec_checkWinner()
        return gameBoards, bestMove, expandedNodes, bestValue, winner
    
if __name__ == "__main__":
    uttt = ultimateTicTacToe()

    start = time.time()
    # gameBoards, best_coord, expandedNodes, best_value, winner = uttt.playGamePredifinedAgent(False, False, False)
    # print(expandedNodes)
    # print(best_value)

    # min_count = 0
    # min_first_cnt = 0
    # nodes_expanded = []
    # for i in range(20):
    #     start_board = randint(0, 8)
    #     max_first = randint(0, 1)
    #     uttt = ultimateTicTacToe()
    #     uttt.startBoardIdx = start_board
    #     gameBoards, best_coord, expandedNodes, best_value, winner = uttt.playGameYourAgent(max_first)
    #     nodes_expanded.append(sum(expandedNodes))
    #     if winner == -1:
    #         min_count += 1
    #     if max_first == 0:
    #         min_first_cnt += 1
    #     print("Round ", i)
    #     print("Max first: ", max_first)
    #     print("Start board: ", start_board)
    # print("Expanded nodes: ", nodes_expanded)
    # print("Our agent wins ", min_count, " games out of 20 games.")
    # print("Our agent plays the first move in ", min_first_cnt, " games.")

    gameBoards, best_coord, expandedNodes, best_value, winner = uttt.playGameHuman()
    # gameBoards, best_coord, expandedNodes, best_value, winner = uttt.ec_playGame()

    print("time spent: ", time.time() - start)


    if winner == 1:
        print("The winner is maxPlayer!!!")
    elif winner == -1:
        print("The winner is minPlayer!!!")
    else:
        print("Tie. No winner:(")