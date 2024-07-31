/*******************************************************************************
* File Name       : thread_usb.h
* Description     : 
* Original Author : Vincent.Chang
* Created on      : Aug 04, 2023
*******************************************************************************/

#ifndef THREAD_USB_H
#define	THREAD_USB_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdio.h>

//------------------------------------------------------------------------------
unsigned char char_to_hex(char c);
void string_to_hex_array(const char* str, unsigned char* hex_array, size_t array_size);
char hex_to_char(unsigned char hex);
void hex_array_to_string(const unsigned char* hex_array, size_t array_size, char* str);
//------------------------------------------------------------------------------
#ifdef	__cplusplus
}
#endif

#endif	/* THREAD_USB_H */
