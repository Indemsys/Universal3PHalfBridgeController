
/*HEADER**********************************************************************
*
* Copyright 2008 Freescale Semiconductor, Inc.
* Copyright 2004-2008 Embedded Access Inc.
* Copyright 1989-2008 ARC International
*
* This software is owned or controlled by Freescale Semiconductor.
* Use of this software is governed by the Freescale MQX RTOS License
* distributed with this Material.
* See the MQX_RTOS_LICENSE file distributed for more details.
*
* Brief License Summary:
* This software is provided in source form for you to use free of charge,
* but it is not open source software. You are allowed to use this software
* but you cannot redistribute it or derivative works of it in source form.
* The software may be used only in connection with a product containing
* a Freescale microprocessor, microcontroller, or digital signal processor.
* See license agreement file for full license terms including other restrictions.
*****************************************************************************
*
* Comments:
*
*   This file contains the function for scanning and converting a line
*   of text.
*
*
*END************************************************************************/
#include "mqx_cnfg.h"
#if MQX_USE_IO_OLD

#include "mqx.h"
#include <string.h>
#include <stdlib.h>

#if MQX_INCLUDE_FLOATING_POINT_IO
#include <math.h>
#endif

#include "fio.h"
#include "fio_prv.h"
#include "io.h"
#include "io_prv.h"
#include "fpio_prv.h"


extern char _io_scanline_ignore_white_space( char  **, _mqx_uint *,
   _mqx_uint);
extern char _io_scanline_format_ignore_white_space( char  **,
   _mqx_uint *);
extern int32_t _io_scanline_base_convert( unsigned char, _mqx_uint );
extern bool _io_scanline_is_octal_digit( char );
extern bool _io_scanline_is_hex_digit( char );
#ifndef strtod
extern double strtod(const char *, char  **);
#endif
/*!
 * \brief Converts an input line of ASCII characters based upon a provided
 * string format.
 *
 * \param[in] line_ptr The input line of ASCII data.
 * \param[in] format   Format first points to the format string.
 * \param[in] args_ptr The list of parameters.
 *
 * \return Number of input items converted and assigned.
 * \return IO_EOF - When line_ptr is empty string "".
 */
