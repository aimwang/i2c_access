# i2c_access
i2c access

syntax:
i2c_access width path rw addr reg [value]
	width: b=byte, w=wide(2 bytes)
	path: i2c handle file. ex: /dev/i2c-0
	rw: read/write: r=read, w=write
	addr: slave addr. ex: 0x30
	reg: register addr. ex: 0x0a
	value: wrtie value. ex: 0xff. This is only need when write mode.
