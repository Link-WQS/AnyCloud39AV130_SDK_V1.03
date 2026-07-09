#ifndef _AK_AMP_H_
#define _AK_AMP_H_

#include "ak_common.h"
#include "ak_common_audio.h"

enum ak_amp_msg_error_type
{
    ERROR_AMP_MSG_OPEN_LIB_ERROR = (MODULE_ID_AMP_MSG << 24) + 0,
    ERROR_AMP_MSG_WRITE_ERROR,
    ERROR_AMP_MSG_READ_ERROR,
    ERROR_AMP_MSG_OPEN_ERROR,
    ERROR_AMP_MSG_CLOSE_ERROR
};

struct rpmsg_ctrl_msg_ack {
    unsigned int id;
    unsigned int ack;
    unsigned int ret;// return value
};

/**
 * @brief ak_amp_msg_get_version - get AMP message library version string
 * @return: version string pointer
 * @notes: returns the version information of the AMP message library
 */
const char* ak_amp_msg_get_version(void);

/**
 * @brief ak_amp_msg_enable - initialize and open rpmsg device
 * @return: AK_SUCCESS on success, AK_FAILED on failure
 * @notes: 
 *     - This function opens the rpmsg device and stores the file descriptor
 *     - Must be called before any other rpmsg operations
 *     - The file descriptor is stored in a static variable rpmsg_fd
 */
int ak_amp_msg_enable(void);

/**
 * @brief ak_amp_msg_disable - close rpmsg device
 * @return: AK_SUCCESS on success, AK_FAILED on failure
 * @notes: 
 *     - This function closes the rpmsg device
 *     - Should be called when rpmsg is no longer needed
 *     - Uses the static rpmsg_fd stored by ak_amp_msg_enable()
 */
int ak_amp_msg_disable(void);

/**
 * @brief ak_amp_msg_create_ch - create a rpmsg communication channel
 * @param[out] amp_ch_fd: pointer to store the created channel file descriptor
 * @param[in] channel_id: the ID of the rpmsg channel to create
 * @return: AK_SUCCESS on success, AK_FAILED on failure
 * @notes: 
 *     - Creates a communication channel for rpmsg data transfer
 *     - The channel_id identifies the specific channel to create
 *     - amp_ch_fd will be set to -1 if channel creation fails
 *     - Must call ak_amp_msg_enable() first to initialize rpmsg device
 */
int ak_amp_msg_create_ch(int *amp_ch_fd, int channel_id);

/**
 * @brief ak_amp_msg_destroy_ch - destroy a rpmsg communication channel
 * @param[in] amp_ch_fd: the channel file descriptor to destroy
 * @return: 0 on success, non-zero on failure
 * @notes: 
 *     - Closes and releases the specified rpmsg channel
 *     - After calling this function, the channel fd is no longer valid
 */
int ak_amp_msg_destroy_ch(int amp_ch_fd);

/**
 * @brief ak_amp_msg_read - read data from rpmsg channel
 * @param[in] amp_ch_fd: the channel file descriptor to read from
 * @param[out] cmd_id: pointer to store the command ID extracted from message
 * @param[out] data: buffer to store the received data (excluding cmd_id)
 * @param[out] data_len: pointer to store the received data length
 * @return: AK_SUCCESS on success
 * @notes: 
 *     - Reads raw data from rpmsg channel
 *     - The first 4 bytes of the message contain the command ID
 *     - The remaining bytes are copied to the data buffer
 *     - data_len is updated to reflect the actual data length (excluding cmd_id)
 *     - Maximum message size is 512 bytes
 */
int ak_amp_msg_read(int amp_ch_fd, int *cmd_id, unsigned char *data, unsigned int *data_len);

/**
 * @brief ak_amp_msg_write - send command with data and wait for ACK reply
 * @param[in] amp_ch_fd: the channel file descriptor to use
 * @param[in] cmd_id: command ID to send
 * @param[in] data: buffer containing data to send
 * @param[in] data_len: length of data to send
 * @return: AK_SUCCESS on success, error code on failure
 * @retval AK_SUCCESS: operation successful
 * @retval ERROR_TYPE_INVALID_ARG: data_len exceeds maximum (508 bytes)
 * @retval ERROR_AMP_MSG_WRITE_ERROR: failed to write to rpmsg channel
 * @retval ERROR_AMP_MSG_READ_ERROR: failed to read ACK reply
 * @notes: 
 *     - Sends a command with data to the remote core
 *     - Waits for ACK reply from the remote core
 *     - Maximum data length is 508 bytes (512 - 4 bytes for cmd_id)
 *     - The first 4 bytes of the message contain the command ID (little-endian)
 */
int ak_amp_msg_write(int amp_ch_fd, int cmd_id, unsigned char *data, unsigned int data_len);

/**
 * @brief ak_amp_msg_get_param - send command and wait for reply with data
 * @param[in] amp_ch_fd: the channel file descriptor to use
 * @param[in] cmd_id: command ID to send
 * @param[in,out] data: buffer containing data to send, and stores received reply data
 * @param[in,out] data_len: pointer to data length (input: send length, output: reply length)
 * @return: AK_SUCCESS on success, error code on failure
 * @retval AK_SUCCESS: operation successful
 * @retval ERROR_TYPE_INVALID_ARG: data_len exceeds maximum (508 bytes)
 * @retval ERROR_AMP_MSG_WRITE_ERROR: failed to write to rpmsg channel
 * @retval ERROR_AMP_MSG_READ_ERROR: failed to read reply from rpmsg channel
 * @notes: 
 *     - Sends a command with data and waits for a reply
 *     - Maximum data length is 508 bytes (512 - 4 bytes for cmd_id)
 *     - If reply contains ACK command, prints acknowledgment info
 *     - If reply matches the sent cmd_id, copies reply data to output buffer
 */
int ak_amp_msg_get_param(int amp_ch_fd, int cmd_id, unsigned char *data, unsigned int *data_len);

#endif
/* end of file */
