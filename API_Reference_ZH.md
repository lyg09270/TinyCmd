# TinyCmd API Reference



## 概述

`TinyCmd` 是一个轻量级的命令解析库，用于处理和解析用户输入的命令。它提供了多种功能，包括命令注册、参数解析和报告功能。

#### 宏定义

- **`CMD_SEND_CHAR(c)`**
  - **用途**：用于发送字符到用户。
  - **描述**：默认使用 `TinyCmd_SendChar(c)` 发送字符。如果需要使用自定义的 `putchar` 函数，可以重新定义此宏。

- **`USE_USART_DMA_SEND_STR`**
  - **用途**：与`CMD_SEND_STRING(str)`搭配使用，用于启用或禁用 USART DMA 发送字符串。
  - **描述**：如果定义了此宏，将启用 USART DMA 发送字符串。如果未定义，将禁用 USART DMA 发送字符串。

- **`CMD_SEND_STRING(str)`**
  - **用途**：用于发送字符串到用户。
  - **描述**：如果 `USE_USART_DMA_SEND_STR` 定义了，将使用 `TinyCmd_SendString(str)` 发送字符串。 如果未定义 `USE_USART_DMA_SEND_STR`，将使用 `CMD_SEND_CHAR(c)` 发送字符。
  - **注意**：
    - 如果定义了 `USE_USART_DMA_SEND_STR`，则需要确保 `TinyCmd_SendString(str)` 函数已正确实现，并且与 `CMD_SEND_STRING(str)` 宏的定义一致。
    - 这个函数用于提高串口发送的性能，尤其是在使用USART+DMA时。

- **`CMD_RPT_BUF_SIZE`**
  - **用途**：报告缓冲区的大小，用于存储待发送的数据
  - **默认值**：255
- **`CMD_NAME_LENGTH`**
  - **用途**：命令或参数名称的最大长度。
  - **默认值**：8
- **`CMD_LIST_SIZE`**
  - **用途**：命令列表的最大长度，默认最大可以添加6个命令
  - **默认值**：6
- **`CMD_MAX_TOKENS`**
  - **用途**：命令中最大令牌数，表示命令+参数的总数，默认值4表示1个命令和3个参数
  - **默认值**：4
- **`CMD_MAX_PARAMS`**
  - **用途**：命令中最大参数数。
  - **计算公式**：`CMD_MAX_TOKENS - 1`
- **`CMD_BUF_SIZE`**
  - **用途**：命令缓冲区字符串的最大长度。
  - **计算公式**：`CMD_NAME_LENGTH * CMD_MAX_TOKENS + CMD_MAX_TOKENS - 1`

#### 类型定义

- **`TinyCmd_CallBack_Ret`**
  - **用途**：回调函数的返回类型。
  - **类型**：`unsigned char`
  - **描述**：在编写回调函数时，必须以这个类型作为返回值
- **`TinyCmd_Counter_Type`**
  - **用途**：计数器类型，用于循环计数等。
  - **类型**：`unsigned char`
  - **描述**：如果输入缓冲区或输出缓冲区超过 255 字节，需要将此类型改为更大的类型（如 `unsigned int`）。
- **`SendCharFunc`**
  - **用途**：发送字符的函数指针类型。
  - **定义**：`typedef void (*SendCharFunc)(char c);`

- **`SendStringFunc`**
  - **用途**：发送字符串的函数指针类型。
  - **定义**：`typedef void (*SendStringFunc)(const char* str);`

#### 枚举

- **`TinyCmd_Status`**
  - **用途**：表示命令处理的状态。
  - 枚举值
    - `TINYCMD_FAILED` (0): 表示失败。
    - `TINYCMD_SUCCESS` (1): 表示成功。
- **`TinyCmd_NumType`**
  - **用途**：表示数值类型，在使用`TinyCmd_Arg_To_Num`函数将输入参数转换为数字时，需要填写转换目标数据的类型。
  - 枚举值
    - `TINYCMD_UINT8`: 无符号 8 位整数。
    - `TINYCMD_INT8`: 有符号 8 位整数。
    - `TINYCMD_UINT16`: 无符号 16 位整数。
    - `TINYCMD_INT16`: 有符号 16 位整数。
    - `TINYCMD_UINT32`: 无符号 32 位整数。
    - `TINYCMD_INT32`: 有符号 32 位整数。
    - `TINYCMD_UINT64`: 无符号 64 位整数（仅当 `CMD_NAME_LENGTH > 9` 时可用）。
    - `TINYCMD_INT64`: 有符号 64 位整数（仅当 `CMD_NAME_LENGTH > 9` 时可用）。
    - `TINYCMD_FLOAT`: 浮点数。
    - `TINYCMD_DOUBLE`: 双精度浮点数。

