#include "sensor_i2c.h"
#include <linux/i2c.h>

typedef int (*i2c_write_type)(
    const struct i2c_client* client, int reg, int value);
typedef int (*i2c_read_type)(const struct i2c_client* client, int reg);


extern int ak_i2c_xfer_in_irqctx(struct i2c_adapter *adap,
                       struct i2c_msg *msgs, int num);

#if 1 //ref to i2c-core.c,allow i2c bus transfer in interrupt context
/**
 * i2c_transfer - execute a single or combined I2C message
 * @adap: Handle to I2C bus
 * @msgs: One or more messages to execute before STOP is issued to
 *	terminate the operation; each message begins with a START.
 * @num: Number of messages to be executed.
 *
 * Returns negative errno, else the number of messages executed.
 *
 * Note that there is no requirement that each message be sent to
 * the same slave address, although that is the most common model.
 */
int i2c_transfer_in_irqctx(struct i2c_adapter *adap,
        struct i2c_msg *msgs, int num)
{
    int ret;


    if (adap->algo->master_xfer) {
        ret = ak_i2c_xfer_in_irqctx(adap, msgs, num);
        return ret;
    } else {
        dev_dbg(&adap->dev, "I2C level transfers not supported\n");
        return -EOPNOTSUPP;
    }
}
/**
 * i2c_master_send - issue a single I2C message in master transmit mode
 * @client: Handle to slave device
 * @buf: Data that will be written to the slave
 * @count: How many bytes to write, must be less than 64k since msg.len is u16
 *
 * Returns negative errno, or else the number of bytes written.
 */
int i2c_master_send_in_irq(const struct i2c_client *client,
        const char *buf, int count)
{
    int ret;
    struct i2c_adapter *adap = client->adapter;
    struct i2c_msg msg;

    msg.addr = client->addr;
    msg.flags = client->flags & I2C_M_TEN;
    msg.len = count;
    msg.buf = (char *)buf;

    ret = i2c_transfer_in_irqctx(adap, &msg, 1);

    /*
     * If everything went ok (i.e. 1 msg transmitted), return #bytes
     * transmitted, else error code.
     */
    return (ret == 1) ? count : ret;
}




/**
 * i2c_master_recv - issue a single I2C message in master receive mode
 * @client: Handle to slave device
 * @buf: Where to store data read from slave
 * @count: How many bytes to read, must be less than 64k since msg.len is u16
 *
 * Returns negative errno, or else the number of bytes read.
 */
int i2c_master_recv_in_irq(
        const struct i2c_client *client, char *buf, int count)
{
    struct i2c_adapter *adap = client->adapter;
    struct i2c_msg msg;
    int ret;

    msg.addr = client->addr;
    msg.flags = client->flags & I2C_M_TEN;
    msg.flags |= I2C_M_RD;
    msg.len = count;
    msg.buf = buf;

    ret = i2c_transfer_in_irqctx(adap, &msg, 1);

    /*
     * If everything went ok (i.e. 1 msg received), return #bytes received,
     * else error code.
     */
    return (ret == 1) ? count : ret;
}
#endif //end of irq flavor







/*******  ref to i2c-core.c,start ************/
#define POLY    (0x1070U << 3)
static u8 crc8(u16 data)
{
    int i;

    for (i = 0; i < 8; i++) {
        if (data & 0x8000)
            data = data ^ POLY;
        data = data << 1;
    }
    return (u8)(data >> 8);
}

/* Incremental CRC8 over count bytes in the array pointed to by p */
static u8 i2c_smbus_pec(u8 crc, u8 *p, size_t count)
{
    int i;

    for (i = 0; i < count; i++)
        crc = crc8((crc ^ p[i]) << 8);
    return crc;
}

/* Assume a 7-bit address, which is reasonable for SMBus */
static u8 i2c_smbus_msg_pec(u8 pec, struct i2c_msg *msg)
{
    /* The address will be sent first */
    u8 addr = (msg->addr << 1) | !!(msg->flags & I2C_M_RD);
    pec = i2c_smbus_pec(pec, &addr, 1);

    /* The data buffer follows */
    return i2c_smbus_pec(pec, msg->buf, msg->len);
}

/* Used for write only transactions */
static inline void i2c_smbus_add_pec(struct i2c_msg *msg)
{
    msg->buf[msg->len] = i2c_smbus_msg_pec(0, msg);
    msg->len++;
}
/* Return <0 on CRC error
   If there was a write before this read (most cases) we need to take the
   partial CRC from the write part into account.
   Note that this function does modify the message (we need to decrease the
   message length to hide the CRC byte from the caller). */
