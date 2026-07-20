#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include "PwmControl.h"

#define RFID_DEV_PATH "/dev/rfid_control"
#define RFID_CODE_LEN 4
#define DEFAULT_INTERVAL_MS 50
#define DUPLICATE_FILTER_MS 1000
#define DEVICE_READY_RETRY 40
#define RFID_PWM_DEV_NO 3
#define RFID_PWM_DUTY_NS 4000
#define RFID_PWM_PERIOD_NS 8000

static const char *const rfid_module_paths[] = {
	"/usr/modules/rfid.ko",
	"./rfid.ko",
	"rfid.ko",
};

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

static long diff_ms(const struct timespec *now, const struct timespec *prev)
{
	long sec = now->tv_sec - prev->tv_sec;
	long nsec = now->tv_nsec - prev->tv_nsec;

	return sec * 1000 + nsec / 1000000;
}

static int filter_duplicate_card(const unsigned char data[RFID_CODE_LEN])
{
	static unsigned char cached_code[RFID_CODE_LEN] = {0};
	static struct timespec cached_time = {0};
	struct timespec now;

	clock_gettime(CLOCK_MONOTONIC, &now);
	if (memcmp(cached_code, data, RFID_CODE_LEN) == 0 &&
	    (cached_time.tv_sec != 0 || cached_time.tv_nsec != 0) &&
	    diff_ms(&now, &cached_time) < DUPLICATE_FILTER_MS) {
		cached_time = now;
		return -1;
	}

	memcpy(cached_code, data, RFID_CODE_LEN);
	cached_time = now;
	return 0;
}

static int path_exists(const char *path)
{
	return access(path, F_OK) == 0;
}

static int ensure_rfid_pwm_ready(void)
{
	if (AkDrvPwmOpen(RFID_PWM_DEV_NO) != 0) {
		fprintf(stderr, "AkDrvPwmOpen(%d) failed\n", RFID_PWM_DEV_NO);
		return -1;
	}

	if (AkDrvPwmSet(RFID_PWM_DEV_NO, RFID_PWM_DUTY_NS, RFID_PWM_PERIOD_NS) != 0) {
		fprintf(stderr, "AkDrvPwmSet(%d, %d, %d) failed\n",
		        RFID_PWM_DEV_NO, RFID_PWM_DUTY_NS, RFID_PWM_PERIOD_NS);
		AkDrvPwmClose(RFID_PWM_DEV_NO);
		return -1;
	}

	return 0;
}

static int ensure_rfid_module_loaded(void)
{
	size_t i;

	if (path_exists(RFID_DEV_PATH))
		return 0;

	for (i = 0; i < sizeof(rfid_module_paths) / sizeof(rfid_module_paths[0]); ++i) {
		char cmd[256];

		if (!path_exists(rfid_module_paths[i]))
			continue;

		snprintf(cmd, sizeof(cmd), "insmod %s", rfid_module_paths[i]);
		if (system(cmd) == 0)
			return 0;

		fprintf(stderr, "insmod %s failed\n", rfid_module_paths[i]);
	}

	fprintf(stderr, "rfid module not found, expected one of:\n");
	for (i = 0; i < sizeof(rfid_module_paths) / sizeof(rfid_module_paths[0]); ++i)
		fprintf(stderr, "  %s\n", rfid_module_paths[i]);

	return -1;
}

static int wait_for_rfid_device(void)
{
	int retry;

	for (retry = 0; retry < DEVICE_READY_RETRY; ++retry) {
		if (path_exists(RFID_DEV_PATH))
			return 0;
		usleep(100 * 1000);
	}

	return -1;
}

static int open_rfid_device(void)
{
	int fd;

	if (ensure_rfid_pwm_ready() != 0)
		return -1;

	fd = open(RFID_DEV_PATH, O_RDONLY);
	if (fd >= 0)
		return fd;

	if (ensure_rfid_module_loaded() != 0)
		return -1;

	if (wait_for_rfid_device() != 0) {
		fprintf(stderr, "wait for %s ready timeout\n", RFID_DEV_PATH);
		return -1;
	}

	fd = open(RFID_DEV_PATH, O_RDONLY);
	if (fd < 0)
		fprintf(stderr, "open %s failed: %s\n", RFID_DEV_PATH, strerror(errno));

	return fd;
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

	fd = open_rfid_device();
	if (fd < 0)
		return errno ? errno : 1;

	printf("reading %s\n", RFID_DEV_PATH);
	fflush(stdout);

	while (1) {
		unsigned char code[RFID_CODE_LEN] = {0};
		ssize_t ret = read(fd, code, sizeof(code));

		if (ret == RFID_CODE_LEN) {
			if (filter_duplicate_card(code) == 0)
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