_mqx_int _io_scanline
   (
      char  *line_ptr,
      char  *format,
      va_list       args_ptr
   )
{ /* Body */
            char       suppress_field;
   register _mqx_int   c;
   register _mqx_uint  n;
            char      *sptr = NULL;
            _mqx_int   sign;
            uint32_t    val;
            _mqx_int   width;
            _mqx_int   numtype;  /* used to indicate bit size of argument */
   register _mqx_int   number_of_chars;
            _mqx_uint  temp;
            _mqx_uint  base;
            void      *tmp_ptr;
#if MQX_INCLUDE_FLOATING_POINT_IO
            double     dnum;
#endif

   if ( *line_ptr == '\0' ) {
      return IO_EOF;
   }

   n = 0;
   number_of_chars = 0;
   while ((c = *format++) != 0) {

      width = 0;

      /*
       * skip white space in format string, and any in input line
       */
      if (  (c == ' ') || (c == '\t') || (c == '\n') || (c == '\r') ||
      (c == '\v') || (c == '\f') ) {
         if ( ! _io_scanline_format_ignore_white_space(
            (char  **)&format, &temp ) ) {
            /*
             * End of format string encountered, scanning is finished.
             */
            return (_mqx_int)n;
         } /* Endif */

         if ( ! _io_scanline_ignore_white_space(
            (char  **)&line_ptr , &temp, 0 ) ) {
            /*
             * End of line string encountered, no more input to scan.
             */
            return (_mqx_int)n;
         } /* Endif */
         number_of_chars += temp;
         continue;
      } /* Endif */

      if ( c != '%' ) {

         /*
          * if format character is not %, then it must match text
          * in input line
          */
         if ( c != _io_scanline_ignore_white_space(
            (char  **)&line_ptr , &temp, 0) ) {
            /*
             * Text not matched, stop scanning..
             */
            return (_mqx_int)n;
         } else {
            ++line_ptr;
         } /* Endif */
         number_of_chars += temp;

      } else {
         /*
          * process % format conversion
          */
         suppress_field = 0;
         width = 0;
         numtype = SCAN_MLONG;
         sign = 1;
         val = 0;

         /*
          * Skip whitespace. Check for " %  ", return if found, otherwise
          * get next character.
          */
         if ( ! _io_scanline_format_ignore_white_space( (char  **)&format,
         &temp ) ) {
            return (_mqx_int)n;
         } /* Endif */
         c = *format;

         /*
          * Check for assignment suppression. if "*" given,
          * suppress assignment
          */
         if ( c == '*' ) {
            ++suppress_field;
            c = *(++format);
         } /* Endif */

         /*
          * Skip whitespace. Check for " %  *  ", return if found, otherwise
          * get next character.
          */
         if ( ! _io_scanline_format_ignore_white_space( (char  **)&format,
            &temp))
         {
            return (_mqx_int)n;
         } /* Endif */
         c = *format;

         /*
          * Now check for a width field
          */
         width = 0;
         while (  ('0' <= c) && (c <= '9')  ) {
            width = width * 10 + (int32_t)(c - '0');
            c = *(++format);
         } /* Endwhile */

         /*
          * Skip whitespace. Check for " %  *  23 ", return if found,
          * otherwise get next character.
          */
         if ( ! _io_scanline_format_ignore_white_space(
            (char  **)&format,   &temp ) ) {
            return (_mqx_int)n;
         } /* Endif */
         c = *format++;

         /*
          * Check to see if c is lower case, if so convert to upper case
          */
         if (  (c >= 'a') && (c <= 'z')  ) {
            c -= 32;
         } /* Endif */

         /*
          * Now check to see if c is a type specifier.
          */
         if (c  == 'H') {
            numtype = SCAN_WLONG;
            if ( ! _io_scanline_format_ignore_white_space(
               (char  **)&format, &temp ) )
            {
               return (_mqx_int)n;
            } /* Endif */
            c = *format++;
         } else if (c == 'L') {
            numtype = SCAN_LLONG;
            if ( ! _io_scanline_format_ignore_white_space(
               (char  **)&format, &temp ) )
            {
               return (_mqx_int)n;
            } /* Endif */
            c = *format++;
         } else if (c == 'Z') {
            numtype = SCAN_BLONG;
            if ( ! _io_scanline_format_ignore_white_space(
               (char  **)&format, &temp ) )
            {
               return (_mqx_int)n;
            } /* Endif */
            c = *format++;
         } else if (c == 'M') {
            numtype = SCAN_MLONG;
            if ( ! _io_scanline_format_ignore_white_space(
               (char  **)&format, &temp ) )
            {
               return (_mqx_int)n;
            } /* Endif */
            c = *format++;
         } /* Endif */

         /*
          * Check to see if c is lower case, if so convert to upper case
          */
         if (  (c >= 'a') && (c <= 'z')  ) {
            c -= 32;
         } /* Endif */

         /*
          * Now check to see if c is a valid format specifier.
          */
         switch ( c ) {

            case 'I':
               c = _io_scanline_ignore_white_space( (char  **)&line_ptr,
                  &temp, 0 );
               if ( *line_ptr == '0' ) {
                  ++number_of_chars;
                  ++line_ptr;
                  if ( width ) {
                     if ( --width <= 0 ) {
                        goto print_val;
                     } /* Endif */
                  } /* Endif */
                  if ( (*line_ptr == 'x') || (*line_ptr == 'X') ) {
                     base = 16;
                     ++line_ptr;
                     ++number_of_chars;
                     if ( width ) {
                        if ( --width <= 0 ) {
                           goto print_val;
                        } /* Endif */
                     } /* Endif */
                  } else if ( (*line_ptr == 'b') || (*line_ptr == 'B') ) {
                     base = 2;
                     ++line_ptr;
                     ++number_of_chars;
                     if ( width ) {
                        if ( --width <= 0 ) {
                           goto print_val;
                        } /* Endif */
                     } /* Endif */
                  } else {
                     base = 8;
                     if ( ! _io_scanline_is_octal_digit(*line_ptr) ) {
                        goto print_val;
                     } /* Endif */
                  } /* Endif */
               } else {
                  goto decimal;
               } /* Endif */
               goto doval;

            case 'P':
            case 'X':
               base = 16;
               c = _io_scanline_ignore_white_space( (char  **)&line_ptr,
                  &temp, 0 );
               if ( *line_ptr == '0' ) {
                  ++line_ptr;
                  ++number_of_chars;
                  if ( width ) {
                     if ( --width <= 0 ) {
                        goto print_val;
                     } /* Endif */
                  } /* Endif */
                  if ( (*line_ptr == 'x') || (*line_ptr == 'X') ) {
                     ++line_ptr;
                     ++number_of_chars;
                     if ( width ) {
                        if ( --width <= 0 ) {
                           goto print_val;
                        } /* Endif */
                     } /* Endif */
                  } else if ( ! _io_scanline_is_hex_digit(*line_ptr) ) {
                     goto print_val;
                  } /* Endif */
               } /* Endif */
               goto doval;

            case 'O':
               base = 8;
               c = _io_scanline_ignore_white_space( (char  **)&line_ptr,
                  &temp, 0 );
               if ( *line_ptr == '0' ) {
                  ++number_of_chars;
                  ++line_ptr;
                  if ( width ) {
                     if ( --width <= 0 ) {
                        goto print_val;
                     } /* Endif */
                  } /* Endif */
                  if ( ! _io_scanline_is_octal_digit(*line_ptr) ) {
                     goto print_val;
                  }
               } /* Endif */
               goto doval;

            case 'B':
               base = 2;
               c = _io_scanline_ignore_white_space( (char  **)&line_ptr,
                  &temp, 0 );
               if ( *line_ptr == '0' ) {
                  ++line_ptr;
                  ++number_of_chars;
                  if ( width ) {
                     if ( --width <= 0 ) {
                        goto print_val;
                     } /* Endif */
                  } /* Endif */
                  if ( (*line_ptr == 'b') || (*line_ptr == 'B') ) {
                     ++line_ptr;
                     ++number_of_chars;
                     if ( width ) {
                        if ( --width <= 0 ) {
                           goto print_val;
                        } /* Endif */
                     } /* Endif */
                  } else if ( ! _io_scanline_is_hex_digit(*line_ptr) ) {
                     goto print_val;
                  } /* Endif */
               } /* Endif */
               goto doval;

            case 'D':
               decimal:
               base = 10;
               temp = 0;
               if (  _io_scanline_ignore_white_space( (char  **)&line_ptr,
                  &temp, 0 ) == '-'  )
               {
                  number_of_chars += temp;
                  sign = -1;
                  ++line_ptr;
                  ++number_of_chars;
                  if ( width ) {
                     width -= (int32_t)temp;
                     if ( width <= 0 ) {
                        goto print_val;
                     } /* Endif */
                  } /* Endif */
               } else {
                  number_of_chars += temp;
               } /* Endif */

            case 'U':
               base = 10;
               c = _io_scanline_ignore_white_space( (char  **)&line_ptr,
                  &temp, 0 );
doval:
               val = 0;
               /* remove spaces if any but don't */
               /* parse passed end of line       */
               c = *line_ptr;
               number_of_chars += temp;
               if ( width ) {
                  width -= temp;
                  if ( width <= 0 ) {
                     break;
                  } /* Endif */
               } /* Endif */
               if (  _io_scanline_base_convert( (unsigned char)c, base ) == SCAN_ERROR  ) {
                  return (_mqx_int)n;
               } /* Endif */

               while ( (( c = _io_scanline_base_convert( *line_ptr, base ))
               != SCAN_ERROR) ) {
                  ++line_ptr;
                  ++number_of_chars;
                  val = val * base + (uint32_t)((unsigned char)c & 0x7F);
                  if ( width ) {
                     if ( --width <= 0 ) {
                        break;
                     } /* Endif */
                  } /* Endif */
               } /* Endwhile */

print_val:
               if (  ! suppress_field  ) {
                  /* assign value using appropriate pointer */
                  val *= sign;
                  tmp_ptr = (void *)va_arg(args_ptr, void *);
                  switch ( numtype ) {
                     case SCAN_LLONG:
                        *((uint32_t *)tmp_ptr) = val;
                        break;
                     case SCAN_WLONG:
                        *((uint16_t *)tmp_ptr) = (uint16_t)val;
                        break;
                     case SCAN_BLONG:
                        *((unsigned char *)tmp_ptr) = (unsigned char)val;
                        break;
                     case SCAN_MLONG:
                        *((_mqx_uint *)tmp_ptr) = (_mqx_uint)val;
                        break;
                     default:
                        break;
                  } /* End Switch */
                  ++n;
               } /* Endif */
               break;

            case 'S':
               temp = 0;
               _io_scanline_ignore_white_space( (char  **)&line_ptr,
                  &temp, 0 );
               number_of_chars += temp;
               if ( ! suppress_field ) {
                  sptr = (char *)va_arg(args_ptr, void *);
               } /* Endif */
               if ( width ) {
                  width -= (int32_t)temp;
                  if ( width <= 0 ) {
                     goto string_done;
                  } /* Endif */
               } /* Endif */

               while (( c = *line_ptr ) != 0) {
                  ++line_ptr;
                  ++number_of_chars;
                  if ( c == *format ) {
                     ++format;
                     break;
                  } /* Endif */

                  if ( ! suppress_field ) {
                     *sptr++ = (char)c;
                  } /* Endif */
                  if ( width ) {
                     if ( --width <= 0 ) {
                        break;
                     } /* Endif */
                  } /* Endif */

               } /* Endwhile */

string_done:
               if ( ! suppress_field ) {
                  ++n;
                  *sptr = '\0';
               } /* Endif */
               break;

            case 'C':
               if ( width == 0 ) {
                  width = 1;
               } /* Endif */

               if ( ! suppress_field ) {
                  sptr = (char *)va_arg(args_ptr, void *);
               } /* Endif */
               while ( width-- > 0 ) {
                  if ( ! suppress_field ) {
                     *sptr++ = (unsigned char)*line_ptr;
                  } /* Endif */

                  ++line_ptr;
                  ++number_of_chars;
               } /* Endwhile */

               if ( ! suppress_field ) {
                  ++n;
               } /* Endif */
               break;

            case 'N':
               if ( ! suppress_field ) {
                  tmp_ptr = va_arg(args_ptr, void *);
                  *(_mqx_int *)(tmp_ptr) = (_mqx_int)number_of_chars;
               } /* Endif */
               break;

#if MQX_INCLUDE_FLOATING_POINT_IO
         case 'G':
         case 'F':
         case 'E':
            dnum = strtod((char *)line_ptr, (char  **)&tmp_ptr);

            if ((dnum == HUGE_VAL) || (dnum == -HUGE_VAL))
            {
               return (_mqx_int)n;
            }

            line_ptr = tmp_ptr;
            tmp_ptr = (void *)va_arg(args_ptr, void *);
            if (SCAN_LLONG == numtype)
            {
               *((double *)tmp_ptr) = dnum;
            } else {
               *((float *)tmp_ptr) = (float)dnum;
            }
            ++n;
         break;
#endif

            default:
               return (_mqx_int)n;

         } /* End Switch */

      } /* Endif */
#if 0
      if ( ! *line_ptr || ! *format ) {
         return (_mqx_int)n;
      } /* Endif */
#endif

      /* if end of input string, return */

   } /* Endwhile */
   return (_mqx_int)n;

} /* Endbody */


