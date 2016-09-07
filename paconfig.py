#!/usr/bin/python
#pconfig.py

#parse config file 

import os
import string

def openconfig(filename) :
	try :
		if not os.path.exists(filename) :
			return None
		f = open(filename)
		return f 
	except IOError:
		return None
	except NameError:
		return None
	except Exception:
		return None
	finally :
		pass

	return None

def readline(file) :
	return file.readline()

def readlinelist(file) :
	return file.readlines()

def closeconfig(file) :
	file.close()

def seek(file, off) :
	file.seek(off, 0)

def reinvalid(text) :
	#remove note
	index = text.rfind(';', 0)
	if index > -1 :
		text = text[0:index]

	#remove space 
	text = text.strip()

	return text

def pnsection(text) :
	#remove invalid string
	text = reinvalid(text)

	#parse section
	begin = text.find('[')
	end = text.rfind(']', 0)
	if begin > -1 and end >= begin :
		return [True, begin, end]

	return [False, 0, 0]

def pasection(text, section) :
	#parse next section
	listresult = pnsection(text)
	if listresult[0] :
		text = text[listresult[1] + 1:listresult[2]]
		if text == section :
			return True

	return False

def pakey(text, key) :
	#remove invalid string
	text = reinvalid(text)
	
	#remove = left and right space
	eqindex = text.find('=')
	if eqindex < 0 :
		return None

	#split eq left and right text
	ltext = text[0:eqindex].strip()
	rtext = text[eqindex + 1:].strip()
	if ltext == key :
		return rtext

	return None

def pastring(fp, section, key, defv) :
	seek(fp, 0)
	textlist = readlinelist(fp)

	fsec = False
	for text in textlist :
		if fsec :
			#find next section
			if pnsection(text)[0] :
				return defv
			#find value
			value = pakey(text, key)
			if value != None :
				if value == "" :
					return defv
				return value
		#find section
		elif pasection(text, section) :
			fsec = True

	return defv

def panumber(fp, section, key, defv, type) :
	value = pastring(fp, section, key, "")
	if value == "" :
		return defv

	try :
		if type == 1 :
			return int(value)
		elif type == 2:
			return string.atof(value)
	except ValueError:
		return defv
	except Exception :
		return defv
	finally :
		pass

	return defv

def paint(fp, section, key, defv) :
	return panumber(fp, section, key, defv, 1)

def pafloat(fp, section, key, defv) :
	return panumber(fp, section, key, defv, 2)

class sys :
    def __init__(self) :
        self.ip = ""
        self.port = 0
        
        self.host = ""
        self.user = ""
        self.paw = ""
        self.db = ""
        self.dbport = 0
    
        
    def getip(self) :
        return self.ip
    
    def getport(self) :
        return self.port
    
    def setip(self, ip) :
        self.ip = ip
    
    def setport(self, port) :
        self.port = port
    
    def gethost(self) :
        return self.host
    
    def getuser(self) :
        return self.user
    
    def getpaw(self) :
        return self.paw
    
    def getdb(self) :
        return self.db
    
    def getdbport(self) :
        return self.dbport

    def sethost(self, host) :
            self.host = host

    def setuser(self, user) :
        self.user = user

    def setpaw(self, paw) :
        self.paw = paw

    def setdb(self, db) :
        self.db = db

    def setdbport(self, dbport) :
        self.dbport = dbport


def getsyscon(filename, sysc) :
    f = openconfig(filename)
    if f == None :
        return 0

    sysc.setip(pastring(f, "NET", "ip", "127.0.0.1"))

    sysc.setport(paint(f, "NET", "port", 8080))

    sysc.sethost(pastring(f, "DB", "host", "127.0.0.1"))

    sysc.setuser(pastring(f, "DB", "user", "root"))

    sysc.setpaw(pastring(f, "DB", "pass", "root"))

    sysc.setdb(pastring(f, "DB", "db", "sharepos"))

    sysc.setdbport(paint(f, "DB", "port", 3306))
    
    closeconfig(f)
        
    return 1

if __name__ == '__main__' :
    sysc = sys()
    ret = getsyscon('./server.ini', sysc)
    if (ret == 1) :
        print sysc.getip(), sysc.getport()


