class QueryBuilder:
    def __init__(self, command):
        self._command = command

    def generate(self):
        if self._command == "CREATE":
            return "CREATE"
        elif self._command == "READ":
            return "READ"
        elif self._command == "UPDATE":
            return "UPDATE"
        elif self._command == "DELETE":
            return "DELETE"