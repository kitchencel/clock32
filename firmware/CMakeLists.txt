SET(SOURCES ui_main.c
    ui_wifiscreen.c
    ui_location.c
    ui_unitsscreen.c
    ui.c
    ui_comp_hook.c
    ui_helpers.c
    ui_events.c
    fonts/ui_font_clockface286oPTimized.c
    fonts/ui_font_quote30oPTimized.c
    fonts/ui_font_settings32.c
    fonts/ui_font_status26oPTimized.c
    fonts/ui_font_weather96.c
    fonts/ui_font_weatherdegrees32oPTimized.c
    fonts/ui_font_weatherdegrees96oPTimized.c)

add_library(ui ${SOURCES})
