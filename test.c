g_object_class_override_property(object_class, PROP_AUTOSAVE_FREQUENCY,



                                 "autosave-frequency");

G_DEFINE_TYPE_WITH_CODE(ViewerFile, viewer_file, G_TYPE_OBJECT,
                        G_IMPLEMENT_INTERFACE(VIEWER_TYPE_EDITABLE,
                                viewer_file_editable_interface_init))

static void
viewer_file_editable_interface_init(ViewerEditableInterface *iface) {
    iface->save = viewer_file_editable_save;
    iface->undo = viewer_file_editable_undo;
    iface->redo = viewer_file_editable_redo;
}
