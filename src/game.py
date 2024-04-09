from configure import Configure

class Game:
    """
    A class to hold all the needed information of a game, including 
    engine configuration, board state, etc.
    """

    def __init__(self, conf_file1, conf_file2):
        """
        Load the configuration from a specified configuration file. 
        Initialize a chess game. 
        """
        self.conf1 = Configure(conf_file1)
        self.conf2 = Configure(conf_file2)
        self.conf1.load_config()
        self.conf2.load_config()

        print(self.conf1.get_minimax_depth())
        print(self.conf2.get_evaluation_weights())




if __name__ == "__main__":
    game = Game("../config/config.json", "../config/config.json")

        