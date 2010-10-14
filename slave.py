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
	1: msg_ls
}


#
#	Main Guts
#

def mainLoop():
	while (1):
		line = sys.stdin.readline().strip()
		try: line = binascii.a2b_base64(line)
		except: continue
		message = TLD()
		message.setData(line)
		reply = handleMessage(message)
		print binascii.b2a_base64(reply.getData())

def handleMessage(message):
	(requestId, messageId) = message.read('LH')
	try:
		if (not messageDefs.has_key(messageId)): throw("Invalid messageId: " + str(messageId))
		result = TLD()
		result.write('LB', requestId, 1)
		messageDefs[messageId](message, result)
	except Exception, e:
		err = TLD()
		err.write('LB', requestId, 0)
		err.writeString(str(e))
		return err
	return result

mainLoop()
