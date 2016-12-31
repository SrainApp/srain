install(FILES Srain.desktop DESTINATION share/applications)
install(DIRECTORY plugins themes DESTINATION share/srain)
install(DIRECTORY icons/16x16 icons/36x36 DESTINATION share/icons/hicolor)

set(RESIZE_ICON_DIR "${CMAKE_CURRENT_BINARY_DIR}/resize_icon")

add_custom_target(
	resize_icon ALL
	${CMAKE_COMMAND} -E make_directory "${RESIZE_ICON_DIR}"
	COMMAND convert icons/raw/srain-black.png -resize 192x192 "${RESIZE_ICON_DIR}/srain-black.png"
	COMMAND convert icons/raw/srain.png -resize 192x192 "${RESIZE_ICON_DIR}/srain.png"
	COMMAND convert icons/raw/srain-red.png -resize 192x192 "${RESIZE_ICON_DIR}/srain-red.png"
	WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)

install(DIRECTORY "${RESIZE_ICON_DIR}/" DESTINATION share/icons/hicolor/192x192)

