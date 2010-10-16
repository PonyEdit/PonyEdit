import os, sys, binascii, struct

thefile = ''
thefilename = ''
logfile = open("log.txt", "a")

def log(t):
	logfile.write(t + '\n')
	logfile.flush()

#
#	TLD class
#

class TLD:
	def __init__(self):
		self.data = ''

	def setData(self, data):
		self.data = data
		self.cursor = 0

	def read(self, fmt):
		fmt = '<' + fmt
		v = struct.unpack_from(fmt, self.data, self.cursor)
		self.cursor += struct.calcsize(fmt)
		return v

	def readString(self):
		length = self.read('L')[0]
		v = self.data[self.cursor:self.cursor + length]
		self.cursor += length
		return v

	def write(self, fmt, *args):
		self.data += struct.pack(fmt, *args)

	def writeString(self, s):
		self.data += struct.pack('<L', len(s)) + s

	def getData(self):
		return self.data

	def atEnd(self):
		return self.cursor >= len(self.data)


#
#	Message Handlers
#

#	ls
def msg_ls(message, result):
	log('Running ls')
	for filename in os.listdir('.'):
		stat = os.stat(filename)
		result.writeString(filename)
		result.write('L', stat.st_size)

#	open
def msg_open(message, result):
	log('Running open')
	global thefilename
	global thefile
	name = message['f']
	log('Filename = ' + name)
	thefilename = name
	f = open(name, "r")
	thefile = f.read()
	f.close()
	log('****** File Contents: ')
	log(thefile)
	log('**********************')

#	change
def msg_change(message, result):
	global thefilename
	global thefile
	position = message['p']
	remove = message['r']
	add = message['a']
	thefile = thefile[0:position] + add + thefile[position + remove:]
	log('file contents:')
	log(thefile)

#	save
def msg_save(message, result):
	global thefilename
	global thefile
	log("saving to: " + thefilename)
	f = open(thefilename, "w")
	f.write(thefile)
	f.close()

#
#	Message Definitions
#

messageDefs = \
{
	1: msg_ls,
	2: msg_open,
	3: msg_change,
	4: msg_save,
}


#
#	Main Guts
#

def mainLoop():
	while (1):
		line = sys.stdin.readline().strip()
		try: line = binascii.a2b_base64(line)
		except:
			log('Receied some bogus input: ' + line)
			continue
		block = TLD()
		block.setData(line)
		log('--> Received ' + str(len(line)) + ' bytes...')
		while (not block.atEnd()):
			log('Reading messages at ' + str(block.cursor) + '...')
			reply = handleMessage(block)
			print binascii.b2a_base64(reply.getData()).strip()
		log('Finished handling them bytes')

def handleMessage(message):
	(messageId, bufferId, length) = message.read('HLL')

	block = {}
	target = message.cursor + length
	while (message.cursor < target):
		(f, t) = message.read('BB')
		if (t == 0x01):
			d = message.read('h')[0]
		elif (t == 0x81):
			d = message.read('H')[0]
		elif (t == 0x02):
			d = message.read('l')[0]
		elif (t == 0x82):
			d = message.read('L')[0]
		else:
			d = message.readString()
		block[chr(f)] = d

	log('messageId = ' + str(messageId))
	log('Paramaters = ' + str(block))
	try:
		if (not messageDefs.has_key(messageId)): raise Exception("Invalid messageId: " + str(messageId))
		result = TLD()
		result.write('B', 1)
		messageDefs[messageId](block, result)
	except Exception, e:
		log('Error occurred: ' + str(e))
		err = TLD()
		err.write('B', 0)
		err.writeString(str(e))
		return err
	return result


#
#	Send startup info
#

log('*************************************** Starting up *********************************************')

mainLoop()