/*!
 * \brief Scanline function which ignores white spaces.
 *
 * \param[in,out] s_ptr The addres of the string pointer to update.
 * \param[out]    count The number of characters skipped.
 * \param[in]     width Maximum number of white spaces to skip.
 *
 * \return String without white spaces.
 */
char _io_scanline_ignore_white_space
   (
      register char  **s_ptr,
      register _mqx_uint   *count,
      register _mqx_uint        width
   )
{ /* Body */
   register char c;

   c = **s_ptr;
   *count = 0;
   while (  (c == ' ') || (c == '\t') || (c == '\n') || (c == '\r') ||
   (c == '\v') || (c == '\f') ) {
      c = *(++*s_ptr);
      (*count)++;
      if ( width ) {
         if ( --width == 0 ) {
            return(c);
         } /* Endif */
      } /* Endif */
   } /* Endwhile */
   return (c);

} /* Endbody */

/*!
 * \brief Scanline function which ignores white spaces.
 *
 * \param[in,out] s_ptr The address of the string pointer.
 * \param[out]    count The number of characters skipped.
 *
 * \return String without white spaces.
 */
char _io_scanline_format_ignore_white_space
   (
      register char  **s_ptr,
      register _mqx_uint   *count
   )
{ /* Body */
   register char c;

   *count = 0;
   c = **s_ptr;
   while (  (c == ' ') || (c == '\t') || (c == '\n') || (c == '\r') ||
   (c == '\v') || (c == '\f') ) {
      c = *(++*s_ptr);
      (*count)++;
   } /* Endwhile */
   return (c);

} /* Endbody */