static int i2c_smbus_check_pec(u8 cpec, struct i2c_msg *msg)
{
    u8 rpec = msg->buf[--msg->len];
    cpec = i2c_smbus_msg_pec(cpec, msg);

    if (rpec != cpec) {
        pr_debug("i2c-core: Bad PEC 0x%02x vs. 0x%02x\n",
            rpec, cpec);
        return -EBADMSG;
    }
    return 0;
}


/* Simulate a SMBus command using the i2c protocol
   No checking of parameters is done!  */
static s32 i2c_smbus_xfer_emulated_in_irq(struct i2c_adapter *adapter, u16 addr,
                   unsigned short flags,
                   char read_write, u8 command, int size,
                   union i2c_smbus_data *data)
{
    /* So we need to generate a series of msgs. In the case of writing, we
      need to use only one message; when reading, we need two. We initialize
      most things with sane defaults, to keep the code below somewhat
      simpler. */
    unsigned char msgbuf0[I2C_SMBUS_BLOCK_MAX+3];
    unsigned char msgbuf1[I2C_SMBUS_BLOCK_MAX+2];
    int num = read_write == I2C_SMBUS_READ ? 2 : 1;
    int i;
    u8 partial_pec = 0;
    int status;
    struct i2c_msg msg[2] = {
        {
            .addr = addr,
            .flags = flags,
            .len = 1,
            .buf = msgbuf0,
        }, {
            .addr = addr,
            .flags = flags | I2C_M_RD,
            .len = 0,
            .buf = msgbuf1,
        },
    };

    msgbuf0[0] = command;
    switch (size) {
    case I2C_SMBUS_QUICK:
        msg[0].len = 0;
        /* Special case: The read/write field is used as data */
        msg[0].flags = flags | (read_write == I2C_SMBUS_READ ?
                    I2C_M_RD : 0);
        num = 1;
        break;
    case I2C_SMBUS_BYTE:
        if (read_write == I2C_SMBUS_READ) {
            /* Special case: only a read! */
            msg[0].flags = I2C_M_RD | flags;
            num = 1;
        }
        break;
    case I2C_SMBUS_BYTE_DATA:
        if (read_write == I2C_SMBUS_READ)
            msg[1].len = 1;
        else {
            msg[0].len = 2;
            msgbuf0[1] = data->byte;
        }
        break;
    case I2C_SMBUS_WORD_DATA:
        if (read_write == I2C_SMBUS_READ)
            msg[1].len = 2;
        else {
            msg[0].len = 3;
            msgbuf0[1] = data->word & 0xff;
            msgbuf0[2] = data->word >> 8;
        }
        break;
    case I2C_SMBUS_PROC_CALL:
        num = 2; /* Special case */
        read_write = I2C_SMBUS_READ;
        msg[0].len = 3;
        msg[1].len = 2;
        msgbuf0[1] = data->word & 0xff;
        msgbuf0[2] = data->word >> 8;
        break;
    case I2C_SMBUS_BLOCK_DATA:
        if (read_write == I2C_SMBUS_READ) {
            msg[1].flags |= I2C_M_RECV_LEN;
            msg[1].len = 1; /* block length will be added by
                       the underlying bus driver */
        } else {
            msg[0].len = data->block[0] + 2;
            if (msg[0].len > I2C_SMBUS_BLOCK_MAX + 2) {
                dev_err(&adapter->dev,
                    "Invalid block write size %d\n",
                    data->block[0]);
                return -EINVAL;
            }
            for (i = 1; i < msg[0].len; i++)
                msgbuf0[i] = data->block[i-1];
        }
        break;
    case I2C_SMBUS_BLOCK_PROC_CALL:
        num = 2; /* Another special case */
        read_write = I2C_SMBUS_READ;
        if (data->block[0] > I2C_SMBUS_BLOCK_MAX) {
            dev_err(&adapter->dev,
                "Invalid block write size %d\n",
                data->block[0]);
            return -EINVAL;
        }
        msg[0].len = data->block[0] + 2;
        for (i = 1; i < msg[0].len; i++)
            msgbuf0[i] = data->block[i-1];
        msg[1].flags |= I2C_M_RECV_LEN;
        msg[1].len = 1; /* block length will be added by
                   the underlying bus driver */
        break;
    case I2C_SMBUS_I2C_BLOCK_DATA:
        if (data->block[0] > I2C_SMBUS_BLOCK_MAX) {
            dev_err(&adapter->dev, "Invalid block %s size %d\n",
                read_write == I2C_SMBUS_READ ? "read" : "write",
                data->block[0]);
            return -EINVAL;
        }
        if (read_write == I2C_SMBUS_READ) {
            msg[1].len = data->block[0];
        } else {
            msg[0].len = data->block[0] + 1;
            for (i = 1; i <= data->block[0]; i++)
                msgbuf0[i] = data->block[i];
        }
        break;
    default:
        dev_err(&adapter->dev, "Unsupported transaction %d\n", size);
        return -EOPNOTSUPP;
    }

