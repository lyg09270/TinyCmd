/*
 * Copyright 2024 Civic_Crab
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * File: TinyCmd.c
 * Author: Civic_Crab
 * Version: 1.2.0
 * Created on: 2024-10-24
 *
 * Description:
 * This file contains the implementation of the TinyCmd library.
 */

#include "TinyCmd.h"
#include <stdarg.h>
#ifndef NULL
#define NULL ((void *)0)
#endif //NULL

#ifndef LIMITS_H
#define UINT8_MAX 255
#define INT8_MAX 127
#define INT8_MIN -128
#define UINT16_MAX 65535
#define INT16_MAX 32767
#define INT16_MIN -32768
#define UINT32_MAX 4294967295u
#define INT32_MAX 2147483647
#define INT32_MIN (~0x7fffffff)
#define FLT_MAX 3.402823e+38
#define DBL_MAX 1.7976931348623157e+308
#endif //LIMITS_H

//Local structs****************************************************************//
typedef struct TinyCmd_List {
    TinyCmd_Command* list[CMD_LIST_SIZE];
    TinyCmd_Counter_Type length;

}TinyCmd_List;

//Local Variables****************************************************************//
static char* strtok_next = NULL;
TinyCmd_List TinyCmdRunning_Cmd;
TinyCmd_Counter_Type token_count;

//Global Variables****************************************************************//
TinyCmd_Buffer TinyCmd_buf;
SendCharFunc TinyCmd_SendChar = NULL;

//Local Function****************************************************************//

static int TinyCmd_strcmp(const char* str1, const char* str2) {
    if (str1 == NULL || str2 == NULL) {
        return -1;
    }

    while (*str1 == *str2) {
        if (*str1 == '\0') {
            return 0;
        }
        str1++;
        str2++;
    }

    return (unsigned char)*str1 - (unsigned char)*str2;
}

static char* TinyCmd_strchr(const char* str, int c) {
    if (str == NULL) {
        return NULL;
    }

    while (*str != '\0') {
        if (*str == (char)c) {
            return (char*)str;
        }
        str++;
    }

    if ((char)c == '\0') {
        return (char*)str;
    }

    return NULL;
}

static char* TinyCmd_strtok_s(char* str, const char* delim, char** saveptr) {
    char* start;
    char* end;

    if (str != NULL) {
        *saveptr = str;
    }

    if (*saveptr == NULL) {
        return NULL;
    }

    start = *saveptr;
    while (*start && TinyCmd_strchr(delim, *start)) {
        start++;
    }

    if (*start == '\0') {
        *saveptr = NULL;
        return NULL;
    }

    end = start;
    while (*end && !TinyCmd_strchr(delim, *end)) {
        end++;
    }

    if (*end) {
        *end = '\0';
        *saveptr = end + 1;
    }
    else {
        *saveptr = NULL;
    }

    return start;
}

static TinyCmd_Counter_Type TinyCmd_strlen(const char* str) {
    const char* p = str;
    while (*p != '\0') {
        p++;
    }
    return p - str;
}

static TinyCmd_Status TinyCmd_Buf_Clear(void)
{
    TinyCmd_Counter_Type i = 0;
    for(i = 0; i < CMD_MAX_PARAMS; i++) {
        TinyCmd_buf.arg[i] = NULL;
    }
    for(i = 0; i < CMD_BUF_SIZE; i++) {
        TinyCmd_buf.input[i] = '\0';
    }
    TinyCmd_buf.length = 0;

    return TINYCMD_SUCCESS;
}

static double TinyCmd_pow(double base, int exponent) {
    if (exponent == 0) {
        return 1.0;
    }

    double result = 1.0;
    int abs_exponent = exponent < 0 ? -exponent : exponent;

    while (abs_exponent > 0) {
        if (abs_exponent % 2 == 1) {
            result *= base;
        }
        base *= base;
        abs_exponent /= 2;
    }

    if (exponent < 0) {
        result = 1.0 / result;
    }

    return result;
}

static TinyCmd_Status TinyCmd_isdigit(int c) {
    return (c >= '0' && c <= '9');
}

