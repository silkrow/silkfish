import json

class Configure:
    """
    A class for loading and accessing configuration settings from a JSON file.

    Attributes:
        filename (str): The name of the JSON file containing configuration settings.
        config (dict): The loaded configuration settings from the JSON file.
    """

    def __init__(self, filename):
        """
        Initialize the Configure instance with the specified filename.

        Args:
            filename (str): The name of the JSON file containing configuration settings.
        """
        self.filename = filename
        self.config = self.load_config()

    def load_config(self):
        """
        Load the configuration settings from the JSON file.

        Returns:
            dict: A dictionary containing the loaded configuration settings.
        """
        try:
            with open(self.filename, 'r') as f:
                config = json.load(f)
                return config
        except FileNotFoundError:
            print(f"Error: File '{self.filename}' not found.")
            return None
        except json.JSONDecodeError:
            print(f"Error: Unable to decode JSON in '{self.filename}'.")
            return None

    def get_minimax_depth(self):
        """
        Get the depth of minimax search from the configuration settings.

        Returns:
            int: The depth of minimax search.
        """
        return self.config.get("minimax_depth")

    def get_evaluation_weights(self):
        """
        Get the evaluation weights from the configuration settings.

        Returns:
            dict: A dictionary containing the evaluation weights.
        """
        return self.config.get("evaluation_weights")
