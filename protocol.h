#ifndef PROTOCOL_H
#define PROTOCOL_H

typedef enum tag_key {
	LP_SOMETHING = 0x00,
	LP_SAMPLE_RATE = 0x02,
	LP_COMM_INTERVAL = 0x03,
	LP_SOURCE_MAC = 0x06,
	LP_HOST_TIME = 0x07,
	LP_DEVICE_NAME = 0x08,
	LP_KEY09 = 0x09,
	LP_SEQ_NO = 0x0f,
	LP_SAMPLES_COUNT = 0x11,
	LP_TIME_OFFSET = 0x15,
	LP_DEVICE_TIME = 0x16,
	LP_ALARM_DELAY = 0x1e,
	LP_KEY50 = 0x50,
	LP_KEY60 = 0x60,
	LP_FAHRENHEIT = 0x90,
	LP_LAST
} __attribute__((__packed__)) key;

typedef enum tag_field {
	LP_BOOL = 0x02,
	LP_FIELD06 = 0x06,
	LP_RECORD = 0x20,
	LP_VALUE = 0x80,

} __attribute__((__packed__)) field;

typedef enum tag_datatype {
	LP_CHAR = 0x01,
	LP_SHORTINT = 0x03,
	LP_UNIXTIME = 0x05,
	LP_STRING = 0x0b,
	LP_TYPE0C = 0x0c,
	LP_MACADDR = 0x0f,
	LP_TYPE63 = 0x63,
} __attribute__((__packed__)) datatype;


typedef struct tag_value_descriptor {
	key k;
	field f;
	datatype t;
} __attribute__((__packed__)) value_descriptor;

typedef struct hash_item {field f; datatype t; void *v;} __attribute__((__packed__)) hash[LP_LAST];

void parse_header(char *buf, int *pos, unsigned char *dest_mac, unsigned char *seqno, unsigned char *battery_status);
void parse_values(char *buf, size_t buflen, int *pos, hash *h);
void print_hash(hash *h);

#endif
