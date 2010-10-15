import os, sys, binascii, struct


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
		v = self.data[self.cursor:length]
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
	for filename in os.listdir('.'):
		stat = os.stat(filename)
		result.writeString(filename)
		result.write('L', stat.st_size)

#
#	Message Definitions
#

messageDefs = \
{
	1: msg_ls,
#	2: msg_open,
#	3: msg_change,
#	4: msg_save,
}


#
#	Main Guts
#

def mainLoop():
	while (1):
		line = sys.stdin.readline().strip()
		try: line = binascii.a2b_base64(line)
		except: continue
		block = TLD()
		block.setData(line)
		while (not block.atEnd()):
			reply = handleMessage(block)
			print binascii.b2a_base64(reply.getData())

def handleMessage(message):
	(messageId, size) = message.read('HL')
	try:
		if (not messageDefs.has_key(messageId)): raise Exception("Invalid messageId: " + str(messageId))
		result = TLD()
		result.write('B', 1)
		messageDefs[messageId](message, result)
	except Exception, e:
		err = TLD()
		err.write('B', 0)
		err.writeString(str(e))
		return err
	return result


#
#	Send startup info
#

#welcomeMsg = TLD()
#welcomeMsg.writeString(os.getcwd())
#print binascii.b2a(welcomeMsg)

mainLoop()
