#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <err.h>
#include <errno.h>

#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define WIDTH_BYTE 1
#define WIDTH_WIDE 2
#define	MODE_READ 0
#define	MODE_WRITE 1

static inline __s32 i2c_smbus_access(int file, char read_write, __u8 command, int size, union i2c_smbus_data *data)
{
	struct i2c_smbus_ioctl_data args;

	args.read_write = read_write;
	args.command = command;
	args.size = size;
	args.data = data;
	return ioctl(file, I2C_SMBUS, &args);
}

static inline __s32 i2c_smbus_read_byte_data(int file, __u8 command)
{
	union i2c_smbus_data data;
	if (i2c_smbus_access(file, I2C_SMBUS_READ, command, I2C_SMBUS_BYTE_DATA, &data)) {
		return -1;
	} else {
		return 0x0FF & data.byte;
	}
}

static inline __s32 i2c_smbus_read_word_data(int file, __u8 command)
{
	union i2c_smbus_data data;
	if (i2c_smbus_access(file, I2C_SMBUS_READ, command, I2C_SMBUS_WORD_DATA, &data)) {
		return -1;
	} else {
		return 0x0FFFF & data.word;
	}
}

static inline __s32 i2c_smbus_write_byte_data(int file, __u8 command, __u8 value)
{
	union i2c_smbus_data data;

	data.byte = (0x0ff & value);

	return i2c_smbus_access(file, I2C_SMBUS_WRITE, command, I2C_SMBUS_BYTE_DATA, &data);
}

static inline __s32 i2c_smbus_write_word_data(int file, __u8 command, __u16 value)
{
	union i2c_smbus_data data;

	data.word = (0x0ffff & value);

	return i2c_smbus_access(file, I2C_SMBUS_WRITE, command, I2C_SMBUS_WORD_DATA, &data);
}

void syntax(char *command)
{
	printf ("compile time %s\n", __DATE__);
	printf ("%s width path rw addr reg [value]\n", command);
	printf ("\twidth: b=byte, w=wide (2 bytes)\n");
	printf ("\tpath: i2c handle file. ex: /dev/i2c-0\n");
	printf ("\trw: read/write: r=read, w=write\n");
	printf ("\taddr: slave addr. ex: 0x30\n");
	printf ("\treg: register addr. ex: 0x0a\n");
	printf ("\tvalue: wrtie value. ex: 0xff\n");
}

int main(int argc, char **argv)
{
	uint8_t width, rw, addr, reg;
	uint8_t data;
	uint16_t wdata;
	int file, rc, ret;

	if (argc < 6 || argc > 7) {
		syntax(argv[0]);
		return -1;
	}

	// [1] width
	if (argv[1][0] == 'b') {
		width = WIDTH_BYTE;
	}
	else if (argv[1][0] == 'w') {
		width = WIDTH_WIDE;
	} else {
		printf ("I can not understand width as [%s]\n", argv[1]);
		return -1;
	}

	// [2] path
	const char *path = argv[2];
	file = open(path, O_RDWR);
	if (file < 0) {
		printf("[ERR %d] Tried to open '%s'", file, path);
		return file;
	}

	// [3] rw
	if (argv[3][0] == 'r') {
		rw = MODE_READ;
	}
	else if (argv[3][0] == 'w') {
		rw = MODE_WRITE;
 	} else {
		printf ("I can not understand rw as [%s]\n", argv[3]);
		return -1;
	}

	// [4] addr
	addr = strtoul(argv[4], NULL, 0);

	rc = ioctl(file, I2C_SLAVE, addr);
	if (rc < 0) {
		printf("[ERR %d] Tried to set device address'0x%02x' FAIL!\n", rc, addr);
		return rc;
	}

	// [5] reg
	reg = strtoul(argv[5], NULL, 0);

	if (width == WIDTH_BYTE) {
		if (rw == MODE_READ) {
			ret = i2c_smbus_read_byte_data(file, reg);
			if (ret < 0) {
				printf("[ERR] i2c read FAIL!\n");
			} else {
				data = ret & 0x0ff;
				printf("%s: device 0x%02x at address 0x%02x: 0x%02x\n", path, addr, reg, data);
			}
		} else {
			// [6] value
			data = strtoul(argv[6], NULL, 0);
			ret = i2c_smbus_write_byte_data(file, reg, data);
			if (ret) {
				printf("[ERR %d] i2c write FAIL!\n", ret);
			}
		}
	} else {
		if (rw == MODE_READ) {
			ret = i2c_smbus_read_word_data(file, reg);
			if (ret < 0) {
				printf("[ERR] i2c read FAIL!\n");
			} else {
				wdata = ret & 0x0ffff;
				printf("%s: device 0x%02x at address 0x%02x: 0x%04x\n", path, addr, reg, wdata);
			}
		} else {
			// [6] value
			wdata = strtoul(argv[6], NULL, 0);
			ret = i2c_smbus_write_word_data(file, reg, wdata);
			if (ret) {
				printf("[ERR %d] i2c write FAIL!\n", ret);
			}
		}
	}
}

