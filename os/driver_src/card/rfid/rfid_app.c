#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define RFID_DEV_PATH "/dev/rfid_control"
#define RFID_CODE_LEN 4
#define DEFAULT_INTERVAL_MS 50

static uint32_t bytes_to_u32_be(const unsigned char data[RFID_CODE_LEN])
{
	return ((uint32_t)data[0] << 24) |
	       ((uint32_t)data[1] << 16) |
	       ((uint32_t)data[2] << 8) |
	       (uint32_t)data[3];
}

static uint32_t bytes_to_u32_le(const unsigned char data[RFID_CODE_LEN])
{
	return ((uint32_t)data[3] << 24) |
	       ((uint32_t)data[2] << 16) |
	       ((uint32_t)data[1] << 8) |
	       (uint32_t)data[0];
}

static void print_rfid_code(const unsigned char data[RFID_CODE_LEN])
{
	printf("card: raw=%02X %02X %02X %02X hex=%02X%02X%02X%02X be=%u le=%u\n",
	       data[0], data[1], data[2], data[3],
	       data[0], data[1], data[2], data[3],
	       bytes_to_u32_be(data), bytes_to_u32_le(data));
	fflush(stdout);
}

int main(void)
{
	int fd;

	fd = open(RFID_DEV_PATH, O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "open %s failed: %s\n", RFID_DEV_PATH, strerror(errno));
		return errno;
	}

	printf("reading %s\n", RFID_DEV_PATH);
	fflush(stdout);

	while (1) {
		unsigned char code[RFID_CODE_LEN] = {0};
		ssize_t ret = read(fd, code, sizeof(code));

		if (ret == RFID_CODE_LEN) {
			print_rfid_code(code);
			continue;
		}

		if (ret < 0) {
			if (errno == EINTR)
				continue;
			fprintf(stderr, "read %s failed: %s\n", RFID_DEV_PATH, strerror(errno));
			close(fd);
			return errno;
		}

		if (ret > 0)
			fprintf(stderr, "short read: %ld bytes\n", (long)ret);

		usleep(DEFAULT_INTERVAL_MS * 1000);
	}
}
