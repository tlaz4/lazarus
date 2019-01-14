from flask import render_template, g, request, jsonify, Response
from app import app
import sqlite3
import datetime
import random

DATABASE = "radio.db"

genres = ["Adventure", "Comedy", "Detective", "Mystery/Horror", "Police", "Science Fiction", "Western"]

def get_db():
    db = getattr(g, '_database', None)
    if db is None:
        db = g._database = sqlite3.connect(DATABASE)
    return db

#c.execute('''CREATE TABLE IF NOT EXISTS radio
			#(title text, series text, genre text, link text)''')

#c.execute("INSERT INTO radio VALUES ('The Hitch-Hiker','Suspense','Suspense', 'https://archive.org/download/OTRR_Suspense_Singles/Suspense_420902_011_The_Hitch-Hiker_-128-44-_27537_29m17s.mp3')")
#c.execute("SELECT * FROM radio")
#print(c.fetchone())
#conn.commit()
#conn.close()


@app.route('/')
def main():
	cur = get_db().cursor()
	#cur.execute("SELECT DISTINCT genre FROM radio")
	#genres = cur.fetchall()
	cur.execute("SELECT * FROM radio WHERE airdate LIKE ?", ('%' + getDate().strftime("%m-%d"),))
	todayShows = cur.fetchall()
	date = getDate().strftime("%b %d")
	return render_template('index.html', genres = genres, todayShows = todayShows, date=date)

@app.route('/scheduleS.txt')
def scheduleS():
	playlist = [(show[4] + "\n").replace("ogg", "mp3") for show in pickRandomShow()]

	return Response(playlist, 
					mimetype='text/plain',
					headers={"Content-Disposition":
                             "attachment;filename=scheduleS.txt"})

@app.route('/listen')
def listen():
	return render_template('listen.html')

@app.teardown_appcontext
def close_connection(exception):
    db = getattr(g, '_database', None)
    if db is not None:
        db.close()

@app.route('/ajax', methods=['POST'])
def ajax():
	query = request.form["query"]
	phase = request.form["menu"]
	cur = get_db().cursor()
	if phase == "Series":
		cur.execute("SELECT DISTINCT series FROM radio WHERE genre=?", (query,))
	elif phase == "Title":
		cur.execute("SELECT DISTINCT title, url, airdate FROM radio WHERE series=? ORDER BY airdate", (query,))
	else:
		cur.execute("SELECT DISTINCT genre FROM radio")
	results = cur.fetchall()
	print(results)
	return jsonify({'results': results})

@app.route('/getRandom', methods=['POST'])
def getRandom():
	randomShows = pickRandomShow()

	return jsonify({'results' : randomShows})

def getDate():
	return datetime.datetime.now()

def pickRandomShow():
	cur = get_db().cursor()

	shows = []
	randomGenres = [genres[random.randint(0, 6)], 
					genres[random.randint(0, 6)], 
					genres[random.randint(0, 6)], 
					genres[random.randint(0, 5)]]

	for genre in randomGenres:
		cur.execute("SELECT series FROM radio WHERE genre=? GROUP BY series ORDER BY RANDOM() LIMIT 1", (genre,))
		result = cur.fetchone()

		cur.execute("SELECT * FROM radio WHERE series=? ORDER BY RANDOM() LIMIT 1", (result[0],))
		shows.append(cur.fetchone())

	return shows


