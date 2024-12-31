#include <inttypes.h>
#include <pico/stdlib.h>
#include <stdio.h>
#include <string.h>

#include "joybus.h"

typedef struct
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
}

static uint8_t __time_critical_func(joybus_callback)(uint8_t ch, uint8_t cmd, uint8_t rx_length, uint8_t *rx_buffer, uint8_t *tx_buffer)
{
    uint8_t tx_length = 0;

    static uint8_t rx_index = 0;
    static uint8_t tx_index = 0;

    if (ch > 0)
    {
        return 0;
    }

    switch (cmd)
    {
    case JOYBUS_CMD_RESET:
        // fallthrough
    case JOYBUS_CMD_INFO:
        if (rx_length == 0)
        {
            uint32_t info = 0xBB6400;

            tx_length = 3;
            tx_buffer[0] = ((info >> 16) & 0xFF);
            tx_buffer[1] = ((info >> 8) & 0xFF);
            tx_buffer[2] = (info & 0xFF);
        }
        break;

    case JOYBUS_CMD_TXRX:
        if (rx_length == 2)
        {
            tx_length = 2;

            const uint8_t byte = rx_buffer[0];
            const uint8_t index = rx_buffer[1];

            if (rx_index != index)
            {
                add_to_queue(&rx_queue, byte);

                rx_index = index;
            }

            const int resp = get_from_queue(&tx_queue);
            if (resp >= 0)
            {
                tx_index++;
                tx_buffer[0] = resp;
            }
            tx_buffer[1] = tx_index;
        }
        break;

    default:
        printf("Unknown command received: 0x%02X, length: %d\n", cmd, rx_length);
        for (unsigned int i = 0; i < rx_length; i++)
        {
            printf("%02X ", rx_buffer[i]);
        }
        printf("\n");
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
        const int ch = getchar_timeout_us(0);
        if (ch != PICO_ERROR_TIMEOUT)
        {
            add_to_queue(&tx_queue, ch);
        }

        const int tx = get_from_queue(&rx_queue);
        if (tx >= 0)
        {
            putchar(tx);
        }
    }
}
