#ifndef __MFS_TEST
  #define __MFS_TEST

typedef struct
{
  unsigned char en_format;
  unsigned char en_erase;
  unsigned int  read_cicles; // Количество дополнительных повторов чтения файла
  unsigned int  files_cnt;
  unsigned int  file_sz;
  unsigned char write_test;
  unsigned char info;

} T_mfs_test;


int   MFS_test1(T_mfs_test *cbl);
int   MFS_test2(T_mfs_test *cbl);
int   MFS_test3(T_mfs_test *cbl);
#endif
