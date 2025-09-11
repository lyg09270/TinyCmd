### TinyCmd API Reference

#### Overview

`TinyCmd` is a lightweight command parsing library designed to handle and parse user input commands. It provides various functionalities, including command registration, argument parsing, and reporting.

#### Macro Definitions

- **`CMD_SEND_CHAR(c)`**
  - **Purpose**: Used to send a character to the user.
  - **Description**: By default, it uses `TinyCmd_SendChar(c)` to send characters. If you need to use a custom `putchar` function, you can redefine this macro.

- **`USE_USART_DMA_SEND_STR`**
  - **Purpose**: Used to enable or disable the use of USART DMA.
  - **Description**: If defined, it enables the use of USART DMA for sending strings. If not defined, it disables the use of USART DMA.

- **`CMD_SEND_STRING(str)`**
  - **Purpose**: Used to send a string to the user.
  - **Description**: If `USE USART DMA SEND STR` is defined, the string will be sent using `TinyCmd SendString(str)`. If 'USE USART DMA SEND STR' is not defined,  will use `CMD_SEND_CHAR(c)` sends characters.
  - **Note**:
    - If `USE USART DMA SEND STR` is defined, it is necessary to ensure that the `TinyCmd SendString(str)` function is implemented correctly and is consistent with the definition of the `CMD SEND STRING(str)` macro.
    - This function is used to enhance the performance of serial port transmission, especially when using USART+DMA.

- **`CMD_RPT_BUF_SIZE`**
  - **Purpose**: The size of the report buffer, used to store data to be sent.
  - **Default Value**: 255
- **`CMD_NAME_LENGTH`**
  - **Purpose**: The maximum length of a command or argument name.
  - **Default Value**: 8
- **`CMD_LIST_SIZE`**
  - **Purpose**: The maximum number of commands that can be added to the command list.
  - **Default Value**: 6
- **`CMD_MAX_TOKENS`**
  - **Purpose**: The maximum number of tokens in a command, representing the total number of the command and its arguments.
  - **Default Value**: 4
- **`CMD_MAX_PARAMS`**
  - **Purpose**: The maximum number of parameters in a command.
  - **Calculation Formula**: `CMD_MAX_TOKENS - 1`
- **`CMD_BUF_SIZE`**
  - **Purpose**: The maximum length of the command buffer string.
  - **Calculation Formula**: `CMD_NAME_LENGTH * CMD_MAX_TOKENS + CMD_MAX_TOKENS - 1`

#### Type Definitions

- **`TinyCmd_CallBack_Ret`**
  - **Purpose**: The return type for callback functions.
  - **Type**: `unsigned char`
  - **Description**: When writing a callback function, it must return this type.
- **`TinyCmd_Counter_Type`**
  - **Purpose**: Counter type used for loops and other counting purposes.
  - **Type**: `unsigned char`
  - **Description**: If your input or output buffer exceeds 255 bytes, you should change this type to a larger one (e.g., `unsigned int`).
- **`SendCharFunc`**
  - **Purpose**: Function pointer type for sending characters.
  - **Definition**: `typedef void (*SendCharFunc)(char c);`

- **`SendStringFunc`**
  - **Purpose**: Function pointer type for sending strings.
  - **Definition**: `typedef void (*SendStringFunc)(const char* str);`

#### Enumerations

- **`TinyCmd_Status`**

  - **Purpose**: Represents the status of command processing.

  - Enum Values

    :

    - `TINYCMD_FAILED` (0): Indicates failure.
    - `TINYCMD_SUCCESS` (1): Indicates success.

- **`TinyCmd_NumType`**

  - **Purpose**: Represents numeric types, used when converting input arguments to numbers using `TinyCmd_Arg_To_Num`.

  - Enum Values

    :

    - `TINYCMD_UINT8`: Unsigned 8-bit integer.
    - `TINYCMD_INT8`: Signed 8-bit integer.
    - `TINYCMD_UINT16`: Unsigned 16-bit integer.
    - `TINYCMD_INT16`: Signed 16-bit integer.
    - `TINYCMD_UINT32`: Unsigned 32-bit integer.
    - `TINYCMD_INT32`: Signed 32-bit integer.
    - `TINYCMD_UINT64`: Unsigned 64-bit integer (available only if `CMD_NAME_LENGTH > 9`).
    - `TINYCMD_INT64`: Signed 64-bit integer (available only if `CMD_NAME_LENGTH > 9`).
    - `TINYCMD_FLOAT`: Floating-point number.
    - `TINYCMD_DOUBLE`: Double-precision floating-point number.

