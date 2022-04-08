/*
    Simple ESP-IDF Demo with WT32-SC01 + LovyanGFX + LVGL8.x
*/
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <string>
#include "sdkconfig.h"
#include "esp_event.h"

static const char *TAG = "MAIN";
#define LV_TICK_PERIOD_MS 1
#define LGFX_WT32_SC01 // Wireless Tag / Seeed WT32-SC01
#define LGFX_USE_V1    // LovyanGFX version

//#define LGFX_AUTODETECT
#include <LovyanGFX.h>
#include <LGFX_AUTODETECT.hpp>
#include "ws.h"
static LGFX lcd;

#include <lvgl.h>

/*** Setup screen resolution for LVGL ***/
static const uint16_t screenWidth = 480;
static const uint16_t screenHeight = 320;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[screenWidth * 10];

lv_obj_t *btn1;
lv_obj_t *btn2;
lv_obj_t *screenMain;
lv_obj_t *label;
lv_obj_t *lbl_kbd;
lv_obj_t *lbl_status;

/*** Function declaration ***/
void display_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);
void touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data);
void lv_button_demo(void);
void security_pad(void);
esp_event_loop_handle_t loop_without_task;
ESP_EVENT_DECLARE_BASE(TASK_EVENTS);         // declaration of the task events family
ESP_EVENT_DEFINE_BASE(TASK_EVENTS);
char txt[100];
lv_obj_t *tlabel; // touch x,y label

extern "C"
{

  


static void application_task(void* args)
{
    while(1) {
        ///* let the GUI do its work */
        lv_timer_handler(); 
        lv_tick_inc(LV_TICK_PERIOD_MS);
        vTaskDelay(1);
    }
}
    void app_main(void)
    {
        lcd.init(); // Initialize LovyanGFX
        lv_init();  // Initialize lvgl

        // Setting display to landscape
        if (lcd.width() < lcd.height())
            lcd.setRotation(lcd.getRotation() ^ 1);

        /* LVGL : Setting up buffer to use for display */
        lv_disp_draw_buf_init(&draw_buf, buf, NULL, screenWidth * 10);

        /*** LVGL : Setup & Initialize the display device driver ***/
        static lv_disp_drv_t disp_drv;
        lv_disp_drv_init(&disp_drv);
        disp_drv.hor_res = screenWidth;
        disp_drv.ver_res = screenHeight;
        disp_drv.flush_cb = display_flush;
        disp_drv.draw_buf = &draw_buf;
        lv_disp_drv_register(&disp_drv);

        /*** LVGL : Setup & Initialize the input device driver ***/
        static lv_indev_drv_t indev_drv;
        lv_indev_drv_init(&indev_drv);
        indev_drv.type = LV_INDEV_TYPE_POINTER;
        indev_drv.read_cb = touchpad_read;
        lv_indev_drv_register(&indev_drv);

        /* Create and start a periodic timer interrupt to call lv_tick_inc */

        /*** Create simple label and show LVGL version ***/
 
        //lv_button_demo(); // lvl buttons
        security_pad();
        ws_main();
        bool started = false;
        
        esp_event_loop_args_t loop_args = {
         .queue_size = 5,
         .task_name = NULL // no task will be created
        };
        esp_event_loop_create(&loop_args, &loop_without_task);
        ESP_ERROR_CHECK(esp_event_handler_register_with(loop_without_task, TASK_EVENTS, TASK_ITERATION_EVENT, application_task, loop_without_task));

        xTaskCreate(application_task, "application_task", 2048, NULL, uxTaskPriorityGet(NULL), NULL);

        if (!started){
            websocket_app_start();
            started = true;
        }
   
    }
}

/*** Display callback to flush the buffer to screen ***/
void display_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    lcd.startWrite();
    lcd.setAddrWindow(area->x1, area->y1, w, h);
    lcd.pushColors((uint16_t *)&color_p->full, w * h, true);
    lcd.endWrite();

    lv_disp_flush_ready(disp);
}

