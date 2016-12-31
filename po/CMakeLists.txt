include(Gettext)
if(XGETTEXT_FOUND)
	set(potfile "${CMAKE_CURRENT_SOURCE_DIR}/srain.pot")
	gettext_create_pot("${potfile}"
		# DESKTOPFILES "${CMAKE_SOURCE_DIR}/data/Srain.desktop"
		OPTIONS msgid-bugs-address "${Srain_EMAIL}")
	gettext_create_translations("${potfile}" ALL)
endif()
