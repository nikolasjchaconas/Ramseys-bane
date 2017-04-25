# Import the driver.
import psycopg2

class DBStore():
    def __init__(self, database, user, host, port=26257):
        # Connect to db on one of db_server hosts.
        self.conn = psycopg2.connect(database=database, user=user, host=host, port=port)

        # Make each statement commit immediately.
        self.conn.set_session(autocommit=True)


    # def createDB(self, create_statement):
    #     # Open a cursor to perform database operations.
    #     cur = self.conn.cursor()

    #     # Create the table.
    #     cur.execute(create_statement)
    #     cur.close()


    def create(self, create_statement):
        # Open a cursor to perform database operations.
        cur = self.conn.cursor()

        # Create the table.
        cur.execute(create_statement)
        cur.close()


    def insert(self, insert_statement):
        # Open a cursor to perform database operations.
        cur = self.conn.cursor()

        # Insert rows into the able.
        cur.execute(insert_statement)
        cur.close()


    def get(self, get_statement):
        # Open a cursor to perform database operations.
        cur = self.conn.cursor()

        cur.execute(get_statement)
        rows = cur.fetchall()

        cur.close()
        return rows


    def delete(self, delete_statement):
        # Open a cursor to perform database operations.
        cur = self.conn.cursor()

        # Insert rows into the able.
        cur.execute(delete_statement)
        cur.close()


    def closeConnection(self):
        self.conn.close()