/*** Touchpad callback to read the touchpad ***/
void touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
    uint16_t touchX, touchY;
    bool touched = lcd.getTouch(&touchX, &touchY);

    if (!touched)
    {
        data->state = LV_INDEV_STATE_REL;
    }
    else
    {
        data->state = LV_INDEV_STATE_PR;

        /*Set the coordinates*/
        data->point.x = touchX;
        data->point.y = touchY;

        sprintf(txt, "Touch:(%03d,%03d)", touchX, touchY);
        
    }
}
void handle_keypress(const char *inp){
printf(inp);
if (!strcmp(inp, "Clear")){
  lv_label_set_text(lbl_kbd,"");
  lv_label_set_text(lbl_status,"Cleared");
  return;
}
else if (!strcmp(inp, "Submit"))
{
  lv_label_set_text(lbl_kbd,"");
  lv_label_set_text(lbl_status,"Submitted");
  /* code */
  return;
}
//txt = lv_btnmatrix_get_btn_text(btn1, inp);
lv_label_ins_text(lbl_kbd, LV_LABEL_POS_LAST, inp);
lv_label_set_text(lbl_status,"Status");

}
static void event_handler_btn(lv_event_t *e){
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_RELEASED){
      handle_keypress(lv_btnmatrix_get_btn_text(btn1, lv_btnmatrix_get_selected_btn(btn1)));
      }
}

void security_pad(void){
    
  // Screen Object

  // Text
  lbl_status = lv_label_create(lv_scr_act());
  lv_label_set_long_mode(lbl_status, false);     /*Break the long lines*/
  lv_label_set_recolor(lbl_status, true);                      /*Enable re-coloring by commands in the text*/
  lv_label_set_text(lbl_status, "#0000ff Status# ");
  lv_obj_set_width(lbl_status, 440);  /*Set smaller width to make the lines wrap*/
    
  lbl_kbd = lv_label_create(lv_scr_act());
  lv_label_set_long_mode(lbl_kbd, false);     /*Break the long lines*/
  lv_label_set_text(lbl_kbd, "");
  lv_obj_set_width(lbl_kbd, 440);  /*Set smaller width to make the lines wrap*/
  
  lv_obj_t * lbl_time = lv_label_create(lv_scr_act());
    lv_label_set_long_mode(lbl_time, false);     /*Break the long lines*/
    lv_label_set_recolor(lbl_time, true);                      /*Enable re-coloring by commands in the text*/
    lv_label_set_text(lbl_time, "#ff00ff Time 00:00#");
    lv_obj_set_width(lbl_time, 440);  /*Set smaller width to make the lines wrap*/
    
  // Set Label Locations
    lv_obj_set_height(lbl_status, 20);  /*Set smaller width to make the lines wrap*/
    lv_obj_set_pos(lbl_status, 20, 290);
    lv_obj_set_height(lbl_time, 20);  /*Set smaller width to make the lines wrap*/
    lv_obj_set_pos(lbl_time, 20, 10);
    lv_obj_set_height(lbl_kbd, 20);  /*Set smaller width to make the lines wrap*/
    lv_obj_set_pos(lbl_kbd, 220, 10);
  static const char * btnm_map[] = {"1", "2", "3", "\n", 
                                  "4", "5", "6", "\n", 
                                  "7", "8", "9", "\n",
                                  "Clear", "0",  "Submit",""
                                 };
  // BUtton 1
  btn1 = lv_btnmatrix_create(lv_scr_act());
  lv_obj_add_event_cb(btn1, event_handler_btn, LV_EVENT_ALL, NULL);
  lv_btnmatrix_set_map(btn1, btnm_map);
  lv_obj_set_size(btn1, 460, 260);
  lv_obj_set_pos(btn1, 0, 40);
  lv_btnmatrix_set_btn_width(btn1, 9, 2);        /*Make "Action1" twice as wide as "Action2"*/
  lv_btnmatrix_set_btn_width(btn1, 11, 2);        /*Make "Action1" twice as wide as "Action2"*/

  lv_obj_align(btn1, LV_ALIGN_CENTER, 0, 0);

  // Screen load
  lv_scr_load(lv_scr_act());
}