static TinyCmd_Status TinyCmd_isspace(int c) {
    return (c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r');
}

static inline char TinyCmd_tolower(char c) {
    if (c >= 'A' && c <= 'Z') {
        return c + ('a' - 'A');
    }
    return c;
}

static TinyCmd_Status str_to_uint(const char* str, unsigned long long* result, int* sign) {
    *result = 0;
    *sign = 1;

    if (*str == '-') {
        *sign = -1;
        str++;
    } else if (*str == '+') {
        str++;
    }

    while (TinyCmd_isdigit(*str)) {
        unsigned long long new_result = *result * 10 + (*str - '0');
        if (new_result < *result) {
            return TINYCMD_FAILED;
        }
        *result = new_result;
        str++;
    }

    return TINYCMD_SUCCESS;
}

static TinyCmd_Status str_to_int(const char* str, long long* result, int* sign) {
    unsigned long long unsigned_result;
    TinyCmd_Status status = str_to_uint(str, &unsigned_result, sign);
    if (status != TINYCMD_SUCCESS) {
        return TINYCMD_FAILED;
    }

    *result = (long long)unsigned_result;
    if (*sign == -1) {
        *result = -*result;
    }

    return TINYCMD_SUCCESS;
}

static TinyCmd_Status str_to_float(const char* str, double* result) {
    if (!str || !result) {
        return TINYCMD_FAILED;
    }

    double value = 0.0;
    double sign = 1.0;
    double fractional_part = 0.0;
    double exponent = 0.0;
    double exponent_sign = 1.0;

    while (TinyCmd_isspace((unsigned char)*str)) {
        str++;
    }

    if (*str == '-') {
        sign = -1.0;
        str++;
    } else if (*str == '+') {
        str++;
    }

    while (TinyCmd_isdigit((unsigned char)*str)) {
        value = value * 10.0 + (*str - '0');
        str++;
    }

    if (*str == '.') {
        str++;
        double place = 0.1;
        while (TinyCmd_isdigit((unsigned char)*str)) {
            fractional_part += (*str - '0') * place;
            place *= 0.1;
            str++;
        }
    }

    if (TinyCmd_tolower((unsigned char)*str) == 'e') {
        str++;
        if (*str == '-') {
            exponent_sign = -1.0;
            str++;
        } else if (*str == '+') {
            str++;
        }
        while (TinyCmd_isdigit((unsigned char)*str)) {
            exponent = exponent * 10.0 + (*str - '0');
            str++;
        }
    }

    value = (value + fractional_part) * sign;
    exponent = TinyCmd_pow(10.0, exponent * exponent_sign);
    *result = value * exponent;

    if (*str != '\0') {
        return TINYCMD_FAILED;
    }

    return TINYCMD_SUCCESS;
}

static void itoa(int value, char* buffer, int base) {
    char* p = buffer;
    int sign = (base == 10 && value < 0);
    unsigned int uvalue = (sign) ? -value : value;

    do {
        int remainder = uvalue % base;
        *p++ = (remainder < 10) ? remainder + '0' : remainder + 'a' - 10;
    } while (uvalue /= base);

    if (sign) {
        *p++ = '-';
    }

    *p = '\0';
    for (int i = 0, j = p - buffer - 1; i < j; i++, j--) {
        char temp = buffer[i];
        buffer[i] = buffer[j];
        buffer[j] = temp;
    }
}

static void uitoa(unsigned int value, char* buffer, int base) {
    char* p = buffer;
    do {
        int remainder = value % base;
        *p++ = (remainder < 10) ? remainder + '0' : remainder + 'a' - 10;
    } while (value /= base);

    *p = '\0';
    for (int i = 0, j = p - buffer - 1; i < j; i++, j--) {
        char temp = buffer[i];
        buffer[i] = buffer[j];
        buffer[j] = temp;
    }
}

static void dtoa(double value, char* buffer, int precision) {
    int integer_part = (int)value;
    double fractional_part = value - integer_part;

    if(fractional_part < 0)
    {
        fractional_part = -fractional_part;
    }

    itoa(integer_part, buffer, 10);
    char* p = buffer + TinyCmd_strlen(buffer);

    *p++ = '.';

    for (int i = 0; i < precision; i++) {
        fractional_part *= 10;
        int digit = (int)fractional_part;
        *p++ = digit + '0';
        fractional_part -= digit;
    }

    *p = '\0';
}

//TinyCmd_Status TinyCmd_SendChar(char c)
//Description:Send a character to some where user designated.
static void send_string(const char* str) {
#ifndef USE_USART_DMA_SEND_STR
    while (*str)
    {
        CMD_SEND_CHAR(*str++);
    }
#else
    CMD_SEND_STRING(str);
#endif
}

//TinyCmd_Status TinyCmd_trim(char *str)
//Description:Trim the unnecessary shit(' ','\r','\n') characters from the end of the string.
static void TinyCmd_trim(char *str) {
    TinyCmd_Counter_Type len = TinyCmd_strlen(str);

    while (len > 0 && (str[len - 1] == ' ' || str[len - 1] == '\r' || str[len - 1] == '\n')) {
        str[--len] = '\0';
    }
}

//Global functions****************************************************************//

//char* TinyCmd_strcpy(char* dest, const char* src)
//Description:Copy the string from src to dest.
char* TinyCmd_strcpy(char* dest, const char* src)
{
    if (dest == NULL || src == NULL) {
        return NULL;
    }

    char* dest_ptr = dest;
    while ((*dest_ptr++ = *src++) != '\0') {

	}

    return dest;
}

//TinyCmd_Status TinyCmd_Init(void):
//Description:Call this function when TinyCmd_buf is filled ,namely after you call TinyCmd_PutString()
//Returns:
//        TINYCMD_SUCCESS: Initialization successful.
//        TINYCMD_FAILED: Initialization failed.
TinyCmd_Status TinyCmd_Handler(void) {
    TinyCmd_Counter_Type i = 0;
    const char* delims = " ";
    char* context;

    TinyCmd_trim(TinyCmd_buf.input);

    //Read Command
    char* token = TinyCmd_strtok_s(TinyCmd_buf.input, delims, &context);
    char* command = token;

    //Read arguments
    token = TinyCmd_strtok_s(NULL, delims ,&context);

    while (token != NULL) {
        if (token_count < CMD_MAX_TOKENS) {
            TinyCmd_buf.arg[i++] = token;
        }
        else {
            // Maximum number of tokens reached.
            strtok_next = NULL;
            break;
        }
        token = TinyCmd_strtok_s(NULL, delims ,&context);
    }

    TinyCmd_Report("Command: %s\n", command);
    TinyCmd_Report("Number of args: %d\n", i);
    for (TinyCmd_Counter_Type j = 0; j < i; j++)
    {
        TinyCmd_Report("Arg[%d]: %s\n", j, TinyCmd_buf.arg[j]);
    }
    
    //Excute callback function of command
    for (i = 0; i < TinyCmdRunning_Cmd.length; i++) {

        if (!TinyCmd_strcmp(command, TinyCmdRunning_Cmd.list[i]->command))
        {
            TinyCmdRunning_Cmd.list[i]->callback();
            //Clear TinyCmd_buf
            TinyCmd_Buf_Clear();
            return TINYCMD_SUCCESS;
        }
    }

    //Clear TinyCmd_buf
    TinyCmd_Buf_Clear();
    return TINYCMD_FAILED;
}

//TinyCmd_Status TinyCmd_Add_Cmd(TinyCmd_Command* newCmd):
//Description:Add a new command to the TinyCmdRunning_Cmd list.
//args:
//        newCmd: Pointer to the TinyCmd_Command struct containing the command and callback function.
//Returns:
//        TINYCMD_SUCCESS: Command added successfully.
//        TINYCMD_FAILED: Command addition failed.
TinyCmd_Status TinyCmd_Add_Cmd(TinyCmd_Command* newCmd)
{
    if (newCmd == NULL){
        return TINYCMD_FAILED;
    }
    else{
        if(newCmd->callback != NULL){
            TinyCmdRunning_Cmd.list[TinyCmdRunning_Cmd.length++] = newCmd;
            return TINYCMD_SUCCESS;
        }
        else{
            return TINYCMD_FAILED;
        }
    }
}


//TinyCmd_Status TinyCmd_Arg_Check(char* arg1,TinyCmd_Counter_Type p_arg):
//Description:Check if the argument at position p_arg2 matches the given argument arg1.
//args:
//        arg1: Pointer to the argument string to compare.
//        p_arg2: Position of the argument in the TinyCmd_buf.arg array.
//Returns:
//        TINYCMD_SUCCESS: Argument matches.
//        TINYCMD_FAILED: Argument does not match.
TinyCmd_Status TinyCmd_Arg_Check(const char* arg1,TinyCmd_Counter_Type p_arg2)
{
    if(!TinyCmd_strcmp(arg1,TinyCmd_buf.arg[p_arg2]))
    {
        return TINYCMD_SUCCESS;
    }
    else{
        return TINYCMD_FAILED;
    }

}

//char* TinyCmd_Arg_Get_Len(TinyCmd_Counter_Type p_arg):
//Description:Get the length of the argument at position p_arg from the TinyCmd_buf.arg array.
//args:
//        p_arg: Position of the argument in the TinyCmd_buf.arg array.
//Returns:
//        Length of the argument string.
TinyCmd_Counter_Type TinyCmd_Arg_Get_Len(TinyCmd_Counter_Type p_arg)
{
    return TinyCmd_strlen(TinyCmd_buf.arg[p_arg]);
}


TinyCmd_Status TinyCmd_Arg_To_Num(TinyCmd_Counter_Type p_arg, void* out_val, TinyCmd_NumType type) {
    const char* str = TinyCmd_buf.arg[p_arg];
    if (!str) return TINYCMD_FAILED;

    int sign = 1;
    switch (type) {
        case TINYCMD_UINT8: {
            unsigned long long result;
            TinyCmd_Status status = str_to_uint(str, &result, &sign);
            if (status != TINYCMD_SUCCESS) {
                return TINYCMD_FAILED;
            }
            if (result > UINT8_MAX) {
                *(unsigned char*)out_val = UINT8_MAX;
            } else {
                *(unsigned char*)out_val = (unsigned char)result;
            }
            break;
        }
        case TINYCMD_INT8: {
            long long result;
            TinyCmd_Status status = str_to_int(str, &result, &sign);
            if (status != TINYCMD_SUCCESS) {
                return TINYCMD_FAILED;
            }
            if (result > INT8_MAX) {
                *(signed char*)out_val = INT8_MAX;
            } else if (result < INT8_MIN) {
                *(signed char*)out_val = INT8_MIN;
            } else {
                *(signed char*)out_val = (signed char)result;
            }
            break;
        }
        case TINYCMD_UINT16: {
            unsigned long long result;
            TinyCmd_Status status = str_to_uint(str, &result, &sign);
            if (status != TINYCMD_SUCCESS) {
                return TINYCMD_FAILED;
            }
            if (result > UINT16_MAX) {
                *(unsigned short*)out_val = UINT16_MAX;
            } else {
                *(unsigned short*)out_val = (unsigned short)result;
            }
            break;
        }
        case TINYCMD_INT16: {
            long long result;
            TinyCmd_Status status = str_to_int(str, &result, &sign);
            if (status != TINYCMD_SUCCESS) {
                return TINYCMD_FAILED;
            }
            if (result > INT16_MAX) {
                *(short*)out_val = INT16_MAX;
            } else if (result < INT16_MIN) {
                *(short*)out_val = INT16_MIN;
            } else {
                *(short*)out_val = (short)result;
            }
            break;
        }
        case TINYCMD_UINT32: {
            unsigned long long result;
            TinyCmd_Status status = str_to_uint(str, &result, &sign);
            if (status != TINYCMD_SUCCESS) {
                return TINYCMD_FAILED;
            }
            #if CMD_NAME_LENGTH > 9
            if (result > UINT32_MAX) {
                *(unsigned int*)out_val = UINT32_MAX;
            } else {
                *(unsigned int*)out_val = (unsigned int)result;
            }
            #else
            *(unsigned int*)out_val = (unsigned int)result;
            #endif
            break;
        }
        case TINYCMD_INT32: {
            long long result;
            TinyCmd_Status status = str_to_int(str, &result, &sign);
            if (status != TINYCMD_SUCCESS) {
                return TINYCMD_FAILED;
            }
            if (result > INT32_MAX) {
                *(int*)out_val = INT32_MAX;
            } else if (result < INT32_MIN) {
                *(int*)out_val = INT32_MIN;
            } else {
                *(int*)out_val = (int)result;
            }
            break;
        }
        //when CMD_NAME_LENGTH > 9 a more bigger type is needed
        #if CMD_NAME_LENGTH > 9
        case TINYCMD_UINT64: {
            unsigned long long result;
            TinyCmd_Status status = str_to_uint(str, &result, &sign);
            if (status != TINYCMD_SUCCESS) {
                return TINYCMD_FAILED;
            }
            *(unsigned long long*)out_val = (unsigned long long)result;
            break;
        }
        case TINYCMD_INT64: {
            long long result;
            TinyCmd_Status status = str_to_int(str, &result, &sign);
            if (status != TINYCMD_SUCCESS) {
                return TINYCMD_FAILED;
            }
            *(long long*)out_val = (long long)result;
            break;
        }
        #endif //CMD_NAME_LENGTH > 9
        case TINYCMD_FLOAT: {
            double result;
            TinyCmd_Status status = str_to_float(str, &result);
            if (status != TINYCMD_SUCCESS) {
                return TINYCMD_FAILED;
            }
                *(float*)out_val = (float)result;
            break;
        }
        case TINYCMD_DOUBLE: {
            double result;
            TinyCmd_Status status = str_to_float(str, &result);
            if (status != TINYCMD_SUCCESS) {
                return TINYCMD_FAILED;
            }
                *(double*)out_val = result;
            break;
        }
        default:
            return TINYCMD_FAILED;
    }

    if (str[0] == '\0' || (str[0] == '-' && str[1] == '\0')) {
        return TINYCMD_FAILED;
    }

    return TINYCMD_SUCCESS;
}

//TinyCmd_Status TinyCmd_Report(const char* format,...)
//Description:A printf-like function print the formatted string to somewhere user designated.
TinyCmd_Status TinyCmd_Report(const char* format, ...)
{
    va_list args;
    va_start(args, format);

    char buffer[CMD_RPT_BUF_SIZE];
    TinyCmd_Counter_Type pos = 0;
    TinyCmd_Counter_Type max_len = sizeof(buffer);

    while (*format && pos < max_len - 1) {
        if (*format == '%') {
            format++;
            switch (*format) {
                case 'd': {
                    int value = va_arg(args, int);
                    char num_buffer[32];
                    itoa(value, num_buffer, 10);
                    send_string(num_buffer);
                    break;
                }
                case 'u': {
                    unsigned int value = va_arg(args, unsigned int);
                    char num_buffer[32];
                    uitoa(value, num_buffer, 10);
                    send_string(num_buffer);
                    break;
                }
                case '.':
                {
                    double value = va_arg(args, double);
                    char num_buffer[64];
                    dtoa(value, num_buffer, (*(++format) - '0'));
                    send_string(num_buffer);
                    format++;
                    break;
                }
                case 'f': {
                    double value = va_arg(args, double);
                    char num_buffer[64];
                    dtoa(value, num_buffer, 6);
                    send_string(num_buffer);
                    break;
                }
                case 's': {
                    const char* str = va_arg(args, const char*);
                    send_string(str);
                    break;
                }
                default:
                    CMD_SEND_CHAR('%');
                    CMD_SEND_CHAR(*format);
                    break;
            }
        } else {
            CMD_SEND_CHAR(*format);
        }
        format++;
    }

    va_end(args);

    return TINYCMD_SUCCESS;
}