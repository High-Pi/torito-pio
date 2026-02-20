#ifndef LORA_CONFIG_H_ESP
#define LORA_CONFIG_H_ESP

// LoRa Module Baud Configuration
#define LORA_BAUD_ESP 115200

// LoRa Network Configuration
#define LORA_BAND_ESP 928000000      // 928 MHz for US
#define LORA_NETWORK_ID_ESP 18        // Must match on all devices
#define LORA_SENDER_ADDRESS_ESP 3    // Address for sender
#define LORA_RECEIVER_ADDRESS_ESP 2  // Address for receiver


// AT Command Templates
#define AT_TEST_ESP "AT"
#define AT_SET_ADDRESS_FMT_ESP "AT+ADDRESS=%d"
#define AT_SET_BAND_FMT_ESP "AT+BAND=%lu"
#define AT_SET_NETWORK_FMT_ESP "AT+NETWORKID=%d"
#define AT_SEND_FMT_ESP "AT+SEND=%d,%d,%s"  // dest, length, data
#define AT_SET_PARAMETER_FMT_ESP "AT+PARAMETER=%d,%d,%d,%d"  // SF, BW, CR, preamble

// Default LoRa PHY parameter set (used by LoRaModule::configure)
#define LORA_PARAMETER_SF_ESP 11
#define LORA_PARAMETER_BW_ESP 9
#define LORA_PARAMETER_CR_ESP 4
#define LORA_PARAMETER_PREAMBLE_ESP 24
#define LORA_PARAMETER_DEFAULT_STR_ESP "11,9,4,24"

// Timing Configuration
#define AT_COMMAND_TIMEOUT_ESP 1000
#define LORA_INIT_DELAY_ESP 1000
#define LORA_CONFIG_DELAY_ESP 100

// Relay Configuration (for receiver)
#define RELAY_MSB_BIT_ESP 0x8000     // Bit 15 must be set for valid command
#define RELAY_BIT_START_ESP 9        // Relays start at bit 9

#endif
