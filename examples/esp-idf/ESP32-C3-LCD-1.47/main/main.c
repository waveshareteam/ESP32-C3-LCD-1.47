#include "example/01_Factory/Factory_Example.h"
#include "example/02_SD/SD_Example.h"
#include "example/03_Brightness/Brightness_Example.h"
#include "example/04_LVGL/LVGL_Example.h"

#define APP_EXAMPLE_FACTORY         0
#define APP_EXAMPLE_SD_JPG          1
#define APP_EXAMPLE_BRIGHTNESS      0
#define APP_EXAMPLE_LVGL_BENCHMARK  0

#define APP_EXAMPLE_ID APP_EXAMPLE_FACTORY

void app_main(void)
{
#if APP_EXAMPLE_FACTORY
    Factory_Example();
#endif

#if APP_EXAMPLE_SD_JPG
    SD_Example();
#endif

#if APP_EXAMPLE_BRIGHTNESS
    Brightness_Example();
#endif

#if APP_EXAMPLE_LVGL_BENCHMARK
    LVGL_Benchmark_Example();
#endif
}
