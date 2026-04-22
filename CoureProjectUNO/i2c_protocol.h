#ifndef I2C_PROTOCOL_H
#define I2C_PROTOCOL_H

// --- I2C SLAVE ADDRESS ---
#define UNO_I2C_ADDRESS   0x08

// --- COMMAND PROTOCOL (Master -> Slave) ---
#define CMD_OBSTACLE_ON   0x30
#define CMD_OBSTACLE_OFF  0x31
#define CMD_BUZZER_START  0x40
#define CMD_BUZZER_STOP   0x41

// --- STATUS PROTOCOL (Slave -> Master) ---
#define STATUS_CLEAR      0x00
#define STATUS_OBSTACLE   0x01

#endif
