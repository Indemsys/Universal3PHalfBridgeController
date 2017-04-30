#include "App.h"
#include "MFS_test.h"

#ifdef MFS_TEST

typedef struct
{
  int    fd;
  uint32_t open_t;
  uint32_t rw_t;
  uint32_t close_t;
} T_file_stat_rec;

  #define STRINGS_SZ 64
/*-------------------------------------------------------------------------------------------------------------
 Последовательно записываем количество файлов заданное в cbl->files_cnt  размером заданным в cbl->file_sz
 Потом последовательно проверяем их содержимое
 Потом последовательно стираем
-------------------------------------------------------------------------------------------------------------*/
int   MFS_test1(T_mfs_test *cbl)
{
  void                *fbuf;
  void                *fbuf2;
  T_file_stat_rec     *ftm;
  char                *str;
  char                *funcname;
  char                *fname_tmpl;
  char                *filename;

  int                  res;
  int                  err_files = 0;
  int                  err_cnt;
  MQX_FILE_PTR         f;
  int                  i,  num;
  uint8_t                *bptr;
  MQX_TICK_STRUCT      t1, t2;
  bool                 overfl;

  uint32_t max_open_t  = 0;
  uint32_t min_open_t  = 0xFFFFFFFF;
  uint32_t max_rw_t    = 0;
  uint32_t min_rw_t    = 0xFFFFFFFF;
  uint32_t max_close_t = 0;
  uint32_t min_close_t = 0xFFFFFFFF;

  float avr_open_t = 0;
  float avr_rw_t = 0;
  float avr_close_t = 0;
  T_monitor_cbl *pvt100_cb;
  pvt100_cb = (T_monitor_cbl *)_task_get_environment(_task_get_id());


  // Выделяем память для массивов
  fbuf = _mem_alloc_zero(cbl->file_sz);
  if (fbuf == NULL)
  {
    pvt100_cb->_printf("\r\nUnnable allocate memory for file first buffer!\r\n");
    goto exit_;
  }
  fbuf2 = _mem_alloc_zero(cbl->file_sz);
  if (fbuf2 == NULL)
  {
    pvt100_cb->_printf("\r\nUnnable allocate memory for file second buffer!\r\n");
    goto exit_;
  }

  ftm = (T_file_stat_rec *)_mem_alloc_zero(cbl->files_cnt * sizeof(T_file_stat_rec));
  if (ftm == NULL)
  {
    pvt100_cb->_printf("\r\nUnnable allocate memory for array of statistic!\r\n");
    goto exit_;
  }

  str = (char *)_mem_alloc_zero(STRINGS_SZ * 3);
  if (ftm == NULL)
  {
    pvt100_cb->_printf("\r\nUnnable allocate memory for strings!\r\n");
    goto exit_;
  }
  funcname   = &str[0];
  fname_tmpl = &str[64];
  filename   = &str[128];


  strcpy(fname_tmpl, DISK_NAME"testf");

  pvt100_cb->_printf("\r\n--------- MFS File System ---------\r\n");


  if (cbl->write_test == 1)
  {

    // Создаем и записываем количество файлов заданное переменной filecount
    pvt100_cb->_printf("-------------------- Writing -----------------\n\r");
    for (num = 0; num < cbl->files_cnt; num++)
    {


      sprintf(filename, "%s%d.BIN", fname_tmpl, num);


      _time_get_ticks(&t1);
      f = fopen(filename, "w");
      _time_get_ticks(&t2);
      ftm[num].open_t = _time_diff_microseconds(&t2, &t1, &overfl);

      if (f == NULL)
      {
        pvt100_cb->_printf("File %s open to write error\n\r", filename);
        continue;
      }

      srand(num);
      bptr = (uint8_t *)fbuf;
      for (i = 0; i < cbl->file_sz; i++)
      {
        *bptr = rand();
        bptr++;
      }

      _time_get_ticks(&t1);
      res = _io_write(f, (void *)fbuf, cbl->file_sz);
      _time_get_ticks(&t2);
      ftm[num].rw_t = _time_diff_microseconds(&t2, &t1, &overfl);

      _time_get_ticks(&t1);
      fclose(f);
      _time_get_ticks(&t2);
      ftm[num].close_t = _time_diff_microseconds(&t2, &t1, &overfl);

      if (res != cbl->file_sz)
      {
        pvt100_cb->_printf("%s Write error. Size = %06d, writen = %d\n\r", filename, cbl->file_sz, res);
      }
      else
      {
        pvt100_cb->_printf("%s %06d %08d %08d %08d %.0f Kb/s\n\r", filename, cbl->file_sz, ftm[num].open_t, ftm[num].rw_t, ftm[num].close_t,  cbl->file_sz * 1000000.0 / ((float)ftm[num].rw_t * 1024.0));
      }


      err_cnt = 0;
      // Проверить файл сразу после записи
      f = fopen(filename, "r");
      if (f == NULL)
      {
        pvt100_cb->_printf("File %s open to read error!\n\r", filename);
        continue;
      }

      _io_read(f, (void *)fbuf2, cbl->file_sz);
      fclose(f);
      {
        uint8_t *ptr1 = (uint8_t *)fbuf;
        uint8_t *ptr2 = (uint8_t *)fbuf2;
        for (i = 0; i < cbl->file_sz; i++)
        {
          if ((*ptr1) != (*ptr2))
          {
            pvt100_cb->_printf("File %s has data error at offset %08X (read -> %02X ,must be -> %02X)\n\r", filename, i, *ptr2, *ptr1);
            err_cnt++;
            if (err_cnt > 32) break; //goto exit_;
          }
          ptr1++;
          ptr2++;
        }
      }


    }

    // Подсчет статистики
    for (num = 0; num < cbl->files_cnt; num++)
    {

      if (ftm[num].open_t > max_open_t) max_open_t = ftm[num].open_t;
      if (ftm[num].open_t < min_open_t) min_open_t = ftm[num].open_t;
      if (ftm[num].rw_t > max_rw_t) max_rw_t = ftm[num].rw_t;
      if (ftm[num].rw_t < min_rw_t) min_rw_t = ftm[num].rw_t;
      if (ftm[num].close_t > max_close_t) max_close_t = ftm[num].close_t;
      if (ftm[num].close_t < min_close_t) min_close_t = ftm[num].close_t;

      avr_open_t  += ftm[num].open_t;
      avr_rw_t    += ftm[num].rw_t;
      avr_close_t += ftm[num].close_t;
    }

    pvt100_cb->_printf("Statistic for write operations.\n\r");
    pvt100_cb->_printf("Open max = %08d us Open min = %08d us, Open aver = %.0f us\n\r", max_open_t, min_open_t,  (float)avr_open_t / (float)cbl->files_cnt);
    pvt100_cb->_printf("Writ max = %08d us Writ min = %08d us, Writ aver = %.0f us\n\r", max_rw_t, min_rw_t,  (float)avr_rw_t / (float)cbl->files_cnt);
    pvt100_cb->_printf("Clos max = %08d us Clos min = %08d us, Clos aver = %.0f us\n\r", max_close_t, min_close_t,  (float)avr_close_t / (float)cbl->files_cnt);
    {
      float t = (float)avr_rw_t / (float)cbl->files_cnt;
      pvt100_cb->_printf("Averaged write speed  = %.0f Kbyte/s\n\r", cbl->file_sz * 1000000.0 / (t * 1024.0));
    }

  }



  // Читаем количество файлов заданное переменной filecount
  pvt100_cb->_printf("-------------------- Reading -----------------\n\r");
  for (num = 0; num < cbl->files_cnt; num++)
  {
    int add_cycle;

    err_cnt = 0;

//      if (num>100)
//      {
    sprintf(filename, "%s%d.BIN", fname_tmpl, num);

    // Организуем дополнительный цикл перепроверки чтения файла в случае появления ошибок
    for (add_cycle = 0; add_cycle < cbl->read_cicles; add_cycle++)
    {


      _time_get_ticks(&t1);
      f = fopen(filename, "r");
      _time_get_ticks(&t2);
      ftm[num].open_t = _time_diff_microseconds(&t2, &t1, &overfl);

      if (f == NULL)
      {
        pvt100_cb->_printf("File %s open to read error!\n\r", filename);
        break;
      }

      _time_get_ticks(&t1);
      _io_read(f, (void *)fbuf, cbl->file_sz);
      _time_get_ticks(&t2);
      ftm[num].rw_t = _time_diff_microseconds(&t2, &t1, &overfl);

      srand(num);
      bptr = (uint8_t *)fbuf;
      for (i = 0; i < cbl->file_sz; i++)
      {
        unsigned char b;
        b = rand();
        if ((*bptr) != b)
        {
          pvt100_cb->_printf("File %s has data error at offset %08X (read -> %02X ,must be -> %02X)\n\r", filename, i, *bptr, b);
          err_cnt++;
          if (err_cnt > 32) break; //goto exit_;
        }
        bptr++;
      }

      if (err_cnt > 0)
      {
        err_files++;
      }


      _time_get_ticks(&t1);
      fclose(f);
      _time_get_ticks(&t2);
      ftm[num].close_t = _time_diff_microseconds(&t2, &t1, &overfl);

      pvt100_cb->_printf("%s %06d %08d %08d %08d %.0f Kb/s\n\r", filename, cbl->file_sz, ftm[num].open_t, ftm[num].rw_t, ftm[num].close_t,  cbl->file_sz * 1000000.0 / ((float)ftm[num].rw_t * 1024.0));
    }
//      }

  }

  max_open_t  = 0;
  min_open_t  = 0xFFFFFFFF;
  max_rw_t    = 0;
  min_rw_t    = 0xFFFFFFFF;
  max_close_t = 0;
  min_close_t = 0xFFFFFFFF;

  avr_open_t = 0;
  avr_rw_t = 0;
  avr_close_t = 0;

  // Подсчет статистики
  for (num = 0; num < cbl->files_cnt; num++)
  {

    if (ftm[num].open_t > max_open_t) max_open_t = ftm[num].open_t;
    if (ftm[num].open_t < min_open_t) min_open_t = ftm[num].open_t;
    if (ftm[num].rw_t > max_rw_t) max_rw_t = ftm[num].rw_t;
    if (ftm[num].rw_t < min_rw_t) min_rw_t = ftm[num].rw_t;
    if (ftm[num].close_t > max_close_t) max_close_t = ftm[num].close_t;
    if (ftm[num].close_t < min_close_t) min_close_t = ftm[num].close_t;

    avr_open_t  += ftm[num].open_t;
    avr_rw_t    += ftm[num].rw_t;
    avr_close_t += ftm[num].close_t;
  }

  pvt100_cb->_printf("Statistic for read operations.\n\r");
  pvt100_cb->_printf("Open max = %08d us Open min = %08d us, Open aver = %.0f us\n\r", max_open_t, min_open_t,  (float)avr_open_t / (float)cbl->files_cnt);
  pvt100_cb->_printf("Read max = %08d us Read min = %08d us, Read aver = %.0f us\n\r", max_rw_t, min_rw_t,  (float)avr_rw_t / (float)cbl->files_cnt);
  pvt100_cb->_printf("Clos max = %08d us Clos min = %08d us, Clos aver = %.0f us\n\r", max_close_t, min_close_t,  (float)avr_close_t / (float)cbl->files_cnt);
  {
    float t = (float)avr_rw_t / (float)cbl->files_cnt;
    pvt100_cb->_printf("Averaged read speed  = %.0f Kbyte/s\n\r", cbl->file_sz * 1000000.0 / (t * 1024.0));
  }


  if (cbl->en_erase)
  {

    // Удаляем количество файлов заданное переменной filecount
    pvt100_cb->_printf("-------------------- Deleting -----------------\n\r");

    for (num = 0; num < cbl->files_cnt; num++)
    {
      MQX_FILE_PTR               fs_ptr;

      sprintf(filename, "%s%d.BIN", fname_tmpl, num);

      fs_ptr = _io_get_fs_by_name(filename);
      _time_get_ticks(&t1);
      _io_ioctl(fs_ptr, IO_IOCTL_DELETE_FILE, (void *)filename);
      _time_get_ticks(&t2);
      ftm[num].open_t = _time_diff_microseconds(&t2, &t1, &overfl);

      pvt100_cb->_printf("%s %06d %08d us\n\r", filename, cbl->file_sz, ftm[num].open_t);

    }

    max_open_t  = 0;
    min_open_t  = 0xFFFFFFFF;

    avr_open_t = 0;

    // Подсчет статистики
    for (num = 0; num < cbl->files_cnt; num++)
    {

      if (ftm[num].open_t > max_open_t) max_open_t = ftm[num].open_t;
      if (ftm[num].open_t < min_open_t) min_open_t = ftm[num].open_t;

      avr_open_t  += ftm[num].open_t;
    }

    pvt100_cb->_printf("Statistic for delete operations.\n\r");
    pvt100_cb->_printf("Delet. max = %08d us Delet. min = %08d us, Delet. aver = %.0f us\n\r", max_open_t, min_open_t,  (float)avr_open_t / (float)cbl->files_cnt);
  }

  pvt100_cb->_printf("\n\rNumber of the damaged files = %d.\n\r", err_files);

exit_:
  if (fbuf)  _mem_free(fbuf);
  if (fbuf2) _mem_free(fbuf2);
  if (ftm)   _mem_free(ftm);
  if (str)   _mem_free(str);
  return 0;
}


