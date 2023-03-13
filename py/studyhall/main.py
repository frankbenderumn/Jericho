from psql import *

conn = Postgres()

tables = conn.tables()
print(len(tables))

# gcloud sql connect sh1-pgbd --user=frank --database=sh1

# list constraints

words = conn.all("wn_semlinkref")
print(words)