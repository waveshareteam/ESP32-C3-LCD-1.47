#include "LVGL_UI.h"

/**********************
 *      TYPEDEFS
 **********************/
typedef enum {
    DISP_SMALL,
    DISP_MEDIUM,
    DISP_LARGE,
} disp_size_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void Page_01_create(lv_obj_t * parent);
static void Page_02_create(lv_obj_t * parent);

void example1_increase_lvgl_tick(lv_timer_t * t);
/**********************
 *  STATIC VARIABLES
 **********************/
static disp_size_t disp_size;

lv_obj_t * tv;
lv_style_t style_text_muted;
lv_style_t style_title;
static lv_style_t style_icon;
static lv_style_t style_bullet;

static const lv_font_t * font_large;
static const lv_font_t * font_normal;

static lv_timer_t * auto_step_timer;
static lv_timer_t * meter2_timer;

lv_obj_t * SD_Size;
lv_obj_t * FlashSize;
lv_obj_t * Board_deflection;
lv_obj_t * Wireless_Scan;


lv_obj_t *Page_panel[50];
lv_obj_t *Simulated_panel1[100];
size_t Simulated_panel1_Size;

static uint8_t current_tab = 0;
static bool boot_btn_prev = false;

void Lvgl_Example1(void){

  gpio_config_t io_conf = {
      .pin_bit_mask = (1ULL << 9),
      .mode = GPIO_MODE_INPUT,
      .pull_up_en = GPIO_PULLUP_ENABLE,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .intr_type = GPIO_INTR_DISABLE,
  };
  gpio_config(&io_conf);

  disp_size = DISP_SMALL;                            

  font_large = LV_FONT_DEFAULT;                             
  font_normal = LV_FONT_DEFAULT;                         
  
  lv_coord_t tab_h;
  tab_h = 45;
  #if LV_FONT_MONTSERRAT_18
    font_large     = &lv_font_montserrat_18;
  #else
    LV_LOG_WARN("LV_FONT_MONTSERRAT_18 is not enabled for the widgets demo. Using LV_FONT_DEFAULT instead.");
  #endif
  #if LV_FONT_MONTSERRAT_12
    font_normal    = &lv_font_montserrat_12;
  #else
    LV_LOG_WARN("LV_FONT_MONTSERRAT_12 is not enabled for the widgets demo. Using LV_FONT_DEFAULT instead.");
  #endif
  
  lv_style_init(&style_text_muted);
  lv_style_set_text_opa(&style_text_muted, LV_OPA_90);

  lv_style_init(&style_title);
  lv_style_set_text_font(&style_title, font_large);

  lv_style_init(&style_icon);
  lv_style_set_text_color(&style_icon, lv_theme_get_color_primary(NULL));
  lv_style_set_text_font(&style_icon, font_large);

  lv_style_init(&style_bullet);
  lv_style_set_border_width(&style_bullet, 0);
  lv_style_set_radius(&style_bullet, LV_RADIUS_CIRCLE);

  tv = lv_tabview_create(lv_scr_act(), LV_DIR_TOP, tab_h);

  lv_obj_set_style_text_font(lv_scr_act(), font_normal, 0);


  Page_panel[0]  = lv_tabview_add_tab(tv, "Page_01");
  Page_panel[1]  = lv_tabview_add_tab(tv, "Page_02");
  
  Page_01_create(Page_panel[0]);
  Page_02_create(Page_panel[1]);
  
  lv_obj_t *temp_objects[] = {SD_Size, FlashSize, Board_deflection, Wireless_Scan};
  memcpy(Simulated_panel1, temp_objects, sizeof(temp_objects));
  Simulated_panel1_Size = sizeof(temp_objects)/sizeof(lv_obj_t*);
}

void Lvgl_Example1_close(void)
{
  /*Delete all animation*/
  lv_anim_del(NULL, NULL);

  lv_timer_del(meter2_timer);
  meter2_timer = NULL;

  lv_obj_clean(lv_scr_act());

  lv_style_reset(&style_text_muted);
  lv_style_reset(&style_title);
  lv_style_reset(&style_icon);
  lv_style_reset(&style_bullet);
}


/**********************
*   STATIC FUNCTIONS
**********************/

