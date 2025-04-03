idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    REQUIRES dht spiffs)
# Compilation failed because main.c (in "main" component) includes esp_spiffs.h, provided by spiffs component(s).
# However, spiffs component(s) is not in the requirements list of "main".
# To fix this, add spiffs to PRIV_REQUIRES list of idf_component_register call in /home/kevin_lucas/esp/v5.4/esp-idf/tcc/MLOps/main/CMakeLists.txt.