#### 结构体

- **`TinyCmd_Buffer`**
  - **用途**：存储输入缓冲区和参数。
  - 描述：用户需要在外部填写**`TinyCmd_Buffer.input`**，作为命令行输入，在调用`TinyCmd_Handler`函数前这个缓冲区都将被保存
  - 成员
    - `char input[CMD_BUF_SIZE]`: 输入缓冲区字符串。
    - `char* arg[CMD_MAX_PARAMS]`: 参数指针数组，在调用了`TinyCmd_Handler`函数后，arg会被附上参数值，不建议在外部直接访问，有专用的函数用于访问命令行参数。
    - `TinyCmd_Counter_Type length`: 输入缓冲区的长度。
- **`TinyCmd_Command`**
  - **用途**：定义命令结构。
  - 描述：创建命令时使用这个结构体填入命令名称和回调函数，然后调用`TinyCmd_Add_Cmd`将这个命令添加到等待运行的命令中
  - 成员
    - `const char* command`: 命令名称。
    - `TinyCmd_CallBack_Ret (*callback)(void)`: 回调函数指针。

#### 全局变量

- **`TinyCmd_Buffer TinyCmd_buf`**
  - **用途**：全局的 `TinyCmd_Buffer` 实例，用于存储输入缓冲区和参数。
- **`SendCharFunc TinyCmd_SendChar`**
  - **用途**：全局的 `SendCharFunc` 实例，用于发送字符到用户。
  - **描述**：用户指定的发送字符函数，如果需要使用 `TinyCmd_Report` 函数，必须在调用 `TinyCmd_Report` 之前将这个变量赋值为用户指定的发送字符函数。

#### 函数

- **`char* TinyCmd_strcpy(char* dest, const char* src)`**
  - **用途**：复制字符串。
  - 参数
    - `dest`: 目标字符串。
    - `src`: 源字符串。
  - **返回值**：指向目标字符串的指针。
- **`TinyCmd_Status TinyCmd_Handler(void)`**
  - **用途**：处理输入缓冲区中的命令，如果检测到命令，将参数放入参数缓冲区，并调用检测到的命令的回调函数
  - **参数**：无。
  - 返回值
    - `TINYCMD_SUCCESS`: 处理成功。
    - `TINYCMD_FAILED`: 处理失败。
- **`TinyCmd_Status TinyCmd_Add_Cmd(TinyCmd_Command* newCmd)`**
  - **用途**：添加新命令到可识别并执行的命令
  - 参数
    - `newCmd`: 指向 `TinyCmd_Command` 结构的指针。
  - 返回值
    - `TINYCMD_SUCCESS`: 添加成功。
    - `TINYCMD_FAILED`: 添加失败。
- **`TinyCmd_Status TinyCmd_Arg_Check(const char* arg1, TinyCmd_Counter_Type p_arg2)`**
  - **用途**：检查参数。
  - 参数
    - `arg1`: 参数字符串。
    - `p_arg2`: 参数索引。
  - 返回值
    - `TINYCMD_SUCCESS`: 检查成功。
    - `TINYCMD_FAILED`: 检查失败。
- **`TinyCmd_Counter_Type TinyCmd_Arg_Get_Len(TinyCmd_Counter_Type p_arg)`**
  - **用途**：获取参数的长度。
  - 参数
    - `p_arg`: 参数索引。
  - **返回值**：参数的长度。
- **`TinyCmd_Status TinyCmd_Arg_To_Num(TinyCmd_Counter_Type p_arg, void* out_val, TinyCmd_NumType type)`**
  - **用途**：将参数转换为指定的数值类型。
  - 参数
    - `p_arg`: 参数索引。
    - `out_val`: 指向输出值的指针。
    - `type`: 数值类型。
  - 返回值
    - `TINYCMD_SUCCESS`: 转换成功。
    - `TINYCMD_FAILED`: 转换失败。
- **`TinyCmd_Status TinyCmd_Report(const char\* format, ...)`**
  - **用途**：报告信息。
  - 参数
    - `format`: 格式字符串。
    - `...`: 可变参数列表。
  - 返回值
    - `TINYCMD_SUCCESS`: 报告成功。
    - `TINYCMD_FAILED`: 报告失败。