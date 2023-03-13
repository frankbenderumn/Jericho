import logging
import time

from neo4j import GraphDatabase
from neo4j.exceptions import Neo4jError

class App:

    def __init__(self, uri, user, password):
        self.driver = GraphDatabase.driver(uri, auth=(user, password))
        self._schema = "CALL apoc.meta.schema() yield value "
        "UNWIND apoc.map.sortedProperties(value) as labelData "
        "WITH labelData[0] as label, labelData[1] as data "
        "WHERE data.type = 'node' "
        "UNWIND apoc.map.sortedProperties(data.properties) as property "
        "WITH label, property[0] as property, property[1] as propData "
        "RETURN label, "
        "property, "
        "propData.type as type, "
        "propData.indexed as isIndexed, "
        "propData.unique as uniqueConstraint, "
        "propData.existence as existenceConstraint"

    def close(self):
        # Don't forget to close the driver connection when you are finished with it
        self.driver.close()

    def read(self, function, args):
        with self.driver.session(database="neo4j") as session:
            result = session.execute_read(function, args)
            # for record in result:

    def write(self, function, args):
        with self.driver.session(database="neo4j") as session:
            result = session.execute_read(function, args)

    def run(self, function, args):
        with self.driver.session(database="neo4j") as session:
            result = session.run(function, args)

    def schema(self):
        with self.driver.session(database="neo4j") as session:
            result = session.run(self._schema)
            for record in result:
                print(record)
                print("-----------------------------------")

    def create_friendship(self, person1_name, person2_name):
        with self.driver.session(database="neo4j") as session:
            # Write transactions allow the driver to handle retries and transient errors
            result = session.execute_write(
                self._create_and_return_friendship, person1_name, person2_name)
            for record in result:
                print("Created friendship between: {p1}, {p2}"
                      .format(p1=record['p1'], p2=record['p2']))

    @staticmethod
    def _create_and_return_friendship(tx, person1_name, person2_name):
        # To learn more about the Cypher syntax, see https://neo4j.com/docs/cypher-manual/current/
        # The Reference Card is also a good resource for keywords https://neo4j.com/docs/cypher-refcard/current/
        query = (
            "CREATE (p1:Person { name: $person1_name }) "
            "CREATE (p2:Person { name: $person2_name }) "
            "CREATE (p1)-[:KNOWS]->(p2) "
            "RETURN p1, p2"
        )
        result = tx.run(query, person1_name=person1_name, person2_name=person2_name)
        try:
            return [{"p1": record["p1"]["name"], "p2": record["p2"]["name"]}
                    for record in result]
        # Capture any errors along with the query and data for traceability
        except Neo4jError as exception:
            logging.error("{query} raised an error: \n {exception}".format(
                query=query, exception=exception))
            raise

    def find_person(self, person_name):
        with self.driver.session(database="neo4j") as session:
            result = session.execute_read(self._find_and_return_person, person_name)
            for record in result:
                print("Found person: {record}".format(record=record))

    @staticmethod
    def _find_and_return_person(tx, person_name):
        query = (
            "MATCH (p:Person) "
            "WHERE p.name = $person_name "
            "RETURN p.name AS name"
        )
        result = tx.run(query, person_name=person_name)
        return [record["name"] for record in result]

    def _keys(tx, name):
        query = (
            "MATCH (a) WHERE a.name = 'Alice'"
            "RETURN keys(a)"
        )

class NeoConn:
    def __init__(self, url, user, password):
        self._url = url
        self._user = user
        self._password = password
        self._driver = None
        self._db = "neo4j"
        try:
            self._driver = GraphDatabase.driver(self._url, auth=(self._user, self._password))
        except Exception as e:
            print("Failed to create the driver: ",e)

    def close(self):
        if self._driver is not None:
            self._driver.close()

    def query(self, query, params=None, db=None):
        if db == None:
            db = self._db
        assert self._driver is not None, "\033[1;31mDriver not initialized!\033[0m"
        session = None
        response = None
        try:
            session = self._driver.session(database=db) if db is not None else self._driver.session()
            response = list(session.run(query, params))
        except Exception as e:
            print("Query failed:", e)
        finally:
            if session is not None:
                session.close()
        return response

# 7xnaMk7uRe5PPoFq_vWEa2jZX6Yqy6RDyFM_14JSYVY

def add_categories(conn, categories):
    query = """
    UNWIND $rows as row
    MERGE (c:Category {category: row.category})
    RETURN count(*) as total
    """

    return conn.query(query, params={'rows':categories.to_dict('records')})

def insert_data(conn, query, rows, batch_size=10000):
    total = 0
    batch = 0
    start = time.time()
    result = None

    while batch * batch_size < len(rows):
        res = conn.query(query, params = {'rows': rows[batch*batch_size:(batch+1)*batch_size].to_dict('records')})
        total += res[0]['total']
        batch += 1
        result = {"total":total,
                "batches":batch,
                "time":time.time()-start}
        print(result)

    return result


def add_papers(rows, batch_size=5000):
   # Adds paper nodes and (:Author)--(:Paper) and 
   # (:Paper)--(:Category) relationships to the Neo4j graph as a 
   # batch job.
 
    query = """
    UNWIND $rows as row
    MERGE (p:Paper {id:row.id}) ON CREATE SET p.title = row.title

    // connect categories
    WITH row, p
    UNWIND row.category_list AS category_name
    MATCH (c:Category {category: category_name})
    MERGE (p)-[:IN_CATEGORY]->(c)

    // connect authors
    WITH distinct row, p // reduce cardinality
    UNWIND row.cleaned_authors_list AS author
    MATCH (a:Author {name: author})
    MERGE (a)-[:AUTHORED]->(p)
    RETURN count(distinct p) as total
    """

    return insert_data(query, rows, batch_size)


def add_authors(rows, batch_size=10000):
    query = """
    UNWIND $rows as row
    MERGE (:Author {name: row.author})
    RETURN count(*) as total
    """

    return insert_data(query, rows, batch_size)

if __name__ == "__main__":
    # Aura queries use an encrypted connection using the "neo4j+s" URI scheme
    url = "neo4j+s://02cb61fc.databases.neo4j.io"
    user = "neo4j"
    password = "shdemo1234"
    # app = App(url, user, password)
    # app.schema()
    # app.close()

    conn = NeoConn(url, user, password)
    # conn.query('CREATE CONSTRAINT papers IF NOT EXISTS ON (p:Paper) ASSERT p.id IS UNIQUE')
    # conn.query('CREATE CONSTRAINT authors IF NOT EXISTS ON (a:Author) ASSERT a.name IS UNIQUE')
    # conn.query('CREATE CONSTRAINT cate1gories IF NOT EXISTS ON (c:Category) ASSERT c.category IS UNIQUE')

    result = conn.query("CREATE (n)")
    print("Did it work?")
    print(result)

    # delete all words

    result = conn.query("CREATE (n:Word {wnid: 5, name: 'walk', pos: 'verb', tense: 'present'}) RETURN n.name")
    print(result)

    result = conn.query("MATCH (n:Word) DETACH DELETE n")

    # categories = ["music", "history", "math"]
    # authors = ["Mark Twain", "Agatha Christie", "Rick Riordan"]
    # add_categories(categories)
    # add_authors(authors)
    print("\033[1;32mFinished\033[0m")