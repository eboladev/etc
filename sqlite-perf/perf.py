import sqlite3
import random

LIMIT = 1000000

def generate(name, perform):
  print "Generating", name, perform

  f = open("words.txt")
  lines = f.read().split("\n")
  f.close()

  conn = sqlite3.connect(name)
  c = conn.cursor()
  c.execute("drop table if exists contacts")
  if perform:
    c.execute("create table contacts (name text, uname text unique)")
  else:
    c.execute("create table contacts (name text)")

  for i in range(LIMIT):
    index = random.randint(0, len(lines) - 1)
    t = (lines[index], )
    c.execute("insert into contacts (name) values(?)", t)

    if perform:
      u = (t[0] + str(c.lastrowid), c.lastrowid)
      c.execute("update contacts set uname = ? where rowid = ?", u)
      print i, t[0], u[0]
    else:
      print i, t[0]

  conn.commit()
  c.close()

if __name__ == "__main__":
  generate("/tmp/perf.db", True)
  generate("/tmp/noperf.db", False)
