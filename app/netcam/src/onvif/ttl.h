#ifndef TTL_H
#define TTL_H


#ifdef __cplusplus
extern "C" {
#endif
void print_hex_array(int len, unsigned char* arr);
int init_uart(int num);
void do_transmit(int uart_fd, int send_bytes, unsigned char *p_tx_buffer);
int checksum(int size, unsigned char* cur_line);
void send_absolutemove_value(int uart_fd, float panPosition, float tiltPosition, float zoomPosition);
void on_receive(int uart_fd);
void close_uart(int uart_fs);

void *rx_receive(void *uart);
void *tx_send(void *uart);
void init_af_send(int uart);
void check_data(unsigned char* rx_buf);
void *begin_485();
uint32_t pelco_set_preset(unsigned char num);
uint32_t pelco_call_preset(unsigned char num);
//uint32_t pelco_set_stop();
//uint32_t pelco_set_up(unsigned char speed);
//uint32_t pelco_set_down(unsigned char speed);
//uint32_t pelco_set_left(unsigned char speed);
//uint32_t pelco_set_right(unsigned char speed);
//uint32_t pelco_set_left_top(unsigned char pan_speed, unsigned char tilt_speed);
//uint32_t pelco_set_right_top(unsigned char pan_speed, unsigned char tilt_speed);
//uint32_t pelco_set_left_bottom(unsigned char pan_speed, unsigned char tilt_speed);
//uint32_t pelco_set_right_bottom(unsigned char pan_speed, unsigned char tilt_speed);



#ifdef __cplusplus
}
#endif


#endif

