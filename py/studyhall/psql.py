import psycopg2
import typing
import psycopg2
from psycopg2 import Error

default_user="fbender"
default_password="password"
default_host="127.0.0.1"
default_port="5432"
default_db="sh-data"

outgoing_host="34.155.202.24"
remote_host="34.163.50.45"
remote_clone="34.163.215.75"

remote_socket='/cloudsql/{}'.format("datascience-358914:europe-west9:sh1-pgbd")

default_user="frank"
default_password="=;x?QroRp3t0*zG)"
default_host=remote_clone
default_port="5432"
default_db="sh1"


Row = list

def red() -> None:
    print("\033[1;31m",end='')

def gre() -> None:
    print("\033[1;32m",end='')

def yel() -> None:
    print("\033[1;33m",end='')

def blu() -> None:
    print("\033[1;34m",end='')

def mag() -> None:
    print("\033[1;35m",end='')

def cya() -> None:
    print("\033[1;36m",end='')

def whi() -> None:
    print("\033[1;37m",end='')

def clr() -> None:
    print("\033[0m",end='')

def read(path: str) -> str:
    text_file = open(path, "r")
    data = text_file.read()
    text_file.close()
    return data

class Postgres:
    def __init__(self, user=default_user,
                        password=default_password,
                        host=default_host,
                        port=default_port,
                        database=default_db) -> None:
        try:
            self.conn = psycopg2.connect(user=user,
                                        password=password,
                                        host=host,
                                        port=port,
                                        database=database)
            cursor = self.conn.cursor()
            yel(); print("PostgreSQL server information",end=''); clr()
            print(self.conn.get_dsn_parameters(), "\n")
            cursor.execute("SELECT version();")
            record = cursor.fetchone()
            gre(); print("You are connected to - ", record, end='\n'); clr();
        except:
            red()
            print("Failed to connect to postgresurl with params:\n",
                "\tUser: ",user,"\n"
                "\tPassword: ",password,"\n"
                "\tHost: ",host,"\n"
                "\tPort: ",port,"\n"
                "\tDatabase: ",database, sep='')
            clr()
                # Create a cursor to perform database operations
                
    def inject(self, path: str) -> None:
        schema: str = read(path)
        cur = self.conn.cursor()
        cur.execute(schema)
        cur.close()
        self.conn.commit()

    def all(self, table: str) -> Row:
        cur = self.conn.cursor()
        q = "SELECT * FROM " + table + ";"
        cur.execute(q)
        l = []
        records: list[tuple] = cur.fetchall()
        print("Print each row and it's columns values")
        for i in range(len(records)):
            print(i, " ", records[i])
            # print("Model = ", row[1])
            # print("Price  = ", row[2], "\n")
        cur.close()
        self.conn.commit()
        return l

    def _cast_value(self, val) -> str:
        newVal = None
        if type(val) is str:
            newVal = "'" + val + "'"
        elif type(val) is int:
            newVal = str(val)
        elif type(val) is float:
            newVal = str(val)
        else:
            red(); print("Variable type is not supported for this postgres driver"); clr();
        return newVal

    def _construct_insert(self, table: str, record: list) -> str:
        attrs = self.attrs(table)
        attr_str = ""
        value_str = ""
        for i in range(len(attrs)):
            print(attrs[i])
            attr_str = attr_str + attrs[i] + ", "
        for i in range(len(record)):
            print(record[i])
            val = self._cast_value(record[i])
            value_str = value_str + val + ", "
        if (len(value_str) > 2):
            value_str = value_str[:-2]
        else:
            red(); print("No values provided"); clr()
        if (len(attr_str) > 2):
            attr_str = attr_str[:-2]
        else:
            red(); print("No attrs provided"); clr()
        prefix = "INSERT INTO " + table + " (" + attr_str + ") VALUES (" + value_str + ");"
        yel(); print("INSERT QUERY IS: ",prefix); clr()
        return prefix

    def tables(self) -> Row:
        cur = self.conn.cursor()
        cur.execute("select schemaname,relname from pg_stat_user_tables;")
        vals = cur.fetchall()
        tables = []
        for el in range(len(vals)):
            tables.append(vals[el][1])
            blu(); print(vals[el][1]); clr()
        return tables

    def insert(self, table: str, record: Row):
        attrs = self.attrs(table)
        print("ATTR LEN: ", len(attrs))
        print("RECORD LEN: ", len(record))
        if (len(attrs) != len(record)):
            red(); print("Invalid number of attrs given for insert: Requires",len(attrs),"but",len(record),"given!"); clr();
            return
        q = self._construct_insert(table, record)
        cur = self.conn.cursor()
        cur.execute(q)
        self.conn.commit()

    def export(self) -> None:
        pass

    def attrs(self, table: str) -> Row:
        cur = self.conn.cursor()
        cur.execute("select column_name from information_schema.columns where table_schema = 'public' and table_name='" + table + "'")
        column_names = [row[0] for row in cur]
        mag(); print(table + ": ", end=''); print(column_names); clr()
        return column_names

    def info(self) -> None:
        for i in self.tables():
            self.attrs(i)