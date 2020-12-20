add_subdirectory(glfw)

set(GLOB GLM_SOURCES glm/*)
set(GLM_INCLUDE glm)
set(IMGUI_DIR ../../)
set(SPDLOG_DIR ./spdlog/)
set(GLOB IMGUI_SOURCES ${IMGUI_DIR}/imgui.cpp ${IMGUI_DIR}/imgui_draw.cpp ${IMGUI_DIR}/imgui_demo.cpp ${IMGUI_DIR}/imgui_tables.cpp ${IMGUI_DIR}/imgui_widgets.cpp)
include_directories(${GLM_INCLUDE} ${IMGUI_DIR} ${IMGUI_DIR}/backends ..)