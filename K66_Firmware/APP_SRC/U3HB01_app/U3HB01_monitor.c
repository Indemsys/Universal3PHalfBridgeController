// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2016-12-07
// 10:43:46
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"


extern const T_VT100_Menu MENU_MAIN;
extern const T_VT100_Menu MENU_DMC01;

static void Do_DRV8305_diagnostic(uint8_t keycode);
// Эти режимы не реализуем поскольку они могут конфликтовать с основным приложением
// Их можно использовать только если основное приложение отключено 
//static void Do_BLDC_async_mode_diagnostic(uint8_t keycode);
//static void Do_BLDC_sync_mode_diagnostic(uint8_t keycode);

const T_VT100_Menu_item MENU_DMC01_ITEMS[] =
{
  { '1', Do_DRV8305_diagnostic, 0 },
//  { '2', Do_BLDC_async_mode_diagnostic,    0 },
//  { '3', Do_BLDC_sync_mode_diagnostic,     0 },
  { 'R', 0, 0 },
  { 'M', 0,(void *)&MENU_MAIN },
  { 0 }
};

const T_VT100_Menu      MENU_DMC01  =
{
  "U3HB01 Menu",
  "\033[5C <1> - DRV8305 diagnostic\r\n"
//  "\033[5C <2> - BLDC Async mode diagnostic\r\n"
//  "\033[5C <3> - BLDC Sync mode diagnostic\r\n"
  "\033[5C <R> - Display previous menu\r\n"
  "\033[5C <M> - Display main menu\r\n",
  MENU_DMC01_ITEMS,
};



/*-----------------------------------------------------------------------------------------------------
 
 \param keycode 
-----------------------------------------------------------------------------------------------------*/
void Do_DRV8305_diagnostic(uint8_t keycode)
{
  const T_DRV8305_map *drvmap;
  uint32_t      sz;
  uint32_t      i;
  uint32_t      itemn;
  uint32_t      row;
  uint32_t      val;

  uint8_t  b;
  T_monitor_cbl *mcbl;
  mcbl = (T_monitor_cbl *)_task_get_environment(_task_get_id());
  mcbl->_printf(VT100_CLEAR_AND_HOME);


  itemn  = 0;
  drvmap = DRV8305_get_map(&sz);


  do
  {
    row = 0;
    mcbl->_printf("\033[%.2d;%.2dH", row, 0);
    mcbl->_printf("DRV8305 diagnostic.\n\r"); row++;
    mcbl->_printf("Press 'R' to exit. 'E' - edit, '<''>' - Up and Down selection \n\r"); row++;
    mcbl->_printf("\n\r"); row++;

    DRV8305_read_all();
    for (i = 0; i < sz; i++)
    {
      mcbl->_printf(VT100_CLR_LINE);
      if (itemn == i) mcbl->_printf("> ");
      else mcbl->_printf("  ");

      mcbl->_printf("%s = %04X\n\r", drvmap[i].vname, *drvmap[i].pval);
      row++;
    }

    if (mcbl->_wait_char(&b, 2000) == MQX_OK)
    {
      switch (b)
      {
      case '>':
      case '.':
        itemn++;
        if (itemn >= sz) itemn = 0;
        break;
      case '<':
      case ',':
        if (itemn == 0) itemn = sz - 1;
        else itemn--;
        break;
      case 'E':
      case 'e':
        {
          uint32_t tmpr = row;
          mcbl->_printf("\n\r"); row++;
          mcbl->_printf("Register [%s] editing:\n\r", drvmap[itemn].vname); row++;
          val = *drvmap[itemn].pval;
          if (Edit_integer_hex_val(row + 1, &val, 0, 0x7FF) == MQX_OK)
          {
            if (DRV8305_write_register(drvmap[itemn].addr, val) != MQX_OK)
            {
              mcbl->_printf("Write error!\n\r");
              _time_delay(1000);
            }
          }
          mcbl->_printf("\033[%.2d;%.2dH", tmpr, 0);
          mcbl->_printf(VT100_CLR_FM_CRSR);
        }
        break;
      case 'R':
      case 'r':
        return;
      }
    }

  }
  while (1);
}