/*------------------------------------------------------------------------------
  Записываем файл, проверяем его содержание, стираем

 \param cbl

 \return int
 ------------------------------------------------------------------------------*/
int   MFS_test2(T_mfs_test *cbl)
{
  MQX_FILE_PTR         f;

  unsigned char       *fbuf;
  unsigned char       *fbuf2;
  char                *str;
  char                *funcname;
  char                *fname_tmpl;
  char                *filename;

  int                  res;
  int                  i;
  int                  num;
  MQX_TICK_STRUCT      t1, t2;
  bool                 overfl;

  unsigned int  open_w_t;  // Время открытия файла на запись
  unsigned int  write_t;   // Время записи файла
  unsigned int  close_w_t; // Время закрытия файла на запись

  unsigned int  open_r_t;  // Время открытия файла на запись
  unsigned int  read_t;    // Время записи файла
  unsigned int  close_r_t; // Время закрытия файла на запись

  unsigned int  delete_t;  // Время удаления файла


  T_monitor_cbl *pvt100_cb;
  pvt100_cb = (T_monitor_cbl *)_task_get_environment(_task_get_id());

  // Выделяем память для массивов
  fbuf = _mem_alloc_zero(cbl->file_sz);
  if (fbuf == NULL)
  {
    pvt100_cb->_printf("\r\nUnnable allocate memory for file first buffer!\r\n");
    goto exit_;
  }
  fbuf2 = _mem_alloc_zero(cbl->file_sz);
  if (fbuf2 == NULL)
  {
    pvt100_cb->_printf("\r\nUnnable allocate memory for file second buffer!\r\n");
    goto exit_;
  }

  str = (char *)_mem_alloc_zero(STRINGS_SZ * 3);
  if (str == NULL)
  {
    pvt100_cb->_printf("\r\nUnnable allocate memory for strings!\r\n");
    goto exit_;
  }
  funcname   = &str[0];
  fname_tmpl = &str[64];
  filename   = &str[128];

  strcpy(fname_tmpl, DISK_NAME"F_");

  pvt100_cb->_printf("\r\n--------- MFS File System test 2 ---------\r\n");

  for (num = 0; num < cbl->files_cnt; num++)
  {

    srand(num);

    // Создаем имя файла
    sprintf(filename, "%s%07d.BIN", fname_tmpl, num);



    // Создаем файл
    _time_get_ticks(&t1);
    f = _io_fopen(filename, "w");
    _time_get_ticks(&t2);
    open_w_t = _time_diff_microseconds(&t2, &t1, &overfl);

    if (f == NULL)
    {
      pvt100_cb->_printf("\r\nFile opening for write error!\r\n");
      goto exit_;
    }
    // Заполняем буфер случайными числами
    for (i = 0; i < cbl->file_sz; i++)
    {
      fbuf[i] = rand();
    }

    // Записываем файл
    _time_get_ticks(&t1);
    res = _io_write(f, (void *)fbuf, cbl->file_sz);
    _time_get_ticks(&t2);
    write_t = _time_diff_microseconds(&t2, &t1, &overfl);
    if (res != cbl->file_sz)
    {
      pvt100_cb->_printf("\r\nWrite size error (%d)!\r\n", res);
      goto exit_;
    }

    // Закрываем файл
    _time_get_ticks(&t1);
    _io_fclose(f);
    _time_get_ticks(&t2);
    close_w_t = _time_diff_microseconds(&t2, &t1, &overfl);

    // Открываем файл на чтение
    _time_get_ticks(&t1);
    f = _io_fopen(filename, "r");
    _time_get_ticks(&t2);
    open_r_t = _time_diff_microseconds(&t2, &t1, &overfl);

    if (f == NULL)
    {
      pvt100_cb->_printf("\r\nFile opening for read error!\r\n");
      goto exit_;
    }


    // Читаем файл
    _time_get_ticks(&t1);
    res = _io_read(f, (void *)fbuf2, cbl->file_sz);
    _time_get_ticks(&t2);
    read_t = _time_diff_microseconds(&t2, &t1, &overfl);

    if (res != cbl->file_sz)
    {
      pvt100_cb->_printf("\r\nRead size error (%d)!\r\n", res);
      goto exit_;
    }

    // Закрываем файл
    _time_get_ticks(&t1);
    _io_fclose(f);
    _time_get_ticks(&t2);
    close_r_t = _time_diff_microseconds(&t2, &t1, &overfl);


    f = _io_get_fs_by_name(filename);
    _time_get_ticks(&t1);
    res = _io_ioctl(f, IO_IOCTL_DELETE_FILE, (void *)filename);
    _time_get_ticks(&t2);
    delete_t = _time_diff_microseconds(&t2, &t1, &overfl);

    if (res != MQX_OK)
    {
      pvt100_cb->_printf("\r\nDeleting  error (%d)!\r\n", res);
      goto exit_;
    }

    // Проверяем прочитанные данные
    for (i = 0; i < cbl->file_sz; i++)
    {
      if (fbuf[i] != fbuf2[i])
      {
        pvt100_cb->_printf("\r\nFile error %02X = %02X at %d\r\n", fbuf[i], fbuf2[i], i);
        goto exit_;
      }
    }
    pvt100_cb->_printf("%s o:%07d w:%07d c:%07d o:%07d r:%07d c:%07d d:%07d\r\n",
                       filename,
                       open_w_t,
                       write_t,
                       close_w_t,
                       open_r_t,
                       read_t,
                       close_r_t,
                       delete_t
                      );

  }

exit_:
  if (fbuf)  _mem_free(fbuf);
  if (fbuf2) _mem_free(fbuf2);
  if (str)   _mem_free(str);
  return 0;
}

