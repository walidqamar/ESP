#include <stdio.h>
#include <string.h>
#include <Arduino.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "driver/sdmmc_host.h"
#include "driver/sdspi_host.h"
#include "sdmmc_cmd.h"
#include "SDCard.h"
#include "../../../src/config.h"

//static const char *TAG = "SDC";

#define SPI_DMA_CHAN 1

SDCard::SDCard(const char *mount_point, gpio_num_t miso, gpio_num_t mosi, gpio_num_t clk, gpio_num_t cs)
{
  gpio_set_pull_mode(PIN_NUM_MOSI, GPIO_PULLUP_ONLY);
  gpio_set_pull_mode(PIN_NUM_CLK, GPIO_PULLUP_ONLY);
  gpio_set_pull_mode(PIN_NUM_MISO, GPIO_PULLUP_ONLY);
  gpio_set_pull_mode(PIN_NUM_CS, GPIO_PULLUP_ONLY);

  m_mount_point = mount_point;
  esp_err_t ret;
  // Options for mounting the filesystem.
  // If format_if_mount_failed is set to true, SD card will be partitioned and
  // formatted in case when mounting fails.
   // Fat FS configuration options
  esp_vfs_fat_sdmmc_mount_config_t mount_config =
      {
          .format_if_mount_failed = true,
          .max_files = 5,
          .allocation_unit_size = 16 * 1024};

  /// ESP_LOGI(TAG, "Initializing SD card");
 // Set up a configuration to the SD host interface
  sdmmc_host_t host_config = SDSPI_HOST_DEFAULT();
  //These Lines is deprecated in new updates
  sdspi_slot_config_t slot_config =SDSPI_SLOT_CONFIG_DEFAULT();
  slot_config.gpio_miso = miso;
  slot_config.gpio_mosi = mosi;
  slot_config.gpio_sck = clk;
  slot_config.gpio_cs = cs;

  /*  // Set up SPI bus
  spi_bus_config_t bus_cfg =
      {
          .mosi_io_num = mosi,
          .miso_io_num = miso,
          .sclk_io_num = clk,
          .quadwp_io_num = -1,
          .quadhd_io_num = -1,
          .max_transfer_sz = 4000};
          
  spi_bus_initialize(HSPI_HOST, &bus_cfg, 1);
  sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
  slot_config.gpio_cs = cs;
  slot_config.host_id = SPI2_HOST; */ //commented for older version

  ret = esp_vfs_fat_sdmmc_mount(m_mount_point.c_str(), &host_config, &slot_config, &mount_config, &m_card);  //Line is deprecated
  //ret = esp_vfs_fat_sdspi_mount(m_mount_point.c_str(), &host_config, &slot_config, &mount_config, &m_card);// commented for older version

  if (ret != ESP_OK)
  {

    card_status = true;
    if (ret == ESP_FAIL)
    {
      // digitalWrite(Cardinfo, HIGH);
      //ESP_LOGE(TAG, "Failed to mount filesystem. "
                 //   "If you want the card to be formatted, set the format_if_mount_failed");
    }
    else
    {
      /// digitalWrite(Cardinfo, HIGH);
      //ESP_LOGE(TAG, "Failed to initialize the card (%s). "
                  //  "Make sure SD card lines have pull-up resistors in place.",
             //  esp_err_to_name(ret));
    }
    return;
  }
  // ESP_LOGI(TAG, "SDCard mounted at: %s", m_mount_point.c_str());
  // Card has been initialized, print its properties
  sdmmc_card_print_info(stdout, m_card);
}

SDCard::~SDCard()
{
  // All done, unmount partition and disable SDMMC or SPI peripheral
  esp_vfs_fat_sdmmc_unmount();
  /// ESP_LOGI(TAG, "Card unmounted");
}