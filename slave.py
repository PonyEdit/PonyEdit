import sys, binascii

while (1):
	line = sys.stdin.readline().strip()
	print "Line = '" + line + "'"
	print binascii.a2b_base64(line)
	