    i = ((flags & I2C_CLIENT_PEC) && size != I2C_SMBUS_QUICK
                      && size != I2C_SMBUS_I2C_BLOCK_DATA);
    if (i) {
        /* Compute PEC if first message is a write */
        if (!(msg[0].flags & I2C_M_RD)) {
            if (num == 1) /* Write only */
                i2c_smbus_add_pec(&msg[0]);
            else /* Write followed by read */
                partial_pec = i2c_smbus_msg_pec(0, &msg[0]);
        }
        /* Ask for PEC if last message is a read */
        if (msg[num-1].flags & I2C_M_RD)
            msg[num-1].len++;
    }

    status = ak_i2c_xfer_in_irqctx(adapter, msg, num);
    if (status < 0)
        return status;

    /* Check PEC if last message is a read */
    if (i && (msg[num-1].flags & I2C_M_RD)) {
        status = i2c_smbus_check_pec(partial_pec, &msg[num-1]);
        if (status < 0)
            return status;
    }

    if (read_write == I2C_SMBUS_READ)
        switch (size) {
        case I2C_SMBUS_BYTE:
            data->byte = msgbuf0[0];
            break;
        case I2C_SMBUS_BYTE_DATA:
            data->byte = msgbuf1[0];
            break;
        case I2C_SMBUS_WORD_DATA:
        case I2C_SMBUS_PROC_CALL:
            data->word = msgbuf1[0] | (msgbuf1[1] << 8);
            break;
        case I2C_SMBUS_I2C_BLOCK_DATA:
            for (i = 0; i < data->block[0]; i++)
                data->block[i+1] = msgbuf1[i];
            break;
        case I2C_SMBUS_BLOCK_DATA:
        case I2C_SMBUS_BLOCK_PROC_CALL:
            for (i = 0; i < msgbuf1[0] + 1; i++)
                data->block[i] = msgbuf1[i];
            break;
        }
    return 0;
}
/**end of i2c_smbus_xfer_emulated_in_irq ***/
/**
 * i2c_smbus_xfer_in_irq - execute SMBus protocol operations
 * @adapter: Handle to I2C bus
 * @addr: Address of SMBus slave on that bus
 * @flags: I2C_CLIENT_* flags (usually zero or I2C_CLIENT_PEC)
 * @read_write: I2C_SMBUS_READ or I2C_SMBUS_WRITE
 * @command: Byte interpreted by slave, for protocols which use such bytes
 * @protocol: SMBus protocol operation to execute, such as I2C_SMBUS_PROC_CALL
 * @data: Data to be read or written
 *
 * This executes an SMBus protocol operation, and returns a negative
 * errno code else zero on success.
 */
s32 i2c_smbus_xfer_in_irq(struct i2c_adapter *adapter, u16 addr, unsigned short flags,
           char read_write, u8 command, int protocol,
           union i2c_smbus_data *data)
{
    s32 res;


    flags &= I2C_M_TEN | I2C_CLIENT_PEC | I2C_CLIENT_SCCB;


    res = i2c_smbus_xfer_emulated_in_irq(adapter, addr, flags, read_write,
                      command, protocol, data);


    return res;
}
/**
 * i2c_smbus_write_byte_data - SMBus "write byte" protocol
 * @client: Handle to slave device
 * @command: Byte interpreted by slave
 * @value: Byte being written
 *
 * This executes the SMBus "write byte" protocol, returning negative errno
 * else zero on success.
 */
