#ifndef PROTOCOL_H
#define PROTOCOL_H

typedef enum tag_key {
	LP_SAMPLE_RATE=0x02,
	LP_COMM_INTERVAL=0x02,
	LP_SOURCE_MAC=0x06,
	LP_HOST_TIME=0x07,
	LP_DEVICE_NAME=0x08,
	LP_SEQ_NO=0x0f,
	LP_SAMPLES_COUNT=0x11,
	LP_TIME_OFFSET=0x15,
	LP_DEVICE_TIME=0x16,
	LP_ALARM_DELAY=0x1e,
	LP_FAHRENHEIT=0x90,
} key;

typedef enum tag_field {
	LP_BOOL=0x02,
	LP_RECORD=0x20,
	LP_VALUE=0x80,

} field;

typedef enum tag_datatype {
	LP_CHAR=0x01,
	LP_SHORTINT=0x03,
	LP_UNIXTIME=0x05,
	LP_STRING=0x0b,
	LP_MACADDR=0x0f,
} datatype;


typedef struct tag_value_descriptor {
	key k;
	field f;
	datatype t;
} __attribute__((__packed__)) value_descriptor;


void parse_header(char *buf, int *pos, unsigned char *dest_mac, unsigned char *seqno, unsigned char *battery_status);
void parse_value(char *buf, int *pos, value_descriptor **desc, void **value, int *value_size);

#endif
