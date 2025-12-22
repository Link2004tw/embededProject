# Password Initialization System - Implementation Guide

## Overview
This document describes the password initialization flag system that has been implemented in the embedded door lock project. The system now requires users to set their own password on first use instead of using a hardcoded default password.

## Key Changes

### 1. EEPROM Driver Updates (`BACK/EEPROM/`)

#### New Constants in `eepromDriver.h`:
```c
#define PASSWORD_FLAG_EEPROM_BLOCK    0U
#define PASSWORD_FLAG_EEPROM_OFFSET   2U
#define PASSWORD_INITIALIZED_MAGIC    0x5AA5C33C
```

#### New Functions:
- `EEPROM_IsPasswordInitialized()` - Checks if password has been set
- `EEPROM_SetPasswordInitialized()` - Marks password as initialized

### 2. Password Module Updates (`HAL/password/`)

#### Modified in `password.h`:
- Added `PASSWORD_NOT_INITIALIZED` return code
- Removed `PASSWORD_DEFAULT` constant (no more hardcoded password)
- Added `Password_IsInitialized()` function
- Added `Password_FirstTimeSetup()` function

#### Modified in `password.c`:
- `Password_Init()` now only verifies EEPROM access, does NOT set default password
- `Password_IsInitialized()` checks the EEPROM flag
- `Password_FirstTimeSetup()` allows first-time password setup by user

### 3. UART Communication Updates (`UART/HMI_Comm.c`)

#### New Communication Modes:

**Mode 3: First-Time Password Setup**
- Format: `3,XXXXX#` where XXXXX is a 5-digit password
- Response on success: `INIT_OK#`
- Response if already initialized: `ALREADY_INIT#`
- Response on error: `INIT_FAIL#` or `BAD_LENGTH#`

**Mode 4: Check Password Status**
- Format: `4#`
- Response: `INITIALIZED#` or `NOT_INITIALIZED#`

#### Modified Existing Modes:
- **Mode 0 (Open Door)** and **Mode 1 (Change Password)** now check if password is initialized
- If not initialized, they return: `NOT_INIT#`

### 4. Main Application Updates (`main.c`)

On startup, the system:
1. Initializes all peripherals
2. Checks password initialization status
3. If NOT initialized:
   - Sends `SETUP_REQUIRED#` via UART
   - Turns on RED LED (PF2) as visual indicator
4. If initialized:
   - Sends `READY#` via UART
   - Briefly blinks GREEN LED (PF3)

## EEPROM Memory Map

```
Block 0:
  Offset 0 (Word 0): Password bytes 0-3
  Offset 1 (Word 1): [Byte 0: Password byte 4] [Byte 1: Comma] [Byte 2: Timeout] [Byte 3: Reserved]
  Offset 2 (Word 2): Password Initialization Flag (0x5AA5C33C = initialized)
```

## Usage Flow

### First-Time Setup (Device Never Used Before)

1. **Power on the device**
   - Device sends: `SETUP_REQUIRED#`
   - Red LED turns on

2. **Check status** (Optional)
   - Send: `4#`
   - Receive: `NOT_INITIALIZED#`

3. **Initialize password**
   - Send: `3,12345#` (or any 5-digit password)
   - Receive: `INIT_OK#`
   - Red LED turns off (system ready)

4. **Password is now set and persistent**

### Normal Operation (Password Already Set)

1. **Power on**
   - Device sends: `READY#`
   - Green LED blinks briefly

2. **Open door**
   - Send: `0,12345#`
   - Receive: `Door$Unlocked#`

3. **Change password**
   - Send: `1,12345,67890#` (old password, new password)
   - Receive: `CHANGED#`

### Error Scenarios

**Attempting to open door before initialization:**
- Send: `0,12345#`
- Receive: `NOT_INIT#`

**Attempting to initialize when already initialized:**
- Send: `3,12345#`
- Receive: `ALREADY_INIT#`