/*------------------------------------------------------------------------------
 Открываем файл и записываем туда непрерывно блоки
 количество блоков задано в cbl->files_cnt
 размер блоков задан cbl->file_sz
 Повторяем для второго файла

 \param cbl

 \return int
 ------------------------------------------------------------------------------*/
int   MFS_test3(T_mfs_test *cbl)
{
  MQX_FILE_PTR         f;
  unsigned char       *fbuf;

  char                *str;
  char                *funcname;
  char                *fname_tmpl;
  char                *filename;

  int                  res;
  int                  i,k;
  int                  num;
  MQX_TICK_STRUCT      t1, t2;
  bool                 overfl;

  unsigned int  write_t;   // Время записи файла

  T_monitor_cbl *pvt100_cb;
  pvt100_cb = (T_monitor_cbl *)_task_get_environment(_task_get_id());

  // Выделяем память для массивов
  fbuf = _mem_alloc_zero(cbl->file_sz);
  if (fbuf == NULL)
  {
    pvt100_cb->_printf("\r\nUnnable allocate memory for file first buffer!\r\n");
    goto exit_;
  }

  str = (char *)_mem_alloc_zero(STRINGS_SZ * 3);
  if (str == NULL)
  {
    pvt100_cb->_printf("\r\nUnnable allocate memory for strings!\r\n");
    goto exit_;
  }
  funcname   = &str[0];
  fname_tmpl = &str[64];
  filename   = &str[128];

  strcpy(fname_tmpl, DISK_NAME"F_");

  pvt100_cb->_printf("\r\n--------- MFS File System test 3 ---------\r\n");



  for (num = 0; num < 2;num++)
  {
    // Создаем имя файла
    sprintf(filename, "%s%07d.BIN", fname_tmpl, num);

    f = _io_fopen(filename, "w");
    if (f == NULL)
    {
      pvt100_cb->_printf("\r\nFile %s opening for write error!\r\n", filename);
      goto exit_;
    }
    else
    {
      pvt100_cb->_printf("\r\nStarted writing to file  %s.\r\n", filename);
    }

    srand(0);

    for (i = 0; i < cbl->files_cnt; i++)
    {
      // Заполняем буфер случайными числами
      for (k = 0; k < cbl->file_sz; k++)
      {
        fbuf[k] = rand();
      }

      // Записываем файл
      _time_get_ticks(&t1);
      res = _io_write(f, (void *)fbuf, cbl->file_sz);
      _time_get_ticks(&t2);
      write_t = _time_diff_microseconds(&t2, &t1, &overfl);
      if (res != cbl->file_sz)
      {
        pvt100_cb->_printf("\r\nWrite size error (%d)!\r\n", res);
        goto exit_;
      }
      pvt100_cb->_printf("%07d %07d us\r\n",i, write_t);

    }
    _io_fclose(f);

  }

exit_:
  if (fbuf)  _mem_free(fbuf);
  if (str)   _mem_free(str);
  return 0;

}
#endif
