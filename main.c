#include <stdio.h>
#include <stdlib.h>
#include <pico/stdlib.h>
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "hardware/gpio.h"
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include "tusb.h"
#include "bsp/board.h"



 SemaphoreHandle_t xSemaphore = NULL;
 BaseType_t xHigherPriorityTaskWoken = pdFALSE;
 

void gpio_callback(uint gpio, uint32_t event_mask) {     
  xSemaphoreGiveFromISR( xSemaphore, &xHigherPriorityTaskWoken);   
}


void vTaskListener( void * pvParameters )
{    
    uint32_t portvalues;
    for( ;; )
    {        
        xSemaphoreTake(xSemaphore, portMAX_DELAY);
        portvalues = gpio_get_all();
        char towrite = (char)((portvalues >> 16) & 0b11);
        
        tud_cdc_n_write_char(0, '0' + towrite);                
        tud_cdc_n_write_flush(0);
        //char testpruts[10] = "test\n";
        //tud_cdc_n_write_str(0, testpruts);
        //gpio_put(PICO_DEFAULT_LED_PIN, !gpio_get(PICO_DEFAULT_LED_PIN));        
        vTaskDelay(pdMS_TO_TICKS(10));         
    }
}

void vTaskSender( void * pvParameters ) {

    for( ;; )
    {   
      if(tud_cdc_connected) {             
        if(tud_cdc_n_available(1)) {

          uint8_t buf[1];

          uint32_t count = tud_cdc_n_read(1, buf, sizeof(buf));

          gpio_clr_mask(~(buf[0]) & 255);
          gpio_set_mask(buf[0] & 255);          
        }
      }
      vTaskDelay(pdMS_TO_TICKS(10));
    }  
}

void usb_device_task(void * params) {
    tusb_init();
  for(;;) {
    tud_task(); // tinyusb device task  
    
  }
}

void initgpios () {
  for(int i = 0; i < 16; i++) {
    gpio_init(i);
    gpio_set_dir(i, GPIO_OUT);     
  }    
  
  gpio_init(PICO_DEFAULT_LED_PIN);
  gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);     
  
  for(int i = 16; i < 18; i++) {
    gpio_init(i);
    gpio_set_dir(i, GPIO_IN);     
    gpio_set_irq_enabled_with_callback(i, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
  }      
}

void main()
{
  stdio_init_all();
  initgpios(); 
  
  sleep_ms(2000);
  board_init();

  
  BaseType_t xReturned;
  
  xSemaphore = xSemaphoreCreateBinary();

  xTaskCreate(vTaskListener, "listenask",configMINIMAL_STACK_SIZE, NULL, (configMAX_PRIORITIES - 3), NULL );       

  xTaskCreate(vTaskSender, "sendtask",   configMINIMAL_STACK_SIZE, NULL, (configMAX_PRIORITIES - 2), NULL );      

  xTaskCreate(usb_device_task, "usb_device_task", configMINIMAL_STACK_SIZE, NULL, (configMAX_PRIORITIES - 1), NULL );      
  
  vTaskStartScheduler();
  for (;;)  ;
}