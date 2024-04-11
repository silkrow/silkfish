import json

class Configure:
    """
    A class for loading and accessing configuration settings from a JSON file.
    """

    def __init__(self, filename):
        """
        Initialize the Configure instance with the specified filename.
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
            raise ValueError(f"Error: Configuration file '{self.filename}' not found.")
        except json.JSONDecodeError:
            raise ValueError(f"Error: Unable to decode JSON configuration in '{self.filename}'.")

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
