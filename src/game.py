class Game:
    '''
    Class Name: Game
    Description: Game holds all the relative configurations of the
        ongoing game, including the information of the two players, the
        turn of the game, the preset configurations for the engine, etc.
    '''

    def __init__(self, conf_file1, conf_file2):
        '''
        Load the configuration from a specified configuration file. 
        Initialize a chess game. 
        '''

        if conf_file1 == None:
            