//  /*-----------------------------------------------------------------------------------------------------
//   
//   \param keycode 
//  -----------------------------------------------------------------------------------------------------*/
//  void Do_BLDC_async_mode_diagnostic(uint8_t keycode)
//  {
//    const  T_DRV8305_map      *DRV8305_map;
//    const  T_BLDC_pars_map    *BLDC_map;
//    uint32_t             sz;
//    uint32_t             blds_map_sz;
//    uint32_t             i;
//    uint32_t             itemn;
//    uint32_t             row;
//    uint32_t             val;
//    uint32_t             hall;
//    uint32_t             sector;
//  
//    uint8_t  b;
//    T_monitor_cbl *mcbl;
//    mcbl = (T_monitor_cbl *)_task_get_environment(_task_get_id());
//    mcbl->_printf(VT100_CLEAR_AND_HOME);
//  
//  
//    sector = 0;
//    itemn  = 0;
//    DRV8305_map = DRV8305_get_map(&sz);
//    BLDC_map    = BLDC_async_get_map(&blds_map_sz);
//  
//    do
//    {
//      row = 0;
//      mcbl->_printf("\033[%.2d;%.2dH", row, 0);
//      mcbl->_printf("BLDC asynchronous mode diagnostic.\n\r"); row++;
//      mcbl->_printf("Press 'R' to exit. 'Q' - start, 'W' - stop, 'E' - edit,'<''>' - Up and Down selection  \n\r"); row++;
//      mcbl->_printf("\n\r"); row++;
//  
//      DRV8305_read_all();
//  
//      mcbl->_printf("Timer MOD  = %d\n\r", FTM_MOTOR_MOD); row++;
//      for (i = 0; i < blds_map_sz; i++)
//      {
//        mcbl->_printf(VT100_CLR_LINE);
//        if (itemn == i) mcbl->_printf("> ");
//        else mcbl->_printf("  ");
//  
//        mcbl->_printf("%s (def=%d)= %d\n\r", BLDC_map[i].vname, BLDC_map[i].defv, *BLDC_map[i].pval);
//        row++;
//      }
//  
//      hall = MDC01_get_hall_state();
//      mcbl->_printf(VT100_CLR_LINE"\r\n");
//      row++;
//      mcbl->_printf(VT100_CLR_LINE"Hall sensors: H1= %d  H2= %d  H3= %d\n\r", (hall & BIT(2)) >> 2, (hall & BIT(1)) >> 1, (hall & BIT(0)) >> 0);
//      row++;
//      if (sector != 0)
//      {
//        mcbl->_printf(VT100_CLR_LINE"Sector: %d\n\r", sector);
//        row++;
//      }
//      else
//      {
//        mcbl->_printf(VT100_CLR_LINE"Sector undefined.\n\r");
//        row++;
//      }
//  
//      if (g_bldc_ctrl_en != 0)
//      {
//        float    speed = 0;
//        uint32_t val = Get_Hall_pulse_length_us();
//  
//        speed = 1000000.0/(val*12.0);
//        mcbl->_printf(VT100_CLR_LINE"Speed= %0.2f RPS ( %0.0f RPM )\n\r", speed, speed*60);
//        row++;
//      }
//      else
//      {
//        mcbl->_printf(VT100_CLR_LINE"Speed= 0\n\r");
//        row++;
//      }
//  
//      {
//        uint32_t val = Get_Hall_pulse_counter();
//        T_hall_capture_cbl *phcbl = Get_Hall_cbl();
//        mcbl->_printf(VT100_CLR_LINE"Counter = %d,  Undef.st.errs=%d, Big delta errs=%d, No change errs=%d\n\r", val, phcbl->undef_state_errors, phcbl->big_delta_errors, phcbl->no_change_state_errors); 
//        row++;
//      }
//  
//  
//      if (mcbl->_wait_char(&b, 2000) == MQX_OK)
//      {
//        switch (b)
//        {
//        case 'Q':
//        case 'q':
//          BLDC_start_async_mode();
//          break;
//        case 'W':
//        case 'w':
//          BLDC_Free_Run();
//          break;
//  
//        case '>':
//        case '.':
//          itemn++;
//          if (itemn >= blds_map_sz) itemn = 0;
//          break;
//        case '<':
//        case ',':
//          if (itemn == 0) itemn = blds_map_sz - 1;
//          else itemn--;
//          break;
//        case 'E':
//        case 'e':
//          {
//            uint32_t tmpr = row;
//            mcbl->_printf("\n\r"); row++;
//            mcbl->_printf("Register [%s] editing:\n\r", BLDC_map[itemn].vname); row++;
//            val = (uint32_t)*BLDC_map[itemn].pval;
//            if (Edit_integer_val(row + 1, &val, 0, 0xFFFF) == MQX_OK)
//            {
//              *BLDC_map[itemn].pval = (uint16_t)val;
//            }
//            mcbl->_printf("\033[%.2d;%.2dH", tmpr, 0);
//            mcbl->_printf(VT100_CLR_FM_CRSR);
//          }
//          break;
//        case '0':
//        case '1':
//        case '2':
//        case '3':
//        case '4':
//        case '5':
//        case '6':
//          {
//            sector = b - '0';
//            BLDC_switch_sector(sector);
//          }
//          break;
//        case 'R':
//        case 'r':
//          return;
//        }
//      }
//  
//    }
//    while (1);
//  }
//  
//  
//  /*-----------------------------------------------------------------------------------------------------
//   
//   \param keycode 
//  -----------------------------------------------------------------------------------------------------*/
//  void Do_BLDC_sync_mode_diagnostic(uint8_t keycode)
//  {
//    const  T_DRV8305_map *DRV8305_map;
//    const  T_BLDC_pars_map    *BLDC_map;
//    uint32_t             sz;
//    uint32_t             blds_map_sz;
//    uint32_t             i;
//    uint32_t             itemn;
//    uint32_t             row;
//    uint32_t             val;
//    uint32_t             hall;
//    uint32_t             sector;
//  
//    uint8_t  b;
//    T_monitor_cbl *mcbl;
//    mcbl = (T_monitor_cbl *)_task_get_environment(_task_get_id());
//    mcbl->_printf(VT100_CLEAR_AND_HOME);
//  
//  
//    sector = 0;
//    itemn  = 0;
//    DRV8305_map = DRV8305_get_map(&sz);
//    BLDC_map    = BLDC_sync_get_map(&blds_map_sz);
//  
//    do
//    {
//      row = 0;
//      mcbl->_printf("\033[%.2d;%.2dH", row, 0);
//      mcbl->_printf("BLDC synchronous mode diagnostic.\n\r"); row++;
//      mcbl->_printf("Press 'R' to exit. 'Q' - start, 'W' - stop, 'E' - edit,'<''>' - Up and Down selection  \n\r"); row++;
//      mcbl->_printf("\n\r"); row++;
//  
//      DRV8305_read_all();
//  
//      mcbl->_printf("Timer MOD  = %d\n\r", FTM_MOTOR_MOD); row++;
//      for (i = 0; i < blds_map_sz; i++)
//      {
//        mcbl->_printf(VT100_CLR_LINE);
//        if (itemn == i) mcbl->_printf("> ");
//        else mcbl->_printf("  ");
//  
//        mcbl->_printf("%s (def=%d)= %d\n\r", BLDC_map[i].vname, BLDC_map[i].defv, *BLDC_map[i].pval);
//        row++;
//      }
//  
//      hall = MDC01_get_hall_state();
//      mcbl->_printf(VT100_CLR_LINE"\r\n");
//      row++;
//      mcbl->_printf(VT100_CLR_LINE"Hall sensors: H1= %d  H2= %d  H3= %d\n\r", (hall & BIT(2)) >> 2, (hall & BIT(1)) >> 1, (hall & BIT(0)) >> 0);
//      row++;
//      if (sector != 0)
//      {
//        mcbl->_printf(VT100_CLR_LINE"Sector: %d\n\r", sector);
//        row++;
//      }
//      else
//      {
//        mcbl->_printf(VT100_CLR_LINE"Sector undefined.\n\r");
//        row++;
//      }
//  
//      if (g_bldc_ctrl_en != 0)
//      {
//        float    speed = 0;
//        uint32_t val = Get_Hall_pulse_length_us();
//  
//        speed = 1000000.0/(val*12.0);
//        mcbl->_printf(VT100_CLR_LINE"Speed= %0.2f RPS ( %0.0f RPM )\n\r", speed, speed*60);
//        row++;
//      }
//      else
//      {
//        mcbl->_printf(VT100_CLR_LINE"Speed= 0\n\r");
//        row++;
//      }
//  
//      {
//        uint32_t val = Get_Hall_pulse_counter();
//        T_hall_capture_cbl *phcbl = Get_Hall_cbl();
//        mcbl->_printf(VT100_CLR_LINE"Counter = %d,  Undef.st.errs=%d, Big delta errs=%d, No change errs=%d\n\r", val, phcbl->undef_state_errors, phcbl->big_delta_errors, phcbl->no_change_state_errors); 
//        row++;
//      }
//  
//  
//      if (mcbl->_wait_char(&b, 2000) == MQX_OK)
//      {
//        switch (b)
//        {
//        case 'Q':
//        case 'q':
//          BLDC_start_sync_mode();
//          break;
//        case 'W':
//        case 'w':
//          BLDC_Free_Run();
//          break;
//  
//        case '>':
//        case '.':
//          itemn++;
//          if (itemn >= blds_map_sz) itemn = 0;
//          break;
//        case '<':
//        case ',':
//          if (itemn == 0) itemn = blds_map_sz - 1;
//          else itemn--;
//          break;
//        case 'E':
//        case 'e':
//          {
//            uint32_t tmpr = row;
//            mcbl->_printf("\n\r"); row++;
//            mcbl->_printf("Register [%s] editing:\n\r", BLDC_map[itemn].vname); row++;
//            val = (uint32_t)*BLDC_map[itemn].pval;
//            if (Edit_integer_val(row + 1, &val, 0, 0xFFFF) == MQX_OK)
//            {
//              *BLDC_map[itemn].pval = (uint16_t)val;
//            }
//            mcbl->_printf("\033[%.2d;%.2dH", tmpr, 0);
//            mcbl->_printf(VT100_CLR_FM_CRSR);
//          }
//          break;
//        case '0':
//        case '1':
//        case '2':
//        case '3':
//        case '4':
//        case '5':
//        case '6':
//          {
//            sector = b - '0';
//            BLDC_switch_sector(sector);
//          }
//          break;
//        case 'R':
//        case 'r':
//          return;
//        }
//      }
//  
//    }
//    while (1);
//  }


