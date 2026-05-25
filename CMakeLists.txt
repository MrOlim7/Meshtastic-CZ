cmake_minimum_required(VERSION 3.16)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/../sdk/cmake")
include(CZApp)

cz_add_lvgl_app(meshtastic_cz
  SOURCES
    src/main.c
    src/screen_messages.c
    src/screen_nodes.c
    src/screen_config.c
    src/meshtastic_api.c
)
