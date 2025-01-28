#include <inttypes.h>
#include <pico/stdlib.h>
#include <stdio.h>
#include <string.h>

#include "joybus.h"
#include "data.h"

/*typedef struct
{
    uint8_t start;
    uint8_t end;
    bool full;
    bool empty;
    uint8_t data[256];
} Queue;

static Queue tx_queue = {.start = 0, .end = 0, .full = false, .empty = true, .data = {0}};
static Queue rx_queue = {.start = 0, .end = 0, .full = false, .empty = true, .data = {0}};

static bool add_to_queue(Queue *queue, uint8_t byte)
{
    if (queue->full)
    {
        return false;
    }

    queue->data[queue->end] = byte;
    queue->end++;
    if (queue->end == queue->start)
    {
        queue->full = true;
    }

    queue->empty = false;

    return true;
}

static int get_from_queue(Queue *queue)
{
    if (queue->empty)
    {
        return -1;
    }

    const uint8_t byte = queue->data[queue->start];
    queue->start++;
    if (queue->end == queue->start)
    {
        queue->empty = true;
    }

    queue->full = false;

    return byte;
}*/

static volatile uint32_t controller_state = 0;
static volatile bool rumble_on = false;
static enum { ACC_NONE,
              ACC_CPAK,
              ACC_RPAK } accessory_inserted = ACC_CPAK;

static uint8_t __time_critical_func(joybus_callback)(uint8_t ch, uint8_t cmd, uint8_t rx_length, uint8_t *rx_buffer, uint8_t *tx_buffer)
{
    uint8_t tx_length = 0;
    uint16_t address_with_crc = (rx_buffer[0] << 8) | rx_buffer[1];
    uint16_t address = address_with_crc & 0xFFE0;
    static uint8_t bank = 0;

    if (ch > 0)
    {
        return 0;
    }

    switch (cmd)
    {
    case JOYBUS_CMD_INFO:
    case JOYBUS_CMD_RESET:
        if (rx_length == 0)
        {
            uint32_t info = 0x050000 | ((accessory_inserted == ACC_NONE) ? 0x02 : 0x01);

            tx_length = 3;
            tx_buffer[0] = ((info >> 16) & 0xFF);
            tx_buffer[1] = ((info >> 8) & 0xFF);
            tx_buffer[2] = (info & 0xFF);

            bank = 0;
        }
        break;

    case JOYBUS_CMD_STATE:
        if (rx_length == 0)
        {
            tx_length = 4;
            tx_buffer[0] = ((controller_state >> 24) & 0xFF);
            tx_buffer[1] = ((controller_state >> 16) & 0xFF);
            tx_buffer[2] = ((controller_state >> 8) & 0xFF);
            tx_buffer[3] = (controller_state & 0xFF);
        }
        break;

    case JOYBUS_CMD_READ:
        if ((rx_length == 2) && joybus_check_address_crc(address_with_crc))
        {
            tx_length = 33;
            switch (accessory_inserted)
            {
            case ACC_NONE:
                memset(tx_buffer, 0x00, 32);
                break;

            case ACC_CPAK:
                if (address < 32 * 1024)
                {
                    memcpy(tx_buffer, controller_pak + address + bank * 0x8000, 32);
                }
                else
                {
                    memset(tx_buffer, 0x00, 32);
                }
                break;

            case ACC_RPAK:
                memset(tx_buffer, (address & 0xC000) == 0x8000 ? 0x80 : 0x00, 32);
                break;
            }
            tx_buffer[32] = joybus_calculate_data_crc(tx_buffer);
        }
        break;

    case JOYBUS_CMD_WRITE:
        if ((rx_length == 34) && joybus_check_address_crc(address_with_crc))
        {
            switch (accessory_inserted)
            {
            case ACC_NONE:
                break;

            case ACC_CPAK:
                if (address < 32 * 1024)
                {
                    memcpy(controller_pak + address + bank * 0x8000, rx_buffer + 2, 32);
                }
                else
                {
                    bank = rx_buffer[2];
                }
                break;

            case ACC_RPAK:
                if ((address & 0xC000) == 0xC000)
                {
                    rumble_on = rx_buffer[33] & 0x01;
                }
                break;
            }

            tx_length = 1;
            tx_buffer[0] = joybus_calculate_data_crc(rx_buffer + 2);
        }
        break;

    default:
        printf("Unknown command received: 0x%02X, length: %d\n", cmd, rx_length);
        // printf_hexdump(rx_buffer, rx_length);
        break;
    }

    return tx_length;
}

int main(void)
{
    stdio_init_all();

    sleep_ms(1000);

    // printf("hello\n");

    uint joybus_pins[1] = {28};
    joybus_init(pio1, 1, joybus_pins, joybus_callback);

    // printf("joybus on pin %d\n", joybus_pins[0]);

    while (true)
    {
        /*const int ch = getchar_timeout_us(0);
        if (ch != PICO_ERROR_TIMEOUT)
        {
            add_to_queue(&tx_queue, ch);
        }

        const int tx = get_from_queue(&rx_queue);
        if (tx >= 0)
        {
            putchar(tx);
        }*/
    }
}