/*!
 * \brief Converts input string.
 *
 * \param[in] c    The character to convert.
 * \param[in] base The base to convert the character in.
 *
 * \return Input string.
 * \return SCAN_ERROR (Failure.)
 */
int32_t _io_scanline_base_convert
   (
      register unsigned char c,
      _mqx_uint              base
   )
{ /* Body */

   if (  c >= 'a' && c <= 'z'  ) {
      /* upper case c */
      c -= 32;
   } /* Endif */

   if (  c >= 'A' && c <= 'Z'  ) {
      /* reduce hex digit */
      c -= 55;
   } else if (  (c >= '0') && (c <= '9')  ) {
      /* reduce decimal digit */
      c -= 0x30;
   } else {
      return SCAN_ERROR;
   } /* Endif */
   if ( (_mqx_uint)c  > (base-1) ) {
      return SCAN_ERROR;
   } else {
      return c;
   } /* Endif */

} /* Endbody */

/*!
 * \brief Determines whether input character is an octal number.
 *
 * \param[in] c The character to check.
 *
 * \return TRUE (Input is an octal number.), FALSE (Input is not an octal number.)
 */
bool _io_scanline_is_octal_digit
   (
      char c
   )
{ /* Body */

   if ( (c >= '0') && (c <= '7') ) {   /* An octal digit */
      return TRUE;
   } else {
      return FALSE;
   } /* Endif */

} /* Endbody */

/*!
 * \brief Determines whether input character is a hexadecimal number.
 *
 * \param[in] c The character to check.
 *
 * \return TRUE (Input is a hexadecimal number.), FALSE (Input is not a
 * hexadecimal number.)
 */
bool _io_scanline_is_hex_digit
   (
      char c
   )
{ /* Body */

   if ( ((c >= '0') && (c <= '9')) ||
      ((c >= 'a') && (c <= 'f')) ||
      ((c >= 'A') && (c <= 'F'))
   ) {   /* A hex digit */
      return TRUE;
   } else {
      return FALSE;
   } /* Endif */

} /* Endbody */

#endif // MQX_USE_IO_OLD
