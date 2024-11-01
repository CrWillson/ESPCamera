idf_component_register(
    SRCS 
        "main.cpp"
    INCLUDE_DIRS 
        "."
        "../include"
    REQUIRES
        driver
        esp32-camera
        spiffs
        fatfs
        sdmmc
)
        