**Attempting to initialize with wrong length:**
- Send: `3,123#` (less than 5 digits)
- Receive: `BAD_LENGTH#`

## Protocol Summary

### UART Message Format
All messages end with `#` delimiter. Format: `MODE,PARAM1,PARAM2#`

### Complete Mode List

| Mode | Format | Description | Response |
|------|--------|-------------|----------|
| 0 | `0,PASS#` | Open door | `Door$Unlocked#` / `WRONG#` / `NOT_INIT#` |
| 1 | `1,OLD,NEW#` | Change password | `CHANGED#` / `WRONG#` / `NOT_INIT#` |
| 2 | `2,TIMEOUT#` | Set timeout (5-30s) | `Time Saved#` / `BAD_VALUE#` |
| 3 | `3,PASS#` | First-time password setup | `INIT_OK#` / `ALREADY_INIT#` / `BAD_LENGTH#` |
| 4 | `4#` | Check initialization status | `INITIALIZED#` / `NOT_INITIALIZED#` |

### Error Responses
- `ERR#` - General protocol error (malformed message)
- `WRONG#` - Incorrect password
- `NOT_INIT#` - Password not initialized yet
- `ALREADY_INIT#` - Attempting to initialize when already done
- `BAD_LENGTH#` - Password length is not 5 digits
- `INIT_FAIL#` - EEPROM error during initialization

## Security Features

1. **No Hardcoded Default Password**: System requires user to set password on first use
2. **Persistent Flag**: Password initialization state is stored in EEPROM with magic value
3. **Lockout Protection**: 3 failed attempts trigger lockout timer
4. **Password Protection**: Password operations blocked until initialization complete

## Testing Procedure

### Test 1: Fresh Device (EEPROM Erased)
```
1. Erase EEPROM or use fresh chip
2. Power on → expect "SETUP_REQUIRED#" and RED LED on
3. Send "4#" → expect "NOT_INITIALIZED#"
4. Send "0,12345#" → expect "NOT_INIT#"
5. Send "3,67890#" → expect "INIT_OK#"
6. Send "4#" → expect "INITIALIZED#"
7. Send "0,67890#" → expect "Door$Unlocked#"
```

### Test 2: Initialized Device
```
1. Power on → expect "READY#" and GREEN LED blink
2. Send "4#" → expect "INITIALIZED#"
3. Send "0,67890#" → expect "Door$Unlocked#"
4. Send "3,11111#" → expect "ALREADY_INIT#"
```

### Test 3: Password Change
```
1. Send "1,67890,11111#" → expect "CHANGED#"
2. Send "0,11111#" → expect "Door$Unlocked#"
3. Send "0,67890#" → expect "WRONG#" (old password no longer works)
```

## Developer Notes

- The magic value `0x5AA5C33C` is arbitrary but unique enough to avoid false positives
- Password is stored in first 5 bytes of EEPROM (across 2 words)
- Initialization flag is in a separate word (offset 2) to avoid conflicts
- All password operations check initialization status before proceeding
- System remains functional for timeout operations even without password initialization

## Troubleshooting

**Q: Device stuck showing "SETUP_REQUIRED#"**
- A: Send Mode 3 command to initialize: `3,12345#`

**Q: Can't initialize password, get "ALREADY_INIT#"**
- A: Password already set. Use Mode 1 to change it with old password.

**Q: How to reset password if forgotten?**
- A: Use EEPROM mass erase function or reprogram device. This will require re-initialization.

**Q: Can I change the password length?**
- A: Yes, modify `PASSWORD_LENGTH` in `password.h`, but ensure EEPROM mapping is updated accordingly.

## Migration from Old System

If updating existing devices that used the old hardcoded "12345" default:
1. Old devices will show as NOT_INITIALIZED (flag not present)
2. Users need to perform first-time setup with Mode 3
3. Alternatively, add migration code in `Password_Init()` to detect old passwords and set the flag
