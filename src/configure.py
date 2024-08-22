import json

class Configure:
    """
    A class for loading and accessing configuration settings from a JSON file.
    """

    def __init__(self, filename):
        self.filename = filename
        self.config = self.load_config()

    def load_config(self):
        try:
            with open(self.filename, 'r') as f:
                config = json.load(f)
                return config
        except FileNotFoundError:
            raise ValueError(f"Error: Configuration file '{self.filename}' not found.")
        except json.JSONDecodeError:
            raise ValueError(f"Error: Unable to decode JSON configuration in '{self.filename}'.")
