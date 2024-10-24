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
 * Version: 1.1.0
 * Created on: 2024-10-24
 *
 * Description:
 * This file contains the implementation of the TinyCmd library.
 */

#include "TinyCmd.h"

#ifndef NULL
#define NULL ((void *)0)
#endif //NULL

//Local structs
typedef struct Tiny_Command_List {
    TinyCmd_Command* list[CMD_LIST_SIZE];
    TinyCmd_Counter_Type length;

}Tiny_Command_List;

typedef struct Tiny_Command_inuput{
	char buf[64];
	char* arg[CMD_MAX_PARAMS];
	TinyCmd_Counter_Type length;
}Tiny_Command_input;

//Local Variables
static char* strtok_next = NULL; 
Tiny_Command_List TinyCmdRunning_Cmd;
TinyCmd_Counter_Type token_count;

//Global Variables
Tiny_Command_input TinyCmd_buf;

//Local Function

//If <string.h> is included,TinyCmd will use functions in <string.h>,instead of the following functions.
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

static char* TinyCmd_strcpy(char* dest, const char* src) {
    if (dest == NULL || src == NULL) {
        return NULL;
    }

    char* dest_ptr = dest;
    while ((*dest_ptr++ = *src++) != '\0') {

	}

    return dest;
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

//Global functions

//TinyCmd_Status TinyCmd_Init(void):
//Description:Call this function when TinyCmd_buf is filled ,namely after you call TinyCmd_PutString()
//Returns:
//        TINYCMD_SUCCESS: Initialization successful.
//        TINYCMD_FAILED: Initialization failed.
TinyCmd_Status Tiny_Command_Handler(void) {
    TinyCmd_Counter_Type i = 0;
    const char* delims = " ";
    char* context;

    //Read Command
    char* token = TinyCmd_strtok_s(TinyCmd_buf.buf, delims, &context);
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

    //Excute callback function of command
    for (i = 0; i < TinyCmdRunning_Cmd.length; i++) {
        
        if (!TinyCmd_strcmp(command, TinyCmdRunning_Cmd.list[i]->command)) 
        {
            TinyCmdRunning_Cmd.list[i]->callback();
            //Clear TinyCmd_buf
            TinyCmd_buf.length = 0; 
            TinyCmd_buf.arg[0] = NULL;
            TinyCmd_buf.buf[0] = '\0';
            return TINYCMD_SUCCESS;
        }
    }

    //Clear TinyCmd_buf
    TinyCmd_buf.length = 0; 
    TinyCmd_buf.arg[0] = NULL;
    TinyCmd_buf.buf[0] = '\0';
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


//TinyCmd_Status TinyCmd_Arg_Check(char* arg1,TinyCmd_Counter_Type p_arg2):
//Description:Check if the argument at position p_arg2 matches the given argument arg1.
//args:
//        arg1: Pointer to the argument string to compare.
//        p_arg2: Position of the argument in the TinyCmd_buf.arg array.
//Returns:
//        TINYCMD_SUCCESS: Argument matches.
//        TINYCMD_FAILED: Argument does not match.
TinyCmd_Status TinyCmd_Arg_Check(char* arg1,TinyCmd_Counter_Type p_arg2)
{
    if(!TinyCmd_strcmp(arg1,TinyCmd_buf.arg[p_arg2]))
    {
        return TINYCMD_SUCCESS;
    }
    else{
        return TINYCMD_FAILED;
    }
        
}

//TinyCmd_Status TinyCmd_PutChar(char c):
//Description:Add a character to the TinyCmd_buf.buf.
//args:
//        c: Character to add to the TinyCmd_buf.buf.
//Returns:
//        TINYCMD_SUCCESS: Character added successfully.
//        TINYCMD_FAILED: Character addition failed.
TinyCmd_Status TinyCmd_PutChar(char c)
{
    if(TinyCmd_buf.length < CMD_BUF_SIZE){
        TinyCmd_buf.buf[TinyCmd_buf.length++] = c;
        return TINYCMD_SUCCESS;
    }
    else{
        return TINYCMD_FAILED;
    }
}

//TinyCmd_Status TinyCmd_trim(char *str)
//Description:Trim the unnecessary shit(' ','\r','\n') characters from the end of the string.
static void TinyCmd_trim(char *str) {
    TinyCmd_Counter_Type len = 0;
    while (str[len] != '\0') {
        len++;
    }

    while (len > 0 && (str[len - 1] == ' ' || str[len - 1] == '\r' || str[len - 1] == '\n')) {
        str[--len] = '\0';
    }
}

//TinyCmd_Status TinyCmd_PutString(char* str):
//Description:Add a string to the TinyCmd_buf.buf.
//args:
//        str: String to add to the TinyCmd_buf.buf.
//Returns:
//        TINYCMD_SUCCESS: String added successfully.
//        TINYCMD_FAILED: String addition failed.
TinyCmd_Status TinyCmd_PutString(char* str)
{
    TinyCmd_trim(str);
    TinyCmd_Counter_Type length = TinyCmd_strlen(str);
    
    if(length <= CMD_BUF_SIZE){
        TinyCmd_buf.length = length;
        TinyCmd_strcpy(TinyCmd_buf.buf,str);
	    TinyCmd_PutChar('\0');
        return TINYCMD_SUCCESS;
    }
    else{
        return TINYCMD_FAILED;
    }
}

//char* TinyCmd_Arg_Get(TinyCmd_Counter_Type p_arg2):
//Description:Get the argument at position p_arg2 from the TinyCmd_buf.arg array.
//args:
//        p_arg2: Position of the argument in the TinyCmd_buf.arg array.
//Returns:
//        Pointer to the argument string.
char* TinyCmd_Arg_Get(TinyCmd_Counter_Type p_arg2)
{
    return TinyCmd_buf.arg[p_arg2];
}




