#import sys
import sqlite3
import json
import re

conn = sqlite3.connect('radio.db')
c = conn.cursor()

init = open("life.txt", 'r')
output = open('test.txt', 'w')

count = 0

extract = []
txtExtractYears = []
txtExtract = []

# x minus one uses the ogg file, as mp3 is broken in chrome
# We will match in this case by year aired
# date formats encountered:
# Suspense: \d\d\d\d\d\d
# Inner Sanctum, Dragnet: \d\d-\d\d-\d\d

with open('life.json') as jsondata:
	d = json.load(jsondata)
	for i in d:
		regex = re.search(r'(\d\d-\d\d-\d\d)', i["orig"])
		if regex:
			yearFormat = regex.group(1)
			#monthFormat = regex.group(2)
			#dayFormat = regex.group(3)
			
			newYear = yearFormat #+ "-" + monthFormat + "-" + dayFormat
			tmp = [i["title"], newYear, i["sources"][0]["file"] ]
			extract.append(tmp)

for line in init:
	year = line[2:10].replace('/', '-')
	title = line[16:].rstrip()
	txtExtractYears.append(year)
	txtExtract.append([year, title])
		#mp3 = "https://archive.org" + (d[count]["sources"][0]["file"])
		#c.execute("INSERT INTO radio VALUES(?, ?, ?, ?, ?)", (title, "Suspense", "Mystery/Horror", year, mp3))

		#count += 1

for ep in extract:
	if ep[1] in txtExtractYears:
		idx = txtExtractYears.index(ep[1])
		title = txtExtract[idx][1]
		year = txtExtract[idx][0]
		mp3 = "https://archive.org" + ep[2]
		#output.write(title + " " + year + " " + mp3 + "\n")
		c.execute("INSERT INTO radio VALUES(?, ?, ?, ?, ?)", (title, "The Life of Riley", "Comedy", year, mp3))

		
		
		

conn.commit()
conn.close()




#with open('text.json') as jsondata:
	#d = json.load(jsondata)
	#for i in d:
		#mp3 = (i["title"])#[0]["file"])
		#new = mp3.replace("_", " ").split(" ")[3]
		#output2.write(new)
		#output2.write("\n")

#numbers = []

#for line in output2:
	#numbers.append(int(line))

#for i in range(0,945):
	#if i not in numbers:
		#print(i)



		