s32 i2c_smbus_write_byte_data_in_irq(const struct i2c_client *client, u8 command,
                  u8 value)
{
    union i2c_smbus_data data;
    data.byte = value;
    return i2c_smbus_xfer_in_irq(client->adapter, client->addr, client->flags,
                  I2C_SMBUS_WRITE, command,
                  I2C_SMBUS_BYTE_DATA, &data);
}
/**
 * i2c_smbus_read_byte_data_in_irq - SMBus "read byte" protocol
 * @client: Handle to slave device
 * @command: Byte interpreted by slave
 *
 * This executes the SMBus "read byte" protocol, returning negative errno
 * else a data byte received from the device.
 */
s32 i2c_smbus_read_byte_data_in_irq(const struct i2c_client *client, u8 command)
{
    union i2c_smbus_data data;
    int status;

    status = i2c_smbus_xfer_in_irq(client->adapter, client->addr, client->flags,
                I2C_SMBUS_READ, command,
                I2C_SMBUS_BYTE_DATA, &data);
    return (status < 0) ? status : data.byte;
}
/**
 * i2c_smbus_write_word_data_in_irq - SMBus "write word" protocol
 * @client: Handle to slave device
 * @command: Byte interpreted by slave
 * @value: 16-bit "word" being written
 *
 * This executes the SMBus "write word" protocol, returning negative errno
 * else zero on success.
 */
s32 i2c_smbus_write_word_data_in_irq(const struct i2c_client *client, u8 command,
                  u16 value)
{
    union i2c_smbus_data data;
    data.word = value;
    return i2c_smbus_xfer_in_irq(client->adapter, client->addr, client->flags,
                  I2C_SMBUS_WRITE, command,
                  I2C_SMBUS_WORD_DATA, &data);
}
/**
 * i2c_smbus_read_word_data_in_irq - SMBus "read word" protocol
 * @client: Handle to slave device
 * @command: Byte interpreted by slave
 *
 * This executes the SMBus "read word" protocol, returning negative errno
 * else a 16-bit unsigned "word" received from the device.
 */
s32 i2c_smbus_read_word_data_in_irq(const struct i2c_client *client, u8 command)
{
    union i2c_smbus_data data;
    int status;

    status = i2c_smbus_xfer_in_irq(client->adapter, client->addr, client->flags,
                I2C_SMBUS_READ, command,
                I2C_SMBUS_WORD_DATA, &data);
    return (status < 0) ? status : data.word;
}
/*************  end of i2c-core.c  ********************/




/*
 * i2c_write_reg2_value1_in_irq -
 * register 2 bytes and value 1 byte to write
 *
 * @client[IN]:         pointer to i2c client
 * @reg[IN]:            register
 * @value[IN]:          value to write
 *
 * RETURN:  0-success, others-fail
 */
static int i2c_write_reg2_value1_in_irq(
    const struct i2c_client* client, int reg, int value)
{
    unsigned char msg[3];

    msg[0] = reg >> 8;
    msg[1] = reg & 0xff;
    msg[2] = value;

    return i2c_master_send_in_irq(client, msg, 3);
}

/*
 * i2c_read_reg2_value1_in_irq -
 * register 2 bytes and value 1 byte to read
 *
 * @client[IN]:         pointer to i2c client
 * @reg[IN]:            register
 *
 * RETURN:  value to the register
 */
static int i2c_read_reg2_value1_in_irq(
        const struct i2c_client* client, int reg)
{
    int ret;
    unsigned char msg[2];
    unsigned char data;

    msg[0] = reg >> 8;
    msg[1] = reg & 0xff;

    ret = i2c_master_send_in_irq(client, msg, 2);
    if (ret < 0)
        return ret;

    ret = i2c_master_recv_in_irq(client, &data, 1);
    if (ret < 0)
        return ret;

    return data;
}

/*
 * i2c_write_reg2_value2_in_irq -
 * register 2 bytes and value 2 byte to write
 *
 * @client[IN]:         pointer to i2c client
 * @reg[IN]:            register
 * @value[IN]:          value to write
 *
 * RETURN:  0-success, others-fail
 */
static int i2c_write_reg2_value2_in_irq(
    const struct i2c_client* client, int reg, int value)
{
    unsigned char msg[4];

    msg[0] = reg >> 8; // high 8bit first send
    msg[1] = reg & 0xff; // low 8bit second send
    msg[2] = value >> 8; // high 8bit firt send
    msg[3] = value & 0xff; // low 8bit second send

    return i2c_master_send_in_irq(client, msg, 4);
}

/*
 * i2c_read_reg2_value2_in_irq -
 * register 2 bytes and value 2 byte to read
 *
 * @client[IN]:         pointer to i2c client
 * @reg[IN]:            register
 *
 * RETURN:  value to the register
 */