static void Page_01_create(lv_obj_t * parent)
{
  /*Create a panel*/
  lv_obj_t * panel1 = lv_obj_create(parent);
  lv_obj_set_width(panel1, LV_PCT(100));
  lv_obj_set_height(panel1, LV_SIZE_CONTENT);
  lv_obj_set_style_pad_top(panel1, 0, 0);

  lv_obj_t * SD_label = lv_label_create(panel1);
  lv_label_set_text(SD_label, "SD Card");
  lv_obj_add_style(SD_label, &style_text_muted, 0);

  SD_Size = lv_textarea_create(panel1);
  lv_textarea_set_one_line(SD_Size, true);
  lv_textarea_set_placeholder_text(SD_Size, "SD Size");

  lv_obj_t * Flash_label = lv_label_create(panel1);
  lv_label_set_text(Flash_label, "Flash Size");
  lv_obj_add_style(Flash_label, &style_text_muted, 0);

  FlashSize = lv_textarea_create(panel1);
  lv_textarea_set_one_line(FlashSize, true);
  lv_textarea_set_placeholder_text(FlashSize, "Flash Size");

  lv_obj_t * deflection_label1 = lv_label_create(panel1);
  lv_label_set_text(deflection_label1, "IMU x y z");
  lv_obj_add_style(deflection_label1, &style_text_muted, 0);

  Board_deflection = lv_textarea_create(panel1);
  lv_textarea_set_one_line(Board_deflection, true);
  lv_textarea_set_placeholder_text(Board_deflection, "Board deflection");

  lv_obj_t * Gyro_label = lv_label_create(panel1);
  lv_label_set_text(Gyro_label, "Gyro x y z");
  lv_obj_add_style(Gyro_label, &style_text_muted, 0);

  lv_obj_t * Gyro_deflection = lv_textarea_create(panel1);
  lv_textarea_set_one_line(Gyro_deflection, true);
  lv_textarea_set_placeholder_text(Gyro_deflection, "Gyro deflection");
  
  static lv_coord_t grid_main_col_dsc[] = {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
  static lv_coord_t grid_main_row_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
  lv_obj_set_layout(parent, LV_LAYOUT_GRID);
  lv_obj_set_grid_dsc_array(parent, grid_main_col_dsc, grid_main_row_dsc);

  static lv_coord_t grid_1_col_dsc[] = {LV_GRID_FR(5), LV_GRID_FR(5), LV_GRID_FR(50), LV_GRID_FR(5), LV_GRID_FR(5), LV_GRID_TEMPLATE_LAST};
  static lv_coord_t grid_1_row_dsc[] = {
    LV_GRID_CONTENT,  /*Box title*/
    30,               /*Box*/
    LV_GRID_CONTENT,  /*Box title*/
    30,               /*Box*/
    LV_GRID_CONTENT,  /*Box title*/
    30,               /*Box*/
    LV_GRID_CONTENT,  /*Box title*/
    30,               /*Box*/
    LV_GRID_TEMPLATE_LAST          
  };

  lv_obj_set_grid_cell(panel1, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_START, 0, 1);
  lv_obj_set_layout(panel1, LV_LAYOUT_GRID);
  lv_obj_set_grid_dsc_array(panel1, grid_1_col_dsc, grid_1_row_dsc);
  lv_obj_set_grid_cell(SD_label, LV_GRID_ALIGN_START, 0, 5, LV_GRID_ALIGN_START, 0, 1);
  lv_obj_set_grid_cell(SD_Size, LV_GRID_ALIGN_STRETCH, 0, 5, LV_GRID_ALIGN_CENTER, 1, 1);
  lv_obj_set_grid_cell(Flash_label, LV_GRID_ALIGN_START, 0, 5, LV_GRID_ALIGN_START, 2, 1);
  lv_obj_set_grid_cell(FlashSize, LV_GRID_ALIGN_STRETCH, 0, 5, LV_GRID_ALIGN_CENTER, 3, 1);
  lv_obj_set_grid_cell(deflection_label1, LV_GRID_ALIGN_START, 0, 5, LV_GRID_ALIGN_START, 4, 1);
  lv_obj_set_grid_cell(Board_deflection, LV_GRID_ALIGN_STRETCH, 0, 5, LV_GRID_ALIGN_CENTER, 5, 1);
  lv_obj_set_grid_cell(Gyro_label, LV_GRID_ALIGN_START, 0, 5, LV_GRID_ALIGN_START, 6, 1);
  lv_obj_set_grid_cell(Gyro_deflection, LV_GRID_ALIGN_STRETCH, 0, 5, LV_GRID_ALIGN_CENTER, 7, 1);
  
  // Store Gyro_deflection globally so we can update it in timer
  extern lv_obj_t * Gyro_deflection_ptr;
  Gyro_deflection_ptr = Gyro_deflection;

  auto_step_timer = lv_timer_create(example1_increase_lvgl_tick, 100, NULL);
}

lv_obj_t * Gyro_deflection_ptr;

static void Page_02_create(lv_obj_t * parent){
  lv_obj_t * panel1 = lv_obj_create(parent);
  lv_obj_set_width(panel1, LV_PCT(100));
  lv_obj_set_height(panel1, LV_SIZE_CONTENT);
  lv_obj_set_style_pad_top(panel1, 0, 0);

  lv_obj_t * Wireless_label = lv_label_create(panel1);
  lv_label_set_text(Wireless_label, "Wifi STA & AP");
  lv_obj_add_style(Wireless_label, &style_text_muted, 0);

  Wireless_Scan = lv_textarea_create(panel1);
  lv_textarea_set_one_line(Wireless_Scan, true);
  lv_textarea_set_placeholder_text(Wireless_Scan, "Wireless status");

  static lv_coord_t grid_1_col_dsc[] = {LV_GRID_FR(5), LV_GRID_FR(5), LV_GRID_FR(50), LV_GRID_FR(5), LV_GRID_FR(5), LV_GRID_TEMPLATE_LAST};
  static lv_coord_t grid_1_row_dsc[] = {
    LV_GRID_CONTENT,  /*Box title*/
    30,               /*Box*/
    LV_GRID_TEMPLATE_LAST          
  };

  lv_obj_set_grid_cell(panel1, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_START, 0, 1);
  lv_obj_set_layout(panel1, LV_LAYOUT_GRID);
  lv_obj_set_grid_dsc_array(panel1, grid_1_col_dsc, grid_1_row_dsc);
  
  lv_obj_set_grid_cell(Wireless_label, LV_GRID_ALIGN_START, 0, 5, LV_GRID_ALIGN_START, 0, 1);
  lv_obj_set_grid_cell(Wireless_Scan, LV_GRID_ALIGN_STRETCH, 0, 5, LV_GRID_ALIGN_CENTER, 1, 1);
}


void example1_increase_lvgl_tick(lv_timer_t * t)
{
  char buf[100]={0}; 
  
  snprintf(buf, sizeof(buf), "%ld MB\r\n", SDCard_Size);
  lv_textarea_set_placeholder_text(SD_Size, buf);
  snprintf(buf, sizeof(buf), "%ld MB\r\n", Flash_Size);
  lv_textarea_set_placeholder_text(FlashSize, buf);
  snprintf(buf, sizeof(buf), "%.2f %.2f %.2f\r\n", Accel.x, Accel.y, Accel.z);
  lv_textarea_set_placeholder_text(Board_deflection, buf);
  
  if (Gyro_deflection_ptr) {
    snprintf(buf, sizeof(buf), "%.2f %.2f %.2f\r\n", Gyro.x, Gyro.y, Gyro.z);
    lv_textarea_set_placeholder_text(Gyro_deflection_ptr, buf);
  }
  
  if (Wireless_Scan) {
    snprintf(buf, sizeof(buf), "STA: %s | AP: %d\r\n", wifi_sta_connected ? "1" : "0", wifi_ap_sta_count);
    lv_textarea_set_placeholder_text(Wireless_Scan, buf);
  }

  // Handle Boot Button (GPIO9)
  bool boot_btn = (gpio_get_level(9) == 0); // Boot button is active low
  if (boot_btn && !boot_btn_prev) {
    current_tab = (current_tab + 1) % 2;
    lv_tabview_set_act(tv, current_tab, LV_ANIM_ON);
  }
  boot_btn_prev = boot_btn;
}