#### Structures

- **`TinyCmd_Buffer`**

  - **Purpose**: Stores the input buffer and arguments.

  - **Description**: Users need to fill `TinyCmd_Buffer.input` with the command line input. This buffer will be saved until the `TinyCmd_Handler` function is called.

  - Members

    :

    - `char input[CMD_BUF_SIZE]`: Input buffer string.
    - `char* arg[CMD_MAX_PARAMS]`: Array of argument pointers. After calling `TinyCmd_Handler`, `arg` will be populated with the argument values. It is not recommended to access these directly; use dedicated functions to access command line arguments.
    - `TinyCmd_Counter_Type length`: Length of the input buffer.

- **`TinyCmd_Command`**

  - **Purpose**: Defines a command structure.

  - **Description**: When creating a command, use this structure to fill in the command name and callback function, then call `TinyCmd_Add_Cmd` to add the command to the list of commands to be executed.

  - Members

    :

    - `const char* command`: Command name.
    - `TinyCmd_CallBack_Ret (*callback)(void)`: Callback function pointer.

#### Global Variables

- **`TinyCmd_Buffer TinyCmd_buf`**
  - **Purpose**: Global instance of `TinyCmd_Buffer` used to store the input buffer and arguments.
- **`SendCharFunc TinyCmd_SendChar`**
  - **Purpose**: Global instance of `SendCharFunc` used to send characters to the user.
  - **Description**: User-specified function for sending characters. If you need to use the `TinyCmd_Report` function, you must assign this variable to your specified send character function before calling `TinyCmd_Report`.

#### Functions

- **`char* TinyCmd_strcpy(char* dest, const char* src)`**

  - **Purpose**: Copies a string.

  - Parameters

    :

    - `dest`: Destination string.
    - `src`: Source string.

  - **Return Value**: Pointer to the destination string.

- **`TinyCmd_Status TinyCmd_Handler(void)`**

  - **Purpose**: Processes the input buffer to detect commands, populate the argument buffer, and call the detected command's callback function.

  - **Parameters**: None.

  - Return Values

    :

    - `TINYCMD_SUCCESS`: Processing successful.
    - `TINYCMD_FAILED`: Processing failed.

- **`TinyCmd_Status TinyCmd_Add_Cmd(TinyCmd_Command* newCmd)`**

  - **Purpose**: Adds a new command to the list of recognizable and executable commands.

  - Parameters

    :

    - `newCmd`: Pointer to a `TinyCmd_Command` structure.

  - Return Values

    :

    - `TINYCMD_SUCCESS`: Addition successful.
    - `TINYCMD_FAILED`: Addition failed.

- **`TinyCmd_Status TinyCmd_Arg_Check(const char* arg1, TinyCmd_Counter_Type p_arg2)`**

  - **Purpose**: Checks an argument.

  - Parameters

    :

    - `arg1`: Argument string.
    - `p_arg2`: Argument index.

  - Return Values

    :

    - `TINYCMD_SUCCESS`: Check successful.
    - `TINYCMD_FAILED`: Check failed.

- **`TinyCmd_Counter_Type TinyCmd_Arg_Get_Len(TinyCmd_Counter_Type p_arg)`**

  - **Purpose**: Gets the length of an argument.

  - Parameters

    :

    - `p_arg`: Argument index.

  - **Return Value**: Length of the argument.

- **`TinyCmd_Status TinyCmd_Arg_To_Num(TinyCmd_Counter_Type p_arg, void* out_val, TinyCmd_NumType type)`**

  - **Purpose**: Converts an argument to a specified numeric type.

  - Parameters

    :

    - `p_arg`: Argument index.
    - `out_val`: Pointer to the output value.
    - `type`: Numeric type.

  - Return Values

    :

    - `TINYCMD_SUCCESS`: Conversion successful.
    - `TINYCMD_FAILED`: Conversion failed.

- **`TinyCmd_Status TinyCmd_Report(const char* format, ...)`**

  - **Purpose**: Reports information.

  - Parameters

    :

    - `format`: Format string.
    - `...`: Variable argument list.

  - Return Values

    :

    - `TINYCMD_SUCCESS`: Report successful.
    - `TINYCMD_FAILED`: Report failed.