static int i2c_read_reg2_value2_in_irq(
        const struct i2c_client* client, int reg)
{
    int ret;
    unsigned char msg[4];
    unsigned char buf[2];

    msg[0] = reg >> 8; // high 8bit first send
    msg[1] = reg & 0xff; // low 8bit second send


    ret = i2c_master_send_in_irq(client, msg, 2);
    if (ret < 0)
        return ret;

    ret = i2c_master_recv_in_irq(client, buf, 2);
    if (ret < 0)
        return ret;

    return (buf[0] << 8) | buf[1];
}

/*
 * i2c_write_reg1_value1_in_irq -
 * register 1 bytes and value 1 byte to write
 *
 * @client[IN]:         pointer to i2c client
 * @reg[IN]:            register
 * @value[IN]:          value to write
 *
 * RETURN:  0-success, others-fail
 */
static int i2c_write_reg1_value1_in_irq(
    const struct i2c_client* client, int reg, int value)
{
    return i2c_smbus_write_byte_data_in_irq(client, reg, value);
}

/*
 * i2c_read_reg1_value1_in_irq -
 * register 1 bytes and value 1 byte to read
 *
 * @client[IN]:         pointer to i2c client
 * @reg[IN]:            register
 *
 * RETURN:  value to the register
 */
static int i2c_read_reg1_value1_in_irq(const struct i2c_client* client,
        int reg)
{
    return i2c_smbus_read_byte_data_in_irq(client, reg);
}


/*
 * i2c_read_reg1_value2_in_irq -
 * register 1 bytes and value 2 byte to read
 *
 * @client[IN]:         pointer to i2c client
 * @reg[IN]:            register
 *
 * RETURN:  value to the register
 */
static int i2c_read_reg1_value2_in_irq(const struct i2c_client* client,
        int reg)
{
    return i2c_smbus_read_word_data_in_irq(client, reg);
}

/*
 * i2c_write_reg1_value2_in_irq -
 * register 1 bytes and value 2 byte to write
 *
 * @client[IN]:         pointer to i2c client
 * @reg[IN]:            register
 * @value[IN]:          value to write
 *
 * RETURN:  0-success, others-fail
 */
static int i2c_write_reg1_value2_in_irq(
    const struct i2c_client* client, int reg, int value)
{
    return i2c_smbus_write_word_data_in_irq(client, reg, value);
}


/*all i2c write functions to a array*/
static i2c_write_type i2c_write_table_in_irq[2][2] = {
    {
        i2c_write_reg1_value1_in_irq,
        i2c_write_reg1_value2_in_irq,
    },
    {
        i2c_write_reg2_value1_in_irq,
        i2c_write_reg2_value2_in_irq,
    },
};

/*all i2c read functions to a array*/
static i2c_read_type i2c_read_table_in_irq[2][2] = {
    {
        i2c_read_reg1_value1_in_irq,
        i2c_read_reg1_value2_in_irq,
    },
    {
        i2c_read_reg2_value1_in_irq,
        i2c_read_reg2_value2_in_irq,
    },
};

/*
 * i2c_write -
 * i2c write entry
 *
 * @trans:          transfor paramters
 */
int i2c_write_support_in_irq(struct i2c_transfer_struct* trans)
{
    int reg_bytes;
    int value_bytes;
    i2c_write_type write;

    if (!trans)
        return -1;

    reg_bytes = trans->reg_bytes;
    value_bytes = trans->value_bytes;

    if (reg_bytes < 1 || reg_bytes > 2 || value_bytes < 1 || value_bytes > 2)
        return -1;

    write = i2c_write_table_in_irq[reg_bytes - 1][value_bytes - 1];

    return write(trans->client, trans->reg, trans->value);
}

/*
 * i2c_read -
 * i2c read entry
 *
 * @trans:          transfor paramters
 */
int i2c_read_support_in_irq(struct i2c_transfer_struct* trans)
{
    int reg_bytes;
    int value_bytes;
    i2c_read_type read;

    if (!trans)
        return -1;

    reg_bytes = trans->reg_bytes;
    value_bytes = trans->value_bytes;

    if (reg_bytes < 1 || reg_bytes > 2 || value_bytes < 1 || value_bytes > 2)
        return -1;

    read = i2c_read_table_in_irq[reg_bytes - 1][value_bytes - 1];

    return read(trans->client, trans->reg);
}

/*end of file*/
