#include "App.h"

static  MQX_FILE_PTR   com_handle;
static  MQX_FILE_PTR   sdcard_handle;
static  MQX_FILE_PTR   partition_handle;
static  MQX_FILE_PTR   filesystem_handle;


/*-------------------------------------------------------------------------------------------------------------
  Инициализация файловой системы MFS
-------------------------------------------------------------------------------------------------------------*/
_mqx_int Init_mfs(void)
{
   _mqx_int        res;

   com_handle = fopen("esdhc:", 0);
   if (NULL == com_handle)
   {
      LOGs(__FUNCTION__, __LINE__, SEVERITY_RED  , "Error opening communication handle 'esdhc:.'");
      return MQX_ERROR;
   }

   res = _io_sdcard_install("sdcard:", (void *)&_bsp_sdcard0_init, com_handle);
   if (res != MQX_OK)
   {
      LOGs(__FUNCTION__, __LINE__, SEVERITY_RED  , "Error installing SD card device (0x%x).", res);
      return MQX_ERROR;
   }

   sdcard_handle = fopen("sdcard:", 0);
   if (sdcard_handle == NULL)
   {
      LOGs(__FUNCTION__, __LINE__, SEVERITY_RED  , "Unable to open SD card device.");
      return MQX_ERROR;
   }

   /* Install partition manager over SD card driver */
   res = _io_part_mgr_install(sdcard_handle, PARTMAN_NAME, 0);
   if (res != MFS_NO_ERROR)
   {
      LOGs(__FUNCTION__, __LINE__, SEVERITY_RED  , "Error installing partition manager: %s", MFS_Error_text((uint32_t)res));
      return MQX_ERROR;
   }

   partition_handle = fopen(PARTITION_NAME, NULL);
   if (partition_handle != NULL)
   {
      LOGs(__FUNCTION__, __LINE__, SEVERITY_DEFAULT  , "Installing MFS over partition...");

      /* Validate partition */
      res = _io_ioctl(partition_handle, IO_IOCTL_VAL_PART, NULL);
      if (res != MFS_NO_ERROR)
      {
         LOGs(__FUNCTION__, __LINE__, SEVERITY_RED  , "Error validating partition: %s", MFS_Error_text((uint32_t)res));
         LOGs(__FUNCTION__, __LINE__, SEVERITY_RED  , "Not installing MFS.");
         return MQX_ERROR;
      }

      /* Install MFS over partition */
      res = _io_mfs_install(partition_handle, DISK_NAME, 0);
      if (res != MFS_NO_ERROR)
      {
         LOGs(__FUNCTION__, __LINE__, SEVERITY_RED  , "Error initializing MFS over partition: %s\n", MFS_Error_text((uint32_t)res));
      }

   }
   else
   {

      LOGs(__FUNCTION__, __LINE__, SEVERITY_DEFAULT  , "Installing MFS over SD card driver...");

      /* Install MFS over SD card driver */
      res = _io_mfs_install(sdcard_handle, DISK_NAME, (_file_size)0);
      if (res != MFS_NO_ERROR)
      {
         LOGs(__FUNCTION__, __LINE__, SEVERITY_RED  , "Error initializing MFS: %s", MFS_Error_text((uint32_t)res));
      }
   }

   /* Open file system */
   if (res == MFS_NO_ERROR)
   {
      filesystem_handle = fopen(DISK_NAME, NULL);
      res = ferror(filesystem_handle);
      if (res == MFS_NOT_A_DOS_DISK)
      {
         LOGs(__FUNCTION__, __LINE__, SEVERITY_RED  , "NOT A DOS DISK! You must format to continue.");
      }
      else if (res != MFS_NO_ERROR)
      {
         LOGs(__FUNCTION__, __LINE__, SEVERITY_RED  , "Error opening filesystem: %s", MFS_Error_text((uint32_t)res));
         return MQX_ERROR;
      }
      LOGs(__FUNCTION__, __LINE__, SEVERITY_DEFAULT  , "SD card installed to %s", DISK_NAME);
   }
   return MQX_